#include "../threads.h"
#include <stdio.h>
#include <sys/types.h>

pthread_mutex_t mutex;
pthread_barrier_t barrier;

void *routine1(void *arg) {
	for (unsigned int i = 0; i < 1000000000; ++i) {
		if (i % 100000000 == 0) {
			printf("%lu says %u\n", pthread_self(), i);
			printf("%u\n", pthread_barrier_wait(&barrier));
		}
	}
	return NULL;
}

void *routine2(void *arg) {
	for (unsigned int i = 0; i < 1000000000; ++i) {
		if (i % 100000000 == 0) {
			printf("%lu says %u\n", pthread_self(), i);
			printf("%u\n", pthread_barrier_wait(&barrier));
		}
	}
	return NULL;
}

int main() {

	pthread_t tid[2];

	pthread_create(tid, NULL, &routine1, NULL);
	pthread_create(tid+1, NULL, &routine2, NULL);
	pthread_barrier_init(&barrier, NULL, 2);

	pthread_exit(NULL);

	printf("this should not be printed\n");

	return 0;
}
