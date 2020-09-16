#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
typedef unsigned long long int mem_addr_t;

typedef struct cache_line{
    char valid;
    mem_addr_t tag;
    unsigned long long int lru; //LRU是用于实施LRU更换策略的计数器
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

int verbosity = 0;
int s = 0;
int b = 0;
int E = 0;
char* trace_file = NULL;

int S;
int B;

int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 0;

cache_t cache;

//初始化cache
void initCache(){
    int i,j;
    cache = (cache_t) malloc(sizeof(cache_set_t)*S);
    for(i=0;i<S;i++){
        cache[i] = (cache_line_t*) malloc(sizeof(cache_line_t)*E);
        for(j=0;j<E;j++){
            cache[i][j].valid = '0';
            cache[i][j].tag = 0;
            cache[i][j].lru = 0;
        }
    }
}

void accessData(mem_addr_t addr){

    int tag_size = 64 - s - b;//t
    mem_addr_t tag = addr >> (s + b);//标记
    mem_addr_t set = addr << (tag_size) >> (tag_size + b);//组索引

    int full = 1;

    int el = 0;//表示空行

    int j;

    for(j=0;j<E;j++){
        cache_line_t line = cache[set][j];//相同标记的那组依次取出每行
        if((line.tag == tag ) && (line.valid == '1')){//击中
            hit_count++;
            lru_counter++;
            cache[set][j].lru = lru_counter;
            return ;
        }else if(line.valid == '0'){//空行
            full = 0;
            el = j;
        }
    }

    miss_count++;//未击中，需要从内存中取，替换使用最少的那行

    if(full){
        eviction_count++;//逐出的总数
        int k;
        int ll = 0;
        mem_addr_t min_lru = cache[set][0].lru;

        //找出最少使用的行
        for(k=1;k<E;k++){
            if(min_lru > cache[set][k].lru){
                ll = k;
                min_lru = cache[set][k].lru;
            }
        }
        //替换
        cache[set][ll].tag = tag;
        lru_counter++;
        cache[set][ll].lru = lru_counter;
    }else{                                  //存在空行
        cache[set][el].tag = tag;
        cache[set][el].valid = '1';
        lru_counter++;
        cache[set][el].lru = lru_counter;
    }
}

void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr = 0;
    unsigned int len = 0;
    FILE* trace_fp = fopen(trace_fn,"r");

    if(!trace_fp){
        fprintf(stderr,"%s: %s\n",trace_fn,strerror(errno));
        exit(1);
    }
    while(fgets(buf,1000,trace_fp)!=NULL){
        if(buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M'){
            sscanf(buf+3,"%llx,%u",&addr,&len);
            if(verbosity){
                printf("%c %llx,%u",buf[1],addr,len);
            }
            accessData(addr);
            if(buf[1] == 'M'){
                accessData(addr);
            }
            if(verbosity){
                printf("\n");
            }
        }
    }fclose(trace_fp);
}
void freeCache(){
    int i;
    for(i=0;i<S;i++){
        free(cache[i]);
    }free(cache);
}

//打印使用说明
void printUsage(char* argv[]){
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n",argv[0]);
    printf("Options:\n");
    printf("  -h          Print this help message.\n");
    printf("  -v          Optional verbose flag.\n");
    printf("  -s <num>    Number of set index bits.\n");
    printf("  -E <num>    Number of lines per set.\n");
    printf("  -b <num>    Number of block offset bits.\n");
    printf("  -t <file>   Trace file.\n");
    printf("\nExamples:\n");
    printf("linux> %s -s 4 -E 1 -b 4 -t traces/yi.trace\n",argv[0]);
    printf("linux> %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n",argv[0]);
    exit(0);
}
int main(int argc,char* argv[])
{
    char c;
    while((c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch (c) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbosity = -1; //打印跟踪（如果设置）
                break;
            case 'h':
                printUsage(argv);
                exit(0);
            default:
                printUsage(argv);
                exit(1);
        }
    }
    if(s == 0 || E == 0 || b == 0|| trace_file == NULL){
        printf("%s:Missing required command line argument\n",argv[0]);
        printUsage(argv);
        exit(1);
    }
    S = 1<<s;
    B = 1<<b;

    initCache();
    replayTrace(trace_file);
    freeCache();
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
