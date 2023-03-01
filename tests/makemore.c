#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// An example where it tests if pthread_create can create threads after reaches limit and exits some threads

/* How many times you're allowed to pthread_create */
#define THREAD_CNT 127

// Start_routine function that will sleep for 2 seconds if its thread id is an even number
// makes it so some, but not all threads are exited by the time main runs again
void * runThread(void *arg) {
    if ( (int)pthread_self() % 2 == 0 ){
        sleep(2);
    }

	return NULL;
}

// Main function that creates 128 threads, sleeps so some threads can exit, then tries to create some more
int main(int argc, char **argv) {

	pthread_t threads[THREAD_CNT];
	int i;

    // Create 128 threads
	for(i = 0; i < THREAD_CNT; i++) {

        // Check if pthread_create returns a -1 (fails to create thread). If so, fails test
        if (pthread_create(&threads[i], NULL, runThread, (void *) NULL) == -1){
            // Fail test case if pthread_create fails
            printf("Test case failed\n");
            return -1;
        }
	}

    // Sleep for 1 second (run through scheduler to allow some threads to be freed)
    sleep(1);
    pthread_t smoreThreads[3];

    for (int i = 0; i < 3; i++) {

        // Check if pthread_create returns a -1 (fails to create thread). If so, fails test
        if (pthread_create(&smoreThreads[i], NULL, runThread, (void *) NULL) == -1){
            // Fail test case if pthread_create fails
            printf("Test case failed\n");
            return -1;
        }
        if ((int) smoreThreads[i] < 0 || (int) smoreThreads[i] > 128){
            // Fail test case if the thread id isn't valid (assuming people set thread id's from 0 to 127 or 1 to 128)
            printf("Test case failed\n");
            return -1;
        }
    }

    // If was able to successfully create those 3 threads and those threads gave thread id's, succeed test
    printf("Test case passed\n");
    return 0;

}
