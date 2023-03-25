#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_hashmap.h"
#include <limits.h>

/** struct to hold parameters to mmm_par **/
typedef struct thread_args {
        int tid;   // thread id 
        int numThreads; //number of threads in total
        ts_hashmap_t *map; //hashmap
} thread_args;

void* testAll(void* args){
        thread_args *a = (thread_args*) args;
        int tid = a->tid;
        int numThreads = a->numThreads;
        ts_hashmap_t *map = a->map;

        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) put(map,i,i);
        }

        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) get(map,i);
        }

        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) del(map,i);
        }

        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) get(map,i);
        }
	return NULL;
}

void run_multithread_tests(int num_threads, int capacity){
        ts_hashmap_t *map = initmap(capacity);

        /**allocates args*/
        thread_args *args = (thread_args*) malloc(num_threads * sizeof(thread_args));
        for (int i = 0; i < num_threads; i++) {
                args[i].tid = i;
                args[i].numThreads = num_threads;
                args[i].map = map;
        }
        pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
        for (int i = 0; i < num_threads; i++) pthread_create(&threads[i], NULL, testAll, &args[i]);
        for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
        printmap(map);

	deallocate_map(map);
        free(args);
        free(threads);
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
		return 1;
	}


	int num_threads = atoi(argv[1]);
	int capacity = (unsigned int) atoi(argv[2]);

	run_multithread_tests(num_threads,capacity);
}

