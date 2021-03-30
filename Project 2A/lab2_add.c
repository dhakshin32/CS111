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
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <sched.h>

long threads = 1;
long iterations = 1;
long long sum = 0;
int opt_yield;
char sync_char=0;
pthread_mutex_t lock;
int stop; 

void process_args(int argc, char *argv[], long *threads, long *iterations)
{
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
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
        case 'i':
            *iterations = atoi(optarg);
            break;
        case 'y':
            opt_yield=1;
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

void add(long long *pointer, long long value)
{
    long long val = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = val;
}
void add_m(long long *pointer, long long value) {
 	pthread_mutex_lock(&lock);
  	long long val = *pointer + value;
  	if (opt_yield) {
   		sched_yield();
	}
 	*pointer = val;
  	pthread_mutex_unlock(&lock);
}

void add_s(long long *pointer, long long value) {
	while (__sync_lock_test_and_set(&stop, 1));
	long long val = *pointer + value;
	if (opt_yield) {
    		sched_yield();
    	}
  	*pointer = val;
 	 __sync_lock_release(&stop);
}

void add_c(long long *pointer, long long value) {
	long long val;
	do {
		val = sum;
	    	if (opt_yield) {
	     		sched_yield();
	    	}
  	} while (__sync_val_compare_and_swap(pointer, val, val+value) != val);
}

void print_output(char *test, long threads, long iterations, long operations, long runtime, long avg, long long total)
{

    fprintf(stdout, "%s,%ld,%ld,%ld,%ld,%ld,%lld\n", test, threads, iterations, operations, runtime, avg, total);
}
void *thread_worker(void *arg)
{
    for (int i = 0; i < iterations; i++)
    {
        if (sync_char == 'c'){
			add_c(&sum, 1);
			add_c(&sum, -1);

		}else if (sync_char == 'm'){
			add_m(&sum, 1);
			add_m(&sum, -1);

		}else if (sync_char == 's'){
			add_s(&sum, 1);
			add_s(&sum, -1);

		}else {
			add(&sum, 1);
			add(&sum, -1);
		}
    }

    return arg;
}

int main(int argc, char *argv[])
{
    process_args(argc, argv, &threads, &iterations);

    //run start time
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    //create array of threads
    pthread_t *thread_arr = malloc(threads * sizeof(pthread_t));
    if (thread_arr == NULL)
    {
        fprintf(stderr, "Error: failed to malloc thread array\r\n");
        exit(1);
    }

    for (int i = 0; i < threads; i++)
    {
        pthread_create(&thread_arr[i], NULL, &thread_worker, NULL);
    }

    for (int i = 0; i < threads; i++)
    {
        pthread_join(thread_arr[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    long operations = threads * iterations * 2;
    long run = 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    long avg = run / operations;

    if (opt_yield == 1 && sync_char == 'c'){
        print_output("add-yield-c", threads, iterations, operations, run, avg, sum);
    }
	else if (opt_yield == 1 && sync_char == 'm'){
	    print_output("add-yield-m", threads, iterations, operations, run, avg, sum);
    }
	else if (opt_yield == 1 && sync_char == 's'){
	    print_output("add-yield-s", threads, iterations, operations, run, avg, sum);
    }
	else if (opt_yield == 1 && sync_char == 0){
        print_output("add-yield-none", threads, iterations, operations, run, avg, sum);
    }
	else if (opt_yield == 0 && sync_char == 'm'){
	    print_output("add-m", threads, iterations, operations, run, avg, sum);
    }
	else if (opt_yield == 0 && sync_char == 's'){
	    print_output("add-s", threads, iterations, operations, run, avg, sum);
    }
	else if (opt_yield == 0 && sync_char == 'c'){
	    print_output("add-c", threads, iterations, operations, run, avg, sum);
    }
	else{
	    print_output("add-none", threads, iterations, operations, run, avg, sum);
    }


    free(thread_arr);
}