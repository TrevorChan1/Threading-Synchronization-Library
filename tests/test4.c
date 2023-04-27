#include "../threads.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_THREADS 128

pthread_mutex_t mutex;
pthread_barrier_t barrier;

// Function to be run that locks, prints, unlocks, sleeps, then does it again (each thread should be run twice)
void *routine(void *arg) {
    for (int i = 0; i < 10; i++){
        pthread_mutex_lock(&mutex);
        printf("Thread %lu has ran %d times!\n", pthread_self(), i);
        sleep(3);

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
	
	return NULL;
}

// Main function that creates 10 threads then exits
int main() {

	pthread_t tid[10];
	pthread_mutex_init(&mutex, NULL);

    // Create 10 separate threads
    for (int i = 0; i < 10; i ++){
        pthread_create(tid + i, NULL, &routine, NULL);
    }
	pthread_exit(NULL);

	printf("this should not be printed\n");

	return 0;
}
