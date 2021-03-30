/*
NAME: Dhakshin Suriakannu
EMAIL: bruindhakshin@g.ucla.edu
ID: 605280083
*/
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "SortedList.h"


long threads = 1;
long iterations = 1;
SortedListElement_t *elements;
SortedList_t* listhead;
char sync_char=0;
pthread_mutex_t* m_lock;
int* lock;
long lists=1;

void process_args(int argc, char *argv[], long *threads, long *iterations, int *opt_yield, long *lists)
{
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
        {0, 0, 0, 0}};
    char c;
    while (1)
    {
        c = getopt_long(argc, argv, "p", long_options, NULL);
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
        case 't':
            *threads = atoi(optarg);
            break;
        case 'l':
            *lists = atoi(optarg);
            break;
        case 'i':
            *iterations = atoi(optarg);
            break;
        case 'y':
            for (long unsigned int i = 0; i < strlen(optarg); i++) {
					if (optarg[i] == 'i') {
						*opt_yield |= INSERT_YIELD;
					} else if (optarg[i] == 'd') {
						*opt_yield |= DELETE_YIELD;
					} else if (optarg[i] == 'l') {
						*opt_yield |= LOOKUP_YIELD;
					}
				}
			break;
        case 's':
            sync_char = optarg[0];
			break;
        default:
            fprintf(stderr, "Error: Invalid input!\r\n");
            exit(1);
        }
    }
}
void print_output(long threads, long iterations, long num, long operations, long runtime, long avg, long lockWait)
{

    fprintf(stdout, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", threads, iterations, num, operations, runtime, avg,lockWait);
}

unsigned int hashkey(const char* key) {
 	return key[0]%lists;
}

void * thread_worker(void *ptr) {
    long time = 0;
	struct timespec start, end;
    SortedListElement_t* arr_elements = ptr;
    
    for (long i = 0; i < iterations; i++){
        unsigned int hash = hashkey( arr_elements[i].key);
        clock_gettime(CLOCK_MONOTONIC, &start);
         if (sync_char == 'm') {
        	pthread_mutex_lock(&m_lock[hash]);
    	} else if (sync_char == 's') {
        	while (__sync_lock_test_and_set(&lock[hash], 1));
    	}
        clock_gettime(CLOCK_MONOTONIC, &end);
        time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        SortedList_insert(&listhead[hash], (SortedListElement_t *) &arr_elements[i]); //insert element

        if (sync_char == 'm') { // unlock the mutex
        pthread_mutex_unlock(&m_lock[hash]);
	} else if (sync_char == 's') { // release lock
       	__sync_lock_release(&lock[hash]);
    }
    } 
    long len=0;
    for (long i = 0; i < lists; i++) {
    		len += SortedList_length(&listhead[i]);
	}
    
    if(len<iterations){
        fprintf(stderr,"Not all elements were inserted!");
        exit(2);
    }
    for (long i = 0; i <iterations; i++) {
        unsigned int hash = hashkey( arr_elements[i].key);
        clock_gettime(CLOCK_MONOTONIC, &start);
         if (sync_char == 'm') {
        	pthread_mutex_lock(&m_lock[hash]);
    	} else if (sync_char == 's') {
        	while (__sync_lock_test_and_set(&lock[hash], 1));
    	}
        clock_gettime(CLOCK_MONOTONIC, &end);
        time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        SortedListElement_t *e = SortedList_lookup(&listhead[hash], (char *)((arr_elements[i]).key)); //lookup inserted element
        if(e==NULL){
            fprintf(stderr, "Cannot find element");
            exit(2);
        }
        int x = SortedList_delete(e); //remove inserted element
        if(x==1){
            fprintf(stderr, "Cannot delete element");
            exit(2);
        }
        if (sync_char == 'm') { // unlock the mutex
        pthread_mutex_unlock(&m_lock[hash]);
	} else if (sync_char == 's') { // release lock
       	__sync_lock_release(&lock[hash]);
    }
    }
    return (void *)time;
}

void signal_handler(int sig){
    if(sig == SIGSEGV){
        fprintf(stderr,"Segfault\n");
        exit(1);
    }
}

char* getKey(){
    char* key = (char*) malloc(sizeof(char)*257);
    for(int i=0;i<256;i++){
        key[i]= (char) rand()%26 + 'a';
    }
    key[256]='\0';
    return key;
}
int main(int argc, char *argv[]){
    opt_yield=0;
    process_args(argc, argv, &threads, &iterations,&opt_yield,&lists);
    signal(SIGSEGV, signal_handler);

    if (sync_char == 'm') {
        m_lock = malloc(sizeof(pthread_mutex_t) * lists);
        if (m_lock == NULL) {
	      		fprintf(stderr, "Failed to allocate mutexs\n");
	      		exit(1);
	    }
        
        for (long i = 0; i < lists; i++) {
	      		if (pthread_mutex_init(&m_lock[i], NULL) != 0) {
		        	fprintf(stderr, "Could not create mutexes\n");
		        	exit(1);
	      		}
	    	}
    }else if (sync_char == 's') {
        lock = malloc(sizeof(int) * lists);
        if (lock == NULL) {
	      		fprintf(stderr, "Failed to allocate locks\n");
	      		exit(1);
	    }
        
        for (int i = 0; i < lists; i++) {
	      		lock[i]=0;
	    	}
    }

    elements =  (SortedListElement_t*) malloc(iterations * threads*sizeof(SortedListElement_t));
    if(elements==NULL){
        fprintf(stderr, "Can not malloc elements.\n");
		exit(1);
    }

    for(int i=0;i<iterations * threads;i++){
        elements[i].key = getKey();
    }

    pthread_t *thread_arr = malloc(threads * sizeof(pthread_t));
    if (thread_arr == NULL)
    {
        fprintf(stderr, "Error: failed to malloc thread array\r\n");
        exit(1);
    }

    //init listhead
    listhead = malloc(sizeof(SortedList_t) * lists);
    if (listhead == NULL) {
		fprintf(stderr, "Failed to allocate listhead\r\n");
		exit(1);
	}
    for (int i = 0; i < lists; i++) {
		listhead[i].next=&listhead[i];
        listhead[i].prev=&listhead[i];
        listhead[i].key=NULL;
	}
    

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < threads; i++)
    {
        //fprintf(stderr,"%s\n\n",(&elements[iterations*i])->key);
        int err =pthread_create(&thread_arr[i], NULL, &thread_worker, (void *) (&elements[iterations*i]));
        if(err!=0){
            fprintf(stderr,"err%s\n\r",strerror(errno));
        }
        
    }
    long totalWait = 0;
    void** waitTime = (void *) malloc(sizeof(void**));
    for (long i = 0; i < threads; i++)
    {
        pthread_join(thread_arr[i], waitTime);
        totalWait+=(long) *waitTime;
    }
    
    
    clock_gettime(CLOCK_MONOTONIC, &end);

    long operations = threads * iterations * 3;
    long run = 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    long avg = run / operations;
    long lockWait = totalWait/operations;

    printf("list");
    if(opt_yield==0){
       printf("-none");
    }else if (opt_yield == 1){
        printf("-i");
    }
	else if (opt_yield == 2){
	    printf("-d");
    }
	else if (opt_yield == 3){
	    printf("-id");
    }
	else if (opt_yield == 4){
        printf("-l");
    }
	else if (opt_yield == 5){
	    printf("-il"); 
    }
	else if (opt_yield == 6){
	    printf("-dl"); 
    }
	else if (opt_yield == 7){
	    printf("-idl"); 
    }

    if(sync_char=='s'){
        printf("-s,"); 
    }else if(sync_char=='m'){
        printf("-m,"); 
    }else{
        printf("-none,"); 
    }

    print_output(threads, iterations, lists, operations, run, avg,lockWait);

    if (sync_char == 'm') {
		for (long i = 0; i < lists; i++){
    	  	pthread_mutex_destroy(&m_lock[i]);
    	}
        free(m_lock);
	}else if(sync_char == 's'){
        free (lock);
    }

    for(long i = 0; i < threads*iterations; i++){
        free((void*)elements[i].key);
    }
    free(waitTime);
    free(elements);
    free(thread_arr);
    exit(0);
}

