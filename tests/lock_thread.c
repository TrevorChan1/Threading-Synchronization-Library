#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <../threads.h>


/*
This test inits a mutex lock and starts two threads.
Both threads start by taking on the lock, count un to half-way, releases the lock, then continues counting until finish
You shuold see that at the start, one thread will take the lock first, and the second thread will immediately get blocked
You should initially only see print statements from the thread that first took the lock.
After the first thread relesaes the lock, the second thread should now be scheduled normally
Both threads also change a shared global variable "changeme" so you can observe the exclusivity property
*/
/* How many threads (aside from main) to create */
#define THREAD_CNT 1

/* pthread_join is not implemented in homework 2 */
#define HAVE_PTHREAD_JOIN 0

#define COUNTER_FACTOR 100000000

int changeme = 0; // Global data structure to change
pthread_mutex_t * lock; // Global lock

void *count(void *arg) {
  unsigned long int c = (unsigned long int) arg;    
    
    // Get lock, print out changeme, change it to pid * 10, wait by counting, exit
    pthread_mutex_lock(lock);
    printf("id: 0x%ld read changeme as %d\n", pthread_self(), changeme);
    changeme = pthread_self() * 10;
    printf("id: 0x%ld changed changeme to %d\n", pthread_self(), changeme);
    
    // struct timespec time_sleep = {.tv_nsec = 50000000};
    
    // Waste time
    int i;
  for (i = 0; i < c/2; i++) {
    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %ld\n",
             pthread_self(), i, c);
            // sleep(10);
            // nanosleep(&time_sleep, NULL);
        }
  }
    
    // Exit critical region
    pthread_mutex_unlock(lock);
    
    // Waste some more time to see normal scheduling in action
  for (i = i; i < c; i++) {
    if ((i % 10000000) == 0) {
      printf("id: 0x%lx counted to %d of %ld\n",
             pthread_self(), i, c);
    }
  }

  return NULL;
}

int main(int argc, char **argv) {
  pthread_t threads[THREAD_CNT];
    lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(lock, NULL); // Init lock
  int i;
  for(i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, count,
                   (void *)(intptr_t)((i + 2) * COUNTER_FACTOR));
  }

#if HAVE_PTHREAD_JOIN == 0
  
    // main thread will also change changeme after making thread. main should be blocked when second thread in crit region
  count((void *)(intptr_t)((i + 2) * COUNTER_FACTOR));
    // Destroy lock
    pthread_mutex_destroy(lock);
#else
  /* Collect statuses of the other threads, waiting for them to finish */
  for(i = 0; i < THREAD_CNT; i++) {
    pthread_join(threads[i], NULL);
  }
#endif
  return 0;
}
