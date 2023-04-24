#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// An example where it tests if pthread_create returns -1 if trying to exceed thread count

/* How many threads (aside from main) to create */
#define THREAD_CNT 128

// Generic start_routine function that just sleeps for 10 seconds then returns NULL
// Added print statement if people needed help debugging
void * runThread(void *arg) {
    printf("I'm thread %d!\n", (int) pthread_self());
	sleep(2);

	return NULL;
}

// Main function that creates 129 threads and checks if returns -1 on exceeding MAX_THREADS
int main(int argc, char **argv) {

	pthread_t threads[THREAD_CNT];
	int i;

    // Create 128 threads (should return -1 on on last one because including thread there would be 129 threads existing)
	for(i = 1; i <= THREAD_CNT; i++) {

        // Check if pthread_create returns a -1 (reaches error) when number of threads = 129
        if (pthread_create(&threads[i], NULL, runThread, (void *) NULL) == -1){
            
            if (i == 128) {

                // Passes test case if 
                printf("Test case passed\n");
                return 0;
            }
        }
	}

    // Fails test case if it's able to exceed MAX_THREAD count or didn't manage to create 128 threads
    printf("Test case failed\n");
    sleep(10);
    return -1;
}
