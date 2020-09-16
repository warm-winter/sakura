#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

//#define DEBUG_ON 
#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
   LRU is a counter used to implement LRU replacement policy  
类型：缓存行LRU是用于实施LRU替换策略的计数器*/
typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    unsigned long long int lru;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 1;

/* The cache we are simulating */
cache_t cache;  
mem_addr_t set_index_mask;

/* 
 * initCache - Allocate memory, write 0's for valid and tag and LRU
 * also computes the set_index_maskinitCache-分配内存，为有效和标记以及LRU写入0
  *还计算set_index_mask
 */
void initCache()
{
	cache = (cache_t)malloc(S * sizeof(cache_set_t));
	for (int i = 0; i < S; ++i) {
		cache[i] = (cache_set_t)malloc(E * sizeof(cache_line_t));
		memset(cache[i], 0, sizeof(cache_line_t)); //clear memeory.
	}	
}


/* 
 * freeCache - free allocated memory
 */
void freeCache()
{
	for (int i = 0; i < S; ++i)
		free(cache[i]);
	free(cache);
}


/* 
 * accessData - Access data at memory address addr.
 *   If it is already in cache, increast hit_count
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase eviction_count if a line is evicted.
 *accessData-访问内存地址addr处的数据。
*如果它已经在缓存中，请增加命中计数
*如果它不在缓存中，请将其放入缓存，增加未命中计数。
*如果一行被逐出，还应增加逐出数量。
 */
void accessData(mem_addr_t addr)
{
	// mem_addr_t _b = addr & (1 << (b-1));
	mem_addr_t _s = (addr >> b) & ((1 << s)-1);
	mem_addr_t _t = addr >> (b + s);
	cache_set_t cline = cache[_s];
	
	for (int i = 0; i < E; ++i)
		if (cline[i].valid && cline[i].tag == _t) {
			hit_count++;
			cline[i].lru = lru_counter++;
			return;
		}
	
	miss_count++;
	unsigned long long int _min = ~0;
	int id = -1;

	for (int i = 0; i < E; ++i)
		if (cline[i].valid == 0) {
			cline[i].valid = 1;
			cline[i].tag = _t;
			cline[i].lru = lru_counter++;
			return;
		}
		else {
			if (cline[i].lru < _min) {
				_min = cline[i].lru;
				id = i;
			}
		}

	eviction_count++;
	cline[id].tag = _t;
	cline[id].lru = lru_counter++;
}


/*
 * replayTrace - replays the given trace file against the cache 
 */
void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while( fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);
      
            if(verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);

            accessData(addr);

            /* If the instruction is R/W then access again */
            if(buf[1]=='M')
                accessData(addr);
            
            if (verbosity)
                printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/* /\* */
/*  * self-wriiten printSummary function */
/*  *\/ */
/* void printSummary(int hit_count, int miss_count, int eviction_count) { */
/* 	printf("hits:%d misses:%d evictions:%d\n", hit_count, miss_count, eviction_count); */
/* } */

/*
 * main - Main routine 
 */
int main(int argc, char* argv[])
{
    char c;

    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    /* Compute S, E and B from command line args */
    S = 1 << s;
	B = 1 << b;
 
    /* Initialize cache */
    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
    printf("DEBUG: set_index_mask: %llu\n", set_index_mask);
#endif
 
    replayTrace(trace_file);

    /* Free allocated memory */
    freeCache();

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}