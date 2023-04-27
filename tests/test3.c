#include "../threads.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_barrier_t barrier;

// Function to be run that locks, prints, unlocks, sleeps, then does it again (each thread should be run twice)
void *routine(void *arg) {
	pthread_mutex_lock(&mutex);
    printf("Thread %lu has ran once!\n", pthread_self());
    pthread_mutex_unlock(&mutex);

    // Sleep for 2 seconds
    sleep(2);

    pthread_mutex_lock(&mutex);
    printf("Thread %lu has ran twice!\n", pthread_self());
    pthread_mutex_unlock(&mutex);
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
