#include "ec440threads.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <unistd.h>

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
	pthread_t tid; //Thread id (will be 0 to 127)
	void * stackPtr; //Information about the stack
	enum thread_status status; //Value about thread status (0 for reading, 1 for running)
	jmp_buf currentContext; //Store jump buffer with current context information
	struct thread_control_block * nextThread;
};

struct thread_control_block *TCBTable[MAX_THREADS];
int front = -1;
int end = -1;

// Enqueue function for Queue of TCB's
void enQueue(struct thread_control_block *thread){
	// If the number of threads is already 128, print error
	if (end == MAX_THREADS -1)
		printf("ERROR: Maximum number of threads reached");
	// Set front to be 0 if first time queueing
	if (front == -1)
		front = 0;
	TCBTable[++end] = thread;
}

// Dequeue function for Queue of TCB's
void deQueue(){
	// If trying to remove from empty, print error
	if (front == -1)
		print("ERROR: No threads open\n");
	free_thread(TCBTable[front]);
	// If current is at the end, reinitialize all values to -1
	if (++front > end){
		front = -1;
		end = -1;
	}
	
}


// // Function to free a thread
// void free_thread(struct thread_control_block * thread){
// 	free(thread->stackPtr);
// 	free(thread->currentContext);
// 	free(thread);
// }

// Global thread variables: TCB table with ALL current threads and the currently running thread
struct TCBTable {
	int size;
	struct thread_control_block * currentThread;
	struct thread_control_block * lastThread;
};

struct TCBTable * TCB;
void * stackToFree = NULL;

// SIGALRM handler that saves current context and moves onto the next function
static void schedule(int signal)
{
	// If a previous thread has exited, free the stack and set global stackToFree to NULL
	if (stackToFree){
		free(stackToFree);
		stackToFree = NULL;
	}

	// Use setjmp to update currently active thread's jmp_buf
	setjmp(TCB->currentThread->currentContext);

	// If current thread is done, then free the thread and move on
	if(TCB->currentThread->status == TS_EXITED){
		struct thread_control_block * current = TCB->currentThread;

		stackToFree = TCB->currentThread->stackPtr;

		// If there are more threads, set up the next thread. Otherwise, do nothing.
		if (current->nextThread != NULL)
			TCB->currentThread = current->nextThread;
		
		// Free the finished thread (don't need to free stack or context since those are freed in thread exit)
		free(current->currentContext);
		free(current);
	}
	else{
		// If a next thread exists, set all the pointers and jump to new thread
		if (TCB->currentThread->nextThread != NULL){
			// Move from current thread to next thread and move current to last thread
			TCB->lastThread->nextThread = TCB->currentThread;
			TCB->lastThread = TCB->currentThread;
			TCB->lastThread->nextThread = NULL;
			TCB->currentThread = TCB->currentThread->nextThread;

			// Jump to the next thread
			longjmp(TCB->currentThread->currentContext, 1);
		}
		// If there is no more next threads but the current thread is not done, just keep running
	}

}

/* TODO: do everything that is needed to initialize your scheduler. For example:
* - Allocate/initialize global threading data structures
* - Create a TCB for the main thread. Note: This is less complicated
*   than the TCBs you create for all other threads. In this case, your
*   current stack and registers are already exactly what they need to be!
*   Just make sure they are correctly referenced in your TCB.
* - Set up your timers to call schedule() at a 50 ms interval (SCHEDULER_INTERVAL_USECS)
*/
static void scheduler_init()
{

	// Allocate memory for the TCB table with MAX_THREADS entries
	TCB = (struct TCBTable *) malloc(sizeof(struct TCBTable) + sizeof(struct thread_control_block) * MAX_THREADS);
	TCB->size = 0;
	TCB->currentThread = (struct thread_control_block *) malloc(sizeof(struct thread_control_block));
	TCB->currentThread->nextThread = NULL;
	TCB->currentThread->nextThread->status = TS_RUNNING;
	
	// Set signal handler to schedule
	struct sigaction sigAlrmAction;
	memset(&sigAlrmAction, 0, sizeof(sigAlrmAction));
	sigemptyset(&sigAlrmAction.sa_mask);
	sigAlrmAction.sa_flags = 0;
	sigAlrmAction.sa_handler = schedule;
	sigaction(SIGCHLD, &sigAlrmAction, NULL);
	
	// Initialize timer: Every 50ms sends SIGALRM
	if (ualarm(SCHEDULER_INTERVAL_USECS, SCHEDULER_INTERVAL_USECS) < 0){
		printf("ERROR: Timer not set\n");
		exit(-1);
	}

}

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
	}



	// Create the stack: Dynamically allocate memory
	void * stackPtr =  malloc(THREAD_STACK_SIZE);
	*(unsigned long *) (stackPtr + THREAD_STACK_SIZE - 8) = (unsigned long int) &pthread_exit;
	
	//ptr mangle start_thunk and the pthread_exit thing

	jmp_buf threadBuf;
	setjmp(threadBuf);
	TCB->currentThread->currentContext.__jmpbuf[JB_R12] = 6;
	RSP and pc use, and ptr to start routine

	/* TODO: Return 0 on successful thread creation, non-zero for an error.
	 *       Be sure to set *thread on success.
	 * Hints:
	 * The general purpose is to create a TCB:
	 * - Create a stack.
	 * - Assign the stack pointer in the thread's registers. Important: where
	 *   within the stack should the stack pointer be? It may help to draw
	 *   an empty stack diagram to answer that question.
	 * - Assign the program counter in the thread's registers.
	 * - Wait... HOW can you assign registers of that new stack? 
	 *   1. call setjmp() to initialize a jmp_buf with your current thread
	 *   2. modify the internal data in that jmp_buf to create a new thread environment
	 *      env->__jmpbuf[JB_...] = ...
	 *      See the additional note about registers below
	 *   3. Later, when your scheduler runs, it will longjmp using your
	 *      modified thread environment, which will apply all the changes
	 *      you made here.
	 * - Remember to set your new thread as TS_READY, but only  after you
	 *   have initialized everything for the new thread.
	 * - Optionally: run your scheduler immediately (can also wait for the
	 *   next scheduling event).
	 */
	/*
	 * Setting registers for a new thread:
	 * When creating a new thread that will begin in start_routine, we
	 * also need to ensure that `arg` is passed to the start_routine.
	 * We cannot simply store `arg` in a register and set PC=start_routine.
	 * This is because the AMD64 calling convention keeps the first arg in
	 * the EDI register, which is not a register we control in jmp_buf.
	 * We provide a start_thunk function that copies R13 to RDI then jumps
	 * to R12, effectively calling function_at_R12(value_in_R13). So
	 * you can call your start routine with the given argument by setting
	 * your new thread's PC to be ptr_mangle(start_thunk), and properly
	 * assigning R12 and R13.
	 *
	 * Don't forget to assign RSP too! Functions know where to
	 * return after they finish based on the calling convention (AMD64 in
	 * our case). The address to return to after finishing start_routine
	 * should be the first thing you push on your stack.
	 */
	return -1;
}

/* TODO: Exit the current thread instead of exiting the entire process.
	 * Hints:
	 * - Release all resources for the current thread. CAREFUL though.
	 *   If you free() the currently-in-use stack then do something like
	 *   call a function or add/remove variables from the stack, bad things
	 *   can happen.
	 * - Update the thread's status to indicate that it has exited
	 */
void pthread_exit(void *value_ptr)
{
	// Set the current thread's status to exited
	TCB->currentThread->status = TS_EXITED;

	// Run schedule to free values and set the next thread to be run
	schedule(0);

	// Exit with value 1	
	exit(1);
}

pthread_t pthread_self(void)
{

	/* TODO: Return the current thread instead of -1
	 * Hint: this function can be implemented in one line, by returning
	 * a specific variable instead of -1.
	 */
	return TCB->currentThread->tid;
}
