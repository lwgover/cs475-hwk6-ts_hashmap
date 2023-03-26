#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_hashmap.h"
#include <limits.h>

/** struct to hold parameters to test functions **/
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

        /** put */
        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) put(map,i,i);
        }
        /** get */
        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) get(map,i);
        }

        /** del */
        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) del(map,i);
        }

        /** get empty */
        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) get(map,i);
        }
	return NULL;
}

void* testPut(void* args){
        thread_args *a = (thread_args*) args;
        int tid = a->tid;
        int numThreads = a->numThreads;
        ts_hashmap_t *map = a->map;

        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) put(map,i,i);
        }
	return NULL;
}

void* testDel(void* args){
        thread_args *a = (thread_args*) args;
        int tid = a->tid;
        int numThreads = a->numThreads;
        ts_hashmap_t *map = a->map;

        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) del(map,i);
        }
	return NULL;
}
void* testAddDelAdd(void* args){
        thread_args *a = (thread_args*) args;
        int tid = a->tid;
        int numThreads = a->numThreads;
        ts_hashmap_t *map = a->map;

        /** put */
        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) put(map,i,i);
        }

        /** del */
        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) del(map,i);
        }

        /** put again */
        for(int i = 0; i < 100000; i++){
                if(i%numThreads == tid) put(map,i,i);
        }
	return NULL;
}


void run_multithread_tests(int num_threads, int capacity){
        /** make maps */
        ts_hashmap_t *multithread_map = initmap(capacity);
        ts_hashmap_t *singlethread_map = initmap(capacity);
	
	/**allocates arg for single_thread*/
	thread_args *arg = (thread_args*) malloc(sizeof(thread_args));
	arg[0].tid = 0; arg[0].numThreads = 1; arg[0].map = singlethread_map;

        /**allocates args for multi_thread*/
        thread_args *args = (thread_args*) malloc(num_threads * sizeof(thread_args));
        for (int i = 0; i < num_threads; i++) {
                args[i].tid = i;
                args[i].numThreads = num_threads;
                args[i].map = multithread_map;
        }
        /** make threads */
        pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));

        /** test put */
        for (int i = 0; i < num_threads; i++) pthread_create(&threads[i], NULL, testPut, &args[i]);
        for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
	pthread_create(&threads[0],NULL,testPut,&arg[0]);
	pthread_join(threads[0],NULL);
        printf("Test Put: %s\n",assert_equals(multithread_map,singlethread_map) ? "passed":"failed");

        /** test del */
        for (int i = 0; i < num_threads; i++) pthread_create(&threads[i], NULL, testDel, &args[i]);
        for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
	pthread_create(&threads[0],NULL,testDel,&arg[0]);
	pthread_join(threads[0],NULL);
        printf("Test Del: %s\n",assert_equals(multithread_map,singlethread_map) ? "passed":"failed");

        /** test add then delete then add */
        for (int i = 0; i < num_threads; i++) pthread_create(&threads[i], NULL, testAddDelAdd, &args[i]);
        for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
	pthread_create(&threads[0],NULL,testAddDelAdd,&arg[0]);
	pthread_join(threads[0],NULL);
        printf("Test AddDelAdd: %s\n",assert_equals(multithread_map,singlethread_map) ? "passed":"failed");

        /** test all */
        for (int i = 0; i < num_threads; i++) pthread_create(&threads[i], NULL, testAll, &args[i]);
        for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
	pthread_create(&threads[0],NULL,testAll,&arg[0]);
	pthread_join(threads[0],NULL);
        printf("Test All: %s\n",assert_equals(multithread_map,singlethread_map) ? "passed":"failed");


        /** free memory */
	deallocate_map(multithread_map);
	deallocate_map(singlethread_map);
        free(args);
        free(threads);
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
		return 1;
	}

        /** parse args */
	int num_threads = atoi(argv[1]);
	int capacity = (unsigned int) atoi(argv[2]);

        /** run tests */
	run_multithread_tests(num_threads,capacity);
}

