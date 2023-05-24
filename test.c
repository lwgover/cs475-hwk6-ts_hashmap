#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_hashmap.h"

/**
 * Work for each thread. Has a 33% chance to either put, del, or get
 * @param args a void pointer to a map (needs to be casted)
 */
void* threadwork(void* args) {
    ts_hashmap_t *map = (ts_hashmap_t*) args;
    int r = 0;
    for (int i = 0; i < 100; i++) {
        r = rand() % 10;
        if (r < 3) {
            put(map, i, i);
        } else if (r < 6) {
            get(map, i);
        } else {
            del(map, i);
        }
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
        return 1;
    }

  srand(0);    // seed to 0 for repetability
    int num_threads = atoi(argv[1]);
    int capacity = (unsigned int) atoi(argv[2]);
    ts_hashmap_t *map = initmap(capacity);

    // spawn threads
    pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, threadwork, map);
    }

    // join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // print content
    printmap(map);

    // clean up
    deallocate_map(map);
    free(threads);


    return 0;
}
