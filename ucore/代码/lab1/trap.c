#include <defs.h>
#include <mmu.h>
#include <memlayout.h>
#include <clock.h>
#include <trap.h>
#include <x86.h>
#include <stdio.h>
#include <assert.h>
#include <console.h>
#include <kdebug.h>

#define TICK_NUM 100

static void print_ticks() {
    cprintf("%d ticks\n",TICK_NUM);
#ifdef DEBUG_GRADE
    cprintf("End of Test.\n");
    panic("EOT: kernel seems ok.");
#endif
}

/* *
 * Interrupt descriptor table:
 *
 * Must be built at run time because shifted function addresses can't
 * be represented in relocation records.
 * */
static struct gatedesc idt[256] = {{0}};

static struct pseudodesc idt_pd = {
    sizeof(idt) - 1, (uintptr_t)idt
};

/* idt_init-初始化idt到kern/trap/vectors.S中的每个入口点  */
void
idt_init(void) {
	/*LAB1您的代码：第2步*/
	/*（1）每个中断服务程序（ISR）的入口地址在哪里？
	*所有ISR的输入地址都存储在__vectors中。uintptr_t __vectors[]在哪里？
	*__vectors[]位于kern/trap/vector.S中，该向量在文件tools/vector.c中生成
	*（在lab1中尝试“make”命令，然后您将在kern/trap DIR中找到vector.S）
	*您可以使用“extern uintptr_t_uvectors[]；”来定义稍后将使用的外部变量。
	*（2）现在您应该在中断描述表（IDT）中设置ISR的条目。
	*你能在这个文件里看到idt[256]吗？是的，是IDT！您可以使用SETGATE宏设置IDT的每个项目
	*（3）在设置IDT的内容后，使用“lidt”指令让CPU知道IDT在哪里。
	*你不知道这个指令的意思吗？只要谷歌一下就行了！查看libs/x86.h以了解更多信息。
	*注意：lidt的参数是idt_pd。试着找到它！
	*/
	extern uintptr_t __vectors[]; //保存vectors.S中256个中断处理例程的入口地址数组(详见vector.S 1278行)
    int i;
	for(i = 0;i<(sizeof(idt) / sizeof(struct gatedesc));i++){
		SETGATE(idt[i],0,GD_KTEXT,__vectors[i],DPL_KERNEL);
		//0表示中断门，GD_KTEXT表示.text的段选择子，__vectors[i]表示偏移量，DPL_KERNEL表示特权级别
    }
	// 设置从用户到内核的切换 
    SETGATE(idt[T_SWITCH_TOK], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER);
	// 加载IDT 
    lidt(&idt_pd);
}

static const char *
trapname(int trapno) {
    static const char * const excnames[] = {
        "Divide error",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "BOUND Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection",
        "Page Fault",
        "(unknown trap)",
        "x87 FPU Floating-Point Error",
        "Alignment Check",
        "Machine-Check",
        "SIMD Floating-Point Exception"
    };

    if (trapno < sizeof(excnames)/sizeof(const char * const)) {
        return excnames[trapno];
    }
    if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16) {
        return "Hardware Interrupt";
    }
    return "(unknown trap)";
}

/* trap_in_kernel - test if trap happened in kernel */
bool
trap_in_kernel(struct trapframe *tf) {
    return (tf->tf_cs == (uint16_t)KERNEL_CS);
}

static const char *IA32flags[] = {
    "CF", NULL, "PF", NULL, "AF", NULL, "ZF", "SF",
    "TF", "IF", "DF", "OF", NULL, NULL, "NT", NULL,
    "RF", "VM", "AC", "VIF", "VIP", "ID", NULL, NULL,
};

void
print_trapframe(struct trapframe *tf) {
    cprintf("trapframe at %p\n", tf);
    print_regs(&tf->tf_regs);
    cprintf("  ds   0x----%04x\n", tf->tf_ds);
    cprintf("  es   0x----%04x\n", tf->tf_es);
    cprintf("  fs   0x----%04x\n", tf->tf_fs);
    cprintf("  gs   0x----%04x\n", tf->tf_gs);
    cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
    cprintf("  err  0x%08x\n", tf->tf_err);
    cprintf("  eip  0x%08x\n", tf->tf_eip);
    cprintf("  cs   0x----%04x\n", tf->tf_cs);
    cprintf("  flag 0x%08x ", tf->tf_eflags);

    int i, j;
    for (i = 0, j = 1; i < sizeof(IA32flags) / sizeof(IA32flags[0]); i ++, j <<= 1) {
        if ((tf->tf_eflags & j) && IA32flags[i] != NULL) {
            cprintf("%s,", IA32flags[i]);
        }
    }
    cprintf("IOPL=%d\n", (tf->tf_eflags & FL_IOPL_MASK) >> 12);

    if (!trap_in_kernel(tf)) {
        cprintf("  esp  0x%08x\n", tf->tf_esp);
        cprintf("  ss   0x----%04x\n", tf->tf_ss);
    }
}

