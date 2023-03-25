#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "ts_hashmap.h"

/**
 * Initializes locks for a hashmap
 * 
 * @param capacity number of locks, corresponds to the capacity of the hashmap
 * 
*/
pthread_mutex_t** init_locks(int capacity){
	pthread_mutex_t **locks = malloc(capacity * sizeof(pthread_mutex_t*));
	for(int i = 0; i < capacity; i++){
		locks[i] = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(locks[i], NULL);
	}
	return locks;
}


/**
 * Creates a new thread-safe hashmap. 
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity) {
	ts_hashmap_t *ts_hashmap = malloc(sizeof(ts_hashmap_t));
	ts_hashmap->capacity = capacity;
	ts_hashmap->size = 0;
	ts_hashmap->locks = init_locks(capacity);

	ts_hashmap->table = malloc(capacity * sizeof(ts_entry_t));
	for (int i = 0; i < capacity; i++) { // clear out the map. Not sure if this is necessary, but it makes me feel safe
		ts_hashmap->table[i] = NULL;
	}

	return ts_hashmap;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
	if (map == NULL) { //if hashmap is empty, don't run
		return INT_MAX;
	}
	unsigned int location = hash(map, key);

	pthread_mutex_lock(map->locks[location]);
	ts_entry_t *currEntry = map->table[hash(map,key)]; // get the right spot in hashtable

	while (currEntry != NULL) {
		if (currEntry->key == key) { //key Found
			int newValue = currEntry->value;
			pthread_mutex_unlock(map->locks[location]);
			return newValue;
		}
		currEntry = currEntry->next;

	}
	pthread_mutex_unlock(map->locks[location]);
	return INT_MAX; //Not Found, return INT_MAX
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
	if (map == NULL) { // if there's no map, don't even try
		return INT_MAX;
	}

	unsigned int location = hash(map, key);

	int currValue = get(map, key);
	pthread_mutex_lock(map->locks[location]);
	if(currValue != INT_MAX){
		ts_entry_t *currEntry = map->table[hash(map,key)]; // get the right spot in hashtable

		while (currEntry != NULL) {
			if (currEntry->key == key) { //key Found
				int oldValue = currEntry->value;
				currEntry->value = value;
				pthread_mutex_unlock(map->locks[location]);
				return oldValue;
			}
			currEntry = currEntry->next;
		}
		return currValue;
	}

	/** initialize new entry*/
	ts_entry_t *newEntry = malloc(sizeof(ts_entry_t));
	newEntry->key = key;
	newEntry->value = value;
	newEntry->next = map->table[location];

	map->table[location] = newEntry;
	map->size++;

	pthread_mutex_unlock(map->locks[location]);
	return INT_MAX; //No old value was associated with this new key value pair
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {
	if (map == NULL) { // don't run on a null map, obviously
		return INT_MAX;
	}

	unsigned int location = hash(map, key);

	int currValue = get(map, key);
	if(currValue == INT_MAX){
		return INT_MAX;
	}
	pthread_mutex_lock(map->locks[location]);
	ts_entry_t *currEntry = map->table[location]; // get the first entry for this hash in hashtable

	if( currEntry->key == key){
		map->table[location] = currEntry->next;
		map->size--;
		int returnValue = currEntry->value;
		free(currEntry);
		pthread_mutex_unlock(map->locks[location]);
		return returnValue;	
	}
	ts_entry_t *last = currEntry;
	currEntry = currEntry->next;
	while (currEntry != NULL) {
		if (currEntry->key == key) { //key Found
			last->next = currEntry->next;
			map->size--;
			free(currEntry);
			pthread_mutex_unlock(map->locks[location]);
			return currEntry->value;
		}
		last = currEntry;
		currEntry = currEntry->next;

	}

	return INT_MAX; //No old value was associated with this new key value pair	

}


/**
 * @return the load factor of the given map
 */
double lf(ts_hashmap_t *map) {
  return (double) map->size / map->capacity;
}

/**
 * Prints the contents of the map
 */
void printmap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
	printf("[%d] -> ", i);
	ts_entry_t *entry = map->table[i];
	while (entry != NULL) {
	  printf("(%d,%d)", entry->key, entry->value);
	  if (entry->next != NULL)
		printf(" -> ");
	  entry = entry->next;
	}
	printf("\n");
  }
}

/**
 * Hashes a number
 */
unsigned int hash(ts_hashmap_t *map, int num){
	return ((unsigned int)num) % map->capacity;
}

/**
 * Destroys a map by freeing all elements, ts_hashmap_t struct and locks array. 
 * 
 * @param map pointer to the map to destroy
 */  

 void deallocate_map(ts_hashmap_t* map) {

	/** Free Table */
	for(int i = 0; i < map->capacity; i++){ 
		while(map->table[i] != NULL){ // while this portion of table contains at least one element
			ts_entry_t *curr = map->table[i];
			map->table[i] = curr->next;
			free(curr);
		}
	}
	free(map->table);

	/** Free Locks*/
	for(int i = 0; i < map->capacity; i++){ 
		pthread_mutex_destroy(map->locks[i]);
	}
	free(map->locks);

	/** Free entire map */
	free(map);
 } 