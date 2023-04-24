#include "ec440threads.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

/* You can support more threads. At least support this many. */
#define MAX_THREADS 128

/* Your stack should be this many bytes in size */
#define THREAD_STACK_SIZE 32767

/* Number of microseconds between scheduling events */
#define SCHEDULER_INTERVAL_USECS (50 * 1000)

/* Extracted from private libc headers. These are not part of the public
 * interface for jmp_buf.
 */
#define JB_RBX 0
#define JB_RBP 1
#define JB_R12 2
#define JB_R13 3
#define JB_R14 4
#define JB_R15 5
#define JB_RSP 6
#define JB_PC 7

/* thread_status identifies the current state of a thread. You can add, rename,
 * or delete these values. This is only a suggestion. */
enum thread_status
{
	TS_EXITED,
	TS_RUNNING,
	TS_READY
};

/* The thread control block stores information about a thread. You will
 * need one of this per thread.
 */
struct thread_control_block {
	int tid; //Thread id (will be 0 to 127)
	void * stackPtr; //Information about the stack
	enum thread_status status; //Value about thread status (0 for reading, 1 for running)
	jmp_buf currentContext; //Store jump buffer with current context information
	struct thread_control_block * nextThread;
};

// Global thread variables: TCB table with ALL current threads and the currently running thread

struct TCBTable {
	int size;
	struct thread_control_block * currentThread;
	struct thread_control_block * lastThread;
};

struct TCBTable * TCB;
void * stackToFree = NULL;
bool available[MAX_THREADS];

// SIGALRM handler that saves current context and moves onto the next function
static void schedule(int sig)
{
	// If a previous thread has exited, free the stack and set global stackToFree to NULL
	if (stackToFree != NULL){
		free(stackToFree);
		stackToFree = NULL;
	}
	
	(void) ptr_demangle;
	// Use setjmp to update currently active thread's jmp_buf, if jumped to = no scheduling
	if(sigsetjmp(TCB->currentThread->currentContext, 1) == 0){
	
		// If current thread is done, then free the thread and move on
		if(TCB->currentThread->status == TS_EXITED){
			struct thread_control_block * current = TCB->currentThread;

			stackToFree = TCB->currentThread->stackPtr;
			TCB->size--;
			available[TCB->currentThread->tid] = true;
			// If there are more threads, set up the next thread. Otherwise, do nothing.
			if (current->nextThread != NULL){
				TCB->currentThread = current->nextThread;
				TCB->currentThread->status = TS_RUNNING;

				if (TCB->currentThread->tid == 0 && TCB->size == 1){
					free(stackToFree);
					stackToFree = NULL;
				}

				free(current);
				current = NULL;
				// Initialize timer: Send SIGALRM in 50ms
				if (ualarm(SCHEDULER_INTERVAL_USECS, 0) < 0){
					printf("ERROR: Timer not set\n");
					exit(-1);
				}
				// Go to the next thread to be run
				siglongjmp(TCB->currentThread->currentContext, 1);
			}
			
			// Free the finished thread (don't need to free stack or context since those are freed in thread exit)
			free(current);
			current = NULL;
		}
		else{
			// Initialize timer: Send SIGALRM in 50ms
			if (ualarm(SCHEDULER_INTERVAL_USECS, 0) < 0){
				printf("ERROR: Timer not set\n");
				exit(-1);
			}

			TCB->currentThread->status = TS_READY;

			// If a next thread exists, set all the pointers and jump to new thread
			if (TCB->currentThread->nextThread != NULL){
				// Move from current thread to next thread and move current to last thread
				TCB->lastThread->nextThread = TCB->currentThread;
				TCB->lastThread = TCB->currentThread;
				TCB->currentThread = TCB->currentThread->nextThread;
				TCB->lastThread->nextThread = NULL;
				TCB->currentThread->status = TS_RUNNING;
				// Jump to the next thread
				siglongjmp(TCB->currentThread->currentContext, 1);
			}
			// If there is no more next threads but the current thread is not done, just keep running
		}
	}
}

// Scheduler_init function for initializing global variables and main thread
static void scheduler_init()
{
	// Allocate memory for the TCB table with MAX_THREADS entries
	TCB = (struct TCBTable *) malloc(sizeof(struct TCBTable) + 2 * sizeof(struct thread_control_block));
	TCB->size = 0;
	TCB->currentThread = (struct thread_control_block *) malloc(sizeof(struct thread_control_block));
	TCB->currentThread->nextThread = NULL;
	TCB->currentThread->stackPtr = NULL;
	TCB->currentThread->status = TS_RUNNING;
	TCB->currentThread->tid = TCB->size++;
	TCB->lastThread = TCB->currentThread;

	// Initialize array of which tid's are available
	for(int i = 0; i < MAX_THREADS; i++)
		available[i] = true;
	available[0] = false;

	// Set signal handler to schedule
	struct sigaction sigAlrmAction;
	memset(&sigAlrmAction, 0, sizeof(sigAlrmAction));
	sigemptyset(&sigAlrmAction.sa_mask);
	sigAlrmAction.sa_flags = 0;
	sigAlrmAction.sa_handler = schedule;
	sigaction(SIGALRM, &sigAlrmAction, NULL);
	
	// Initialize timer: Every 50ms sends SIGALRM
	if (ualarm(SCHEDULER_INTERVAL_USECS, 0) < 0){
		printf("ERROR: Timer not set\n");
		exit(-1);
	}
}

// Thread creation function that allocates and initializes values for a new thread.
int pthread_create(
	pthread_t *thread, const pthread_attr_t *attr,
	void *(*start_routine) (void *), void *arg)
{
	// Create the timer and handler for the scheduler. Create thread 0.
	static bool is_first_call = true;
	if (is_first_call)
	{
		is_first_call = false;
		scheduler_init();
		// Save context of main thread (current only thread), leave if longjmp'd here
		if(sigsetjmp(TCB->currentThread->currentContext, 1) != 0)
			return 0;
	}

	if (TCB->size >= MAX_THREADS){
		*thread = (pthread_t) -1;
		printf("ERROR: Max number of threads reached\n");
		return -1;
	}

	struct thread_control_block * newThread = (struct thread_control_block *) malloc(sizeof(struct thread_control_block));
	newThread->nextThread = NULL;
	newThread->status = TS_READY;

	// Iterate through all tid's and find first available one. If none are available, print error and return
	int i = 0;
	bool found = false;
	while(i < MAX_THREADS && !found){
		if(available[i]){
			newThread->tid = i;
			available[i] = false;
			found = true;
			break;
		}
		i++;
	}
	
	if(!found){
		free(newThread);
		*thread = (pthread_t) -1;
		printf("ERROR: No available threads\n");
		return -1;
	}

	// Create the stack: Dynamically allocate memory
	void * stackPtr =  malloc(THREAD_STACK_SIZE);
	*(unsigned long *) (stackPtr + THREAD_STACK_SIZE - 8) = (unsigned long) &pthread_exit;
	newThread->stackPtr = stackPtr;
	

	//ptr mangle start_thunk and the pthread_exit thing
	sigsetjmp(newThread->currentContext, 1);

	newThread->currentContext[0].__jmpbuf[JB_RSP] = ptr_mangle( (unsigned long) stackPtr + THREAD_STACK_SIZE - 8);
	newThread->currentContext[0].__jmpbuf[JB_R12] = (unsigned long) start_routine;
	newThread->currentContext[0].__jmpbuf[JB_R13] = (unsigned long) arg;
	newThread->currentContext[0].__jmpbuf[JB_PC] = ptr_mangle( (unsigned long) start_thunk );

	*thread = (pthread_t) newThread->tid;

	TCB->lastThread->nextThread = newThread;
	TCB->lastThread = newThread;
	TCB->lastThread->nextThread = NULL;
	TCB->size++;

	return 0;
}

// Exit function that runs whenever a thread has exited either implicitly or explicitly
void pthread_exit(void *value_ptr)
{
	// Cancel any current alarms
	ualarm(0,0);
	// Set the current thread's status to exited
	TCB->currentThread->status = TS_EXITED;

	// Run schedule to free values and set the next thread to be run
	schedule(0);

	// No more threads to jump to => free the linked list and exit
	free(TCB);
	exit(0);
}

// Function that returns the thread id of the currently running thread
pthread_t pthread_self(void)
{
	if (TCB->size > 0){
		// Return tid of current thread
		return (pthread_t) TCB->currentThread->tid;
	}
	return (pthread_t) -1;
}

// Mutex function that initializes the mutex values
int pthread_mutex_init(pthread_mutex_t * restrict mutex,
						const pthread_mutexattr_t * restrict attr){

}

// Mutex function used to destroy the inputted mutex
int pthread_mutex_destroy(pthread_mutex_t * mutex){

}

// Mutex function used to lock current thread or block until resource available
int pthread_mutex_lock(pthread_mutex_t * mutex){

}

// Mutex unlock function used to unlock a resource and notify first blocked
int pthread_mutex_unlock(pthread_mutex_t *mutex){

}

// Barrier initialization function that creates barrier
int pthread_barrier_init(pthread_barrier_t *restrict barrier,
						const pthread_barrierattr_t *restrict attr,
						unsigned count){
							
						}