void
print_regs(struct pushregs *regs) {
    cprintf("  edi  0x%08x\n", regs->reg_edi);
    cprintf("  esi  0x%08x\n", regs->reg_esi);
    cprintf("  ebp  0x%08x\n", regs->reg_ebp);
    cprintf("  oesp 0x%08x\n", regs->reg_oesp);
    cprintf("  ebx  0x%08x\n", regs->reg_ebx);
    cprintf("  edx  0x%08x\n", regs->reg_edx);
    cprintf("  ecx  0x%08x\n", regs->reg_ecx);
    cprintf("  eax  0x%08x\n", regs->reg_eax);
}

/* trap_dispatch - dispatch based on what type of trap occurred */
static void
trap_dispatch(struct trapframe *tf) {
    char c;

    switch (tf->tf_trapno) {
    case IRQ_OFFSET + IRQ_TIMER:
        /* LAB1 YOUR CODE : STEP 3 */
        /* handle the timer interrupt */
        /* (1) After a timer interrupt, you should record this event using a global variable (increase it), such as ticks in kern/driver/clock.c
         * (2) Every TICK_NUM cycle, you can print some info using a funciton, such as print_ticks().
         * (3) Too Simple? Yes, I think so!
         */
		/*LAB1您的代码：步骤3*/
		/*处理定时器中断*/
		/*（1）在计时器中断之后，您应该使用全局变量（增加它）来记录这个事件，比如kern/driver/clock.c中的ticks
		*（2）每个TICK_NUM循环中，可以使用print_ticks（）等函数打印一些信息。
		*（3）太简单了？是的，我想是的！]
		*/ 
		if (((++ticks) % TICK_NUM) == 0) {
            print_ticks();
            ticks = 0;
        }
        break;
    case IRQ_OFFSET + IRQ_COM1:
        c = cons_getc();
        cprintf("serial [%03d] %c\n", c, c);
        break;
    case IRQ_OFFSET + IRQ_KBD:
        c = cons_getc();
        cprintf("kbd [%03d] %c\n", c, c);
        break;
    //LAB1挑战1：你的代码你应该修改下面的代码。 
    case T_SWITCH_TOU:
		if(tf -> tf_cs != USER_CS){
			switchk2u = *tf;
			switchk2u.tf_cs = USER_CS;
			switchk2u.tf_ds = switchk2u.tf_es = switchk2u.tf_ss = USER_DS;
			switchk2u.tf_esp = (uint32_t)tf + sizeof(struct trapframe) - 8;
			
			switchk2u.tf_eflags |= FL_IOPL_MASK;
			
			*((uint32_t *)tf -1) = (uint32_t)&switchk2u;
		}
		break;
		
    case T_SWITCH_TOK:
		if(tf -> tf_cs != KERNEL_CS){
			tf->cs = KERNEL_CS;
			tf->tf_ds = tf->tf_es = KERNEL_DS;
			tf->tf_eflags &= ~FL_IOPL_MASK;
			switchu2k = (struct trapframe *)(tf->tf_esp - (sizeof(struct trapframe)-8));
			memmove(switchu2k,tf,sizeof(struct trapframe)-8);
			*((uint32_t *)tf - 1) = (uint32_t)switchu2k;
		}
        break;
    case IRQ_OFFSET + IRQ_IDE1:
    case IRQ_OFFSET + IRQ_IDE2:
        /* do nothing */
        break;

    default:
        // in kernel, it must be a mistake
        if ((tf->tf_cs & 3) == 0) {
            print_trapframe(tf);
            panic("unexpected trap in kernel.\n");
        }
    }
}

/* *
 * trap-处理或调度异常/中断。
 * 如果trap（）返回，kern/trap/trapentry.S中的代码将恢复trapframe中保存的旧CPU状态，
 * 然后使用iret指令从异常中返回。
 * */
void
trap(struct trapframe *tf) {
    // 根据发生的陷阱类型进行调度
    trap_dispatch(tf);
}

