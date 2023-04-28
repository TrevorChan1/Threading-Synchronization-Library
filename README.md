# Challenge 2: Threading Library

This is a projevt for EC440: Operating Systems to write a threading library similar to pthread.h to implement our own versions of pthread_create, pthread_exit, pthread_self using a round-robin style scheduling algorithm. This project is mainly consisting of a threads.c file that contains the implementations of the functions, as well as a ec440threads.h file that has some provided functions.

## Global variables
**TCB**: Linked list containing the current thread being run, the last thread in the list, and the number of threads currently open.
**stackToFree**: A void * to the last freed thread's stack so that the next call to schedule will be able to free it (necessary to prevent freeing while using)
**available**: A boolean array with 128 values. Each value corresponds to if that tid is currently available.

## scheduler_init

This function is called in the first call of pthread_create. Its purpose is to initialize the scheduler, create a thread for the main function, and set up all global variables needed. In this function, it dynamically allocates memory for a linked list of a datatype I named TCB, as well as a thread_control_block for the main thread. It initializes the jump buffer using sigsetjmp, initializes a 50ms timer, and sets the main thread to running. This function initializes the linked list to be used by the round robin scheduler.

The function also initializes the signal handler of schedule() as the signal handler for SIGALRM so that everytime the timer goes off the library will handle the round robin scheduling and clean up threads as needed.

## pthread_create

This function takes in the inputs of pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), and void *arg) where start_routine is the function to be run as the thread and arg as the arguments for that function. The purpose of this function is to create, dynamically allocate memory for, and initialize values for a new thread. The function returns 0 on a success and -1 on a failure. The thread id is returned to the pthread_t pointer thread.

To initialize the thread, the function dynamically allocates memory for its stack and creates a jump buffer that is linked to that stack memory. To do this, I put pthread_exit at the top of the stack (so it's called no matter what at the end of a thread), and sets up the initial registers like PC to run the inputted function start_routine.

The function will also check if the number of threads is over 128 (the maximum number of threads). If the thread is created, it's added to the end of the linked list at the end of the queue for the round robin scheduling

## pthread_exit

This function is called whenever a thread exits either implicitly (reaching the end of the stack after finishing the function) or explicitly (called in the function itself). The function is very simple: It resets the timer (preventing any accidental calls while in pthread_exit), sets the current thread's status to exited, and calls the schedule() signal handler.

If the status is exited in schedule(), it makes it so that it'll free the thread and set the exited thread's stack as a global stackToFree variable so that the next call of schedule will free the stack (needs to be this way to not free memory that you're currently in).

## schedule

This function is the signal handler for SIGALRM. It's called whenever the timer goes off or when a thread exits so that it can set the scheduling for the next thread to be run and jumping to it.

The function first turns off any current alarms to not be interrupted and frees any stack to be freed. It then saves the context of the current interrupted thread using sigsetjmp and, if it didn't just longjmp back to the function, it will set the next thread to be run. If current thread is exited, it will free everything necessary, set that thread id to being available, and if there is a next thread set current thread to that thread. If the status is not exited (meaning it just interrupted) then it will move onto the next thread if it exists and setting the current thread to the end of the list. If there are no more next threads it will do nothing since that'll just continue running the current thread.

### Some sources:
**http://www.csl.mtu.edu/cs4411.ck/common/Coroutines.pdf**: Used to better understand setjmp, longjmp, and how they are used for threading
**http://web.eecs.utk.edu/~jplank/plank/classes/cs360/360/notes/Setjmp/lecture.html**: Used to understand how signal handlers interact with setjmp and longjmp
**Man pages for ualarm, pthread_create, etc.**

## Synchronization
This library also implements synchronization through mutexes and barriers. The mutexes enforce that only one thread can own that specific lock at a time and the barriers ensure that a certain number of threads will have to reach the barrier before getting to continue running.

### Mutex
The Mutex functions allow for the initialization, locking, and unlocking of the mutex data type. The functions use the pthread.h library's version of the mutex data type and pads on my own data struct to be used when handling blocking.

When a thread acquires a lock, any thread that tries to acquire that same lock afterwards will be BLOCKED (taken out of the run queue). It will not be allowed to run until the owner of the lock calls unlock, in which case the first thread to block will be awoken and added back to the run queue. Blocked threads are handled via a linked list that provides the lock to the blocked threads in a FCFS fashion.

### Barrier
The barrier functions allow for the initialization of barriers with a specified number of threads to catch before allowing them all to run. The pthread_barrier_wait function can be called on a thread to be caught into the barrier. When `count` threads are caught in the barrier, the barrier will then free all of them to start running again and add them back to the run queue. All of the calls to pthread_barrier_wait will return 0 except for the last thread to enter which will return -1.