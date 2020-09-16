/* 
 * Author: Zachary Vollen
 * csim.c - A cache simulator that can replay traces from Valgrind
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is LRU.
 *
 * The function printSummary() is given to print hits, misses, and evictions.
 */
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
   LRU is a counter used to implement LRU replacement policy  */
typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    unsigned long long int lru;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0;         /* set index bits */
int b = 0;         /* block offset bits */
int E = 0;         /* associativity 结合性 */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 0;

/* The cache we are simulating */
cache_t cache;  
//mem_addr_t set_index_mask;


//initCache - Allocate memory, and initialize the cache.
void initCache()
{
	int i,j;

	//allocate space for the sets
	cache = (cache_t) malloc(sizeof(cache_set_t)*S);
	for(i=0;i<S;i++){

		//allocate space for the lines
		cache[i] = (cache_line_t*) malloc(sizeof(cache_line_t)*E);
		for(j=0;j<E;j++){

			//initialize the values in the line
			cache[i][j].valid = '0';
	 		cache[i][j].tag = 0;
			cache[i][j].lru = 0;
		}		
	} 
}

//freeCache - free allocated memory
void freeCache()
{
	int i;
	for(i=0;i<S;i++){
		free(cache[i]);
	}
}

/* The key function in your simulator that 
    accesses data at memory address addr and 
    accordingly makes changes to the structures 
    used for implememting the cache.*/
void accessData(mem_addr_t addr)
{
	
	//calculate tag size, the tag, and the set index
	int tag_size = 64 - s - b;
	mem_addr_t tag = addr>>(s+b);
	mem_addr_t set = addr<<(tag_size)>>(tag_size+b);

	//keep track of whether the set is full or not
	int full = 1;

	//an empty line index
	int el = 0;

	int j;
	for(j=0;j<E;j++){		

		//get line
		cache_line_t line = cache[set][j];

		//hit case
		if((line.tag == tag) && (line.valid == '1')){

			//update hit counter
			hit_count++;
			//set lru counter
			lru_counter++;
			cache[set][j].lru = lru_counter;
			return;
		}

		//check if set is full and set empty line idx
		else if(line.valid == '0'){
			full = 0;
			el = j;
		}
	}

	//if not a hit case then it's a miss case
	miss_count++;

	//add evict case here
	if(full){

		//update evict count
		eviction_count++;

		//find lru line to evict
		int k;
		int ll = 0;
		mem_addr_t min_lru = cache[set][0].lru;

		for(k=1;k<E;k++){
			if(min_lru > cache[set][k].lru){
				ll = k;
				min_lru = cache[set][k].lru;
			}
		}				

		//overwrite tag
		cache[set][ll].tag = tag;
		//set lru counter
		lru_counter++;
		cache[set][ll].lru = lru_counter;
		
	}
	//empty line write case
	else{
		cache[set][el].tag = tag;
		cache[set][el].valid = '1';
		//set lru counter
		lru_counter++;
		cache[set][el].lru = lru_counter;
		
	}				
}


//replayTrace - replays the given trace file against the cache replayTrace-针对缓存重播给定的跟踪文件
void replayTrace(char* trace_fn)
{
	char cmd;
	mem_addr_t addr;
	int size;

	//read file
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }
	
	//go through the commands
    while(fscanf(trace_fp, " %c %llx,%d", &cmd, &addr, &size) == 3) {
		switch(cmd){
			case 'M':
				accessData(addr);
				accessData(addr);
				if(verbosity){
					printf("hits:%d misses:%d evictions:%d\n", hit_count, miss_count, eviction_count);
				}
			break;
			case 'L':
				accessData(addr);
				if(verbosity){
					printf("hits:%d misses:%d evictions:%d\n", hit_count, miss_count, eviction_count);
				}
			break;
			case 'S':
				accessData(addr);
				if(verbosity){
					printf("hits:%d misses:%d evictions:%d\n", hit_count, miss_count, eviction_count);
				}
			break;
			default:
			break;
		}
    }

    fclose(trace_fp);   
}

//printUsage - Print usage info
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

    /* Compute S from command line args */
 	  S = 1<<s;

    /* Initialize cache */
    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
    printf("DEBUG: set_index_mask: %llu\n", set_index_mask);
#endif
 
    /* Read the trace and access the cache */
    replayTrace(trace_file);

    /* Free allocated memory */
    freeCache();

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}