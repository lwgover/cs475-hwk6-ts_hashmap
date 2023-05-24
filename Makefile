all: main.c ts_hashmap.o
	gcc -Wall -g -o hashtest main.c ts_hashmap.o -lpthread

ts_hashmap.o: ts_hashmap.h ts_hashmap.c
	gcc -Wall -g -c ts_hashmap.c

test: test.c ts_hashmap.o
	gcc -Wall -g -o test test.c ts_hashmap.o -lpthread
clean:
	rm -f hashtest *.o
