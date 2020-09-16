/* 
 * tsh - A tiny shell program with job control
 * tsh-具有作业控制的小型Shell程序
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants 其他清单常数*/
#define MAXLINE    1024   /* max line size 最大行尺寸*/
#define MAXARGS     128   /* max args on a command line命令行上的最大参数 */
#define MAXJOBS      16   /* max jobs at any point in time 29/5000任何时间点的最大职位数*/
#define MAXJID    1<<16   /* max job ID 最大工作编号*/

/* Job states工作状态 */
#define UNDEF 0 /* undefined 未定义*/
#define FG 1    /* running in foreground 在前台运行*/
#define BG 2    /* running in background在后台运行 */
#define ST 3    /* stopped 停止了*/

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 *工作状态：FG（前景），BG（背景），ST（已停止）
  *作业状态转换和启用操作：
  * FG-> ST：ctrl-z
  * ST-> FG：fg命令
  * ST-> BG：bg命令
  * BG-> FG：fg命令
  * FG状态最多可以有1个工作。
 */

/* Global variables 全局变量*/
extern char **environ;      /* defined in libc 在libc中定义*/
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE)命令行提示符（请勿更改） */
int verbose = 0;            /* if true, print additional output 如果为true，则输出其他输出*/
int nextjid = 1;            /* next job ID to allocate 下一个要分配的作业ID*/
char sbuf[MAXLINE];         /* for composing sprintf messages用于撰写sprintf消息 */

struct job_t {              /* The job struct工作结构 */
    pid_t pid;              /* job PID 工作PID*/
    int jid;                /* job ID [1, 2, ...]作业ID [1、2，...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line 命令行*/
};
struct job_t jobs[MAXJOBS]; /* The job list 工作清单*/
/* End global variables 结束全局变量*/


/* Function prototypes功能原型 */

/* Here are the functions that you will implement这是您将要实现的功能 */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you 这是我们为您提供的帮助程序*/
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine main-外壳程序的主例程
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) 发出提示（默认）*/

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout)
	 *将stderr重定向到stdout（这样驱动程序将获得所有输出在连接到stdout的管道上） */
    dup2(1, 2);

    /* Parse the command line 解析命令行*/
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message 打印帮助信息*/
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info发出其他诊断信息 */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt 不要打印提示*/
            emit_prompt = 0;  /* handy for automatic testing方便进行自动测试*/
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers 安装信号处理程序*/

    /* These are the ones you will need to implement这些是您需要实现的 */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child 终止或停止的孩子*/

    /* This one provides a clean way to kill the shell这提供了一种清除外壳的干净方法 */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list 初始化工作清单*/
    initjobs(jobs);

    /* Execute the shell's read/eval loop 执行shell的读/评估循环*/
    while (1) {

	/* Read command line 读取命令行*/
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) 文件结尾（ctrl-d）*/
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line 评估命令行*/
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here 控制永远不会到达这里*/
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
 *eval-评估用户刚刚输入的命令行
  *如果用户请求了内置命令（退出，作业，bg或fg）
  *然后立即执行。 否则，派生一个子进程并
  *在孩子的背景下工作。 如果作业正在运行
  *前景，等待其终止然后返回。 注意：
  *每个子进程必须具有唯一的进程组ID，以便我们
  *后台子进程未从内核接收SIGINT（SIGTSTP）
  *当我们在键盘上输入ctrl-c（ctrl-z）时。
*/
void eval(char *cmdline) 
{
   char *argv[MAXARGS] = {NULL,};
   int state = UNDEF;
   sigset_t set;
   pid_t pid;
   
   //参数
   if(parseline(cmdline,argv) == 1){
	   state = BG;
   }else{
		state = FG;
   }
   
   if(argv[0] == NULL) {
	   return ;
   }
   
   if(!builtin_cmd(argv)){
	   if(sigemptyset(&set) < 0){
		   unix_error("sigemptyset error");
	   }
	   if(sigaddset(&set,SIGINT) < 0 || sigaddset(&set,SIGTSTP) < 0 || sigaddset(&set,SIGCHLD) < 0){
		   unix_error("sigaddset error");
	   }
	   if(sigprocmask(SIG_BLOCK,&set,NULL) < 0){
		   unix_error("sigprocmask error");
	   }
	   
		if((pid = fork()) < 0){
			unix_error("fork error");
		}else if(pid == 0){
			if(sigprocmask(SIG_UNBLOCK,&set,NULL)<0){
				unix_error("sigprocmask error");
			}
			if(setpgid(0,0) < 0){
				unix_error("setpgid error");
			}
			if(execve(argv[0],argv,environ) < 0){
				printf("%s: command not found\n",argv[0]);
				exit(0);
			}
		}
		if(sigprocmask(SIG_UNBLOCK,&set,NULL) < 0){
			unix_error("sigprocmask error");
		}
		addjob(jobs,pid,state,cmdline);
		
		//如果FG,等待直到结束，否则打印信息
		if(state == FG){
			waitfg(pid);
		}else{
			printf("[%d] (%d) %s",pid2jid(pid),pid,cmdline);
		}
   }
   return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 
 
* parseline-解析命令行并构建argv数组，用单引号引起来的字符被视为单个参数。 如果用户请求了BG作业，则返回true；如果用户请求了FG作业，则返回false。
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.   builtin_cmd-如果用户键入了内置命令，请立即执行。
 */
int builtin_cmd(char **argv) 
{
	if(!strcmp(argv[0],"quit")){
		exit(0);
	}else if(!strcmp(argv[0],"fg") || !strcmp(argv[0],"bg")){
		do_bgfg(argv);
	}else if(!strcmp(argv[0],"jobs")){
		listjobs(jobs);
	}else{
		return 0;
	}
    return 1;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands 执行内置的bg和fg命令
 */
void do_bgfg(char **argv) 
{
	int parsed;
	struct job_t *job;
	//如果没有参数
	if(!argv[1]){
		printf("%s command requires PID or %%jobid argument\n",argv[0]);
		return ;
	}
	if(argv[1][0] == '%'){
		if(!sscanf(&argv[1][1],"%d",&parsed)){
			printf("%s:argument must be a PID or %%jobid\n",argv[0]);
			return ;
		}
		if((job = getjobjid(jobs,parsed)) == NULL){
			printf("%%%d:No such job\n",parsed);
			return ;
		}
	}else {
		if(!sscanf(argv[1],"%d",&parsed)){
			printf("%s: argument must be a PID or %%jobid\n",argv[0]);
			return ;
		}
		if((job = getjobpid(jobs,parsed)) == NULL){
			printf("(%d): No such process\n",parsed);
			return;
		}
	}
	
	if(!strcmp(argv[0],"bg")){
		job->state = BG;
		if(kill(-job->pid,SIGCONT) < 0){
			unix_error("kill error");
		}
		printf("[%d] (%d) %s",job->jid,job->pid,job->cmdline);
	}else if(!strcmp(argv[0],"fg")){
		job->state = FG;
		if(kill(-job->pid,SIGCONT) < 0){
			unix_error("kill error");
		}
		waitfg(job->pid);
	}else{
		puts("do_bgfg:Internal error");
		exit(0);
	}
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 阻塞直到进程pid不再是前台进程
 */
void waitfg(pid_t pid)
{
	struct job_t *job = getjobpid(jobs,pid);
	if(!job){
		return ;
	}
	while(job->state == FG){
		sleep(1);
	}
	if(verbose){
		printf("waitfg:Process (%d) no longer the fg process\n",pid);
	}
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 sigchld_handler-每当子作业终止（成为僵尸）或由于接收到SIGSTOP或SIGTSTP信号而停止时，内核都会将SIGCHLD发送到外壳。 处理程序会收割所有可用的僵尸子级，但不等待其他任何正在运行的子级终止。
 */
void sigchld_handler(int sig) 
{
    int status,jid;
	pid_t pid;
	struct job_t *job;
	
	if(verbose){ //如果为true，则输出其他输出
		puts("sigchd_handler: entering");
	}
	while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED)) > 0){
		if((job = getjobpid(jobs,pid)) == NULL ){
			printf("Lost track of (%d)\n",pid);
			return;
		}
		
		jid = job->jid;
		//stop signal
		if(WIFSTOPPED(status)){
			printf("Job [%d] (%d) stopped by signal %d\n",jid,job->pid,WSTOPSIG(status));
			job->state = ST;
		}
		//exit
		else if(WIFEXITED(status)){
			if(deletejob(jobs,pid)){
				if(verbose){
					printf("sigchld_handler: Job [%d] (%d) deleted\n",jid,pid);
					printf("sigchld_handler: Job [%d] (%d) terminates OK(status %d)\n",jid,pid,WEXITSTATUS(status));
					
				}
			}
		}
		//exit by signal
		else{
			if(deletejob(jobs,pid)){
				if(verbose){
					printf("sigchld_handler: Job [%d] (%d) deleted\n",jid,pid);
				}
			}
			printf("Job [%d] (%d) terminated by signal %d\n",jid,pid,WTERMSIG(status));
		}
	}
	//end handler
	if(verbose){
		puts("sigchld_handler: exiting");
	}
	return;
}
/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 * sigint处理程序-每当用户在键盘上键入ctrl-c时，内核都会将SIGINT发送到外壳。 赶上并将其发送到前台作业。
 */
void sigint_handler(int sig) 
{
	pid_t pid = fgpid(jobs);
	if(verbose){
		puts("sigint_handler: exiting");
	}
	
	//if fg exists send SIGINT
	if(pid){
		if(kill(-pid,SIGINT)< 0){
			unix_error("kill (sigint) error");
			if(verbose){
				printf("sigint_handler: Job (%d) killed\n",pid);
			}
		}
	}
	//end handler
	if(verbose){
		puts("sigint_handler: exiting");
	}
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 sigtstp_handler-每当用户在键盘上键入ctrl-z时，内核就会将SIGTSTP发送到外壳。 捕获它并通过向其发送SIGTSTP来挂起前台作业。
 */
void sigtstp_handler(int sig) 
{
	pid_t pid = fgpid(jobs);
	struct job_t * job = getjobpid(jobs,pid);
	
	if(verbose){
		puts("sigstp_handler: entering");
	}
	//if fg exists then send SIGSTP to job
	if(pid){
		if(kill(-pid,SIGTSTP) < 0){
			unix_error("kill (tstp) error");
		}
		if(verbose){
			printf("sigstp_handler: Job [%d] (%d) stopped\n",job->jid,pid);
		}
	}
	//end handler
	if(verbose){
		puts("sigstp_handler: exiting");
	}
	return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



