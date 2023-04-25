#include "../threads.h"
#include <stdio.h>
#include <sys/types.h>

pthread_mutex_t mutex;

void *add(void *arg) {
	pthread_mutex_lock(&mutex);
	int *money = (int *) arg;
	for (unsigned int i = 0; i < 1000000000; ++i) {
		if (i % 10000000 == 0) {
			*money += 1;
			printf("add: money = %u\n", *money);
		}
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void *sub(void *arg) {
	pthread_mutex_lock(&mutex);
	int *money = (int *) arg;
	for (unsigned int i = 0; i < 1000000000; ++i) {
		if (i % 10000000 == 0) {
			*money -= 1;
			printf("sub: money = %u\n", *money);
		}
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main() {

	pthread_t tid[2];

	pthread_mutex_init(&mutex, NULL);

	int money = 0;

	pthread_create(tid, NULL, &add, &money);
	pthread_create(tid+1, NULL, &sub, &money);

	pthread_exit(NULL);

	printf("this line should not be printed\n");

	return 0;
}
