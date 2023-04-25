#include <pthread.h>

// Thread creation function that allocates and initializes values for a new thread.
int pthread_create(
	pthread_t *thread, const pthread_attr_t *attr,
	void *(*start_routine) (void *), void *arg);

// Exit function that runs whenever a thread has exited either implicitly or explicitly
void pthread_exit(void *value_ptr);

// Function that returns the thread id of the currently running thread
pthread_t pthread_self(void);

// Mutex function that initializes the mutex values
int pthread_mutex_init(pthread_mutex_t * restrict mutex,
						const pthread_mutexattr_t * restrict attr);

// Mutex function used to destroy the inputted mutex
int pthread_mutex_destroy(pthread_mutex_t * mutex);

// Mutex function used to lock current thread or block until resource available
int pthread_mutex_lock(pthread_mutex_t * mutex);

// Mutex unlock function used to unlock a resource and notify first blocked
int pthread_mutex_unlock(pthread_mutex_t *mutex);

// Barrier initialization function that creates barrier
int pthread_barrier_init(pthread_barrier_t *restrict barrier,
						const pthread_barrierattr_t *restrict attr,
						unsigned count);

// Barrier function used to destroy current barrier
int pthread_barrier_destroy(pthread_barrier_t *barrier);

// Barrier function used to block current thread until barrier broken
int pthread_barrier_wait(pthread_barrier_t *barrier);

