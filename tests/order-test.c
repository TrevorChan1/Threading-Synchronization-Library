#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>

#define THREAD_COUNT 9

// From Shell test files given to us 
#define TEST_ASSERT(x) do {			\
  if (!(x)) { \
  fprintf(stderr, "%s:%d: Assertion (%s) failed!\n", \
	  __FILE__, __LINE__, #x); \
  abort(); \
  } \
  } while(0)


pthread_mutex_t mutex;

pthread_t lock_order[THREAD_COUNT];
int lock_index = 0;

int time_waste = 100000000;
int result = 0;
int expected_result = 90;
/*
  Add += 10
  Sub -= 1
  Mul *= 5
  Div /= 2
  Thread Arithmetic Operations in Round Robin Order: Add -> Div -> Mul -> Sub -> Sub -> Add -> Div -> Mul -> Add 
                Expected Result:  Result Starts @ 0  +10    /2     *5     -1     -1     +10    /2     *5     +10  =  90
  Running these arithmetic operations in any other order (meaning mutex does not run round robin) Has a very high prob of producing a different result
  from the expected result each time run.
*/

void *add(void *arg) {
  pthread_mutex_lock(&mutex);
  printf("Hey I am thread #%lu! I got the lock! \n", pthread_self());
  lock_order[lock_index] = pthread_self();
  lock_index++;

  result += 10;
  
  int i = 0;
  while (i < time_waste) {
    i++;
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void *sub(void *arg) {
  pthread_mutex_lock(&mutex);
  printf("Hey I am thread #%lu! I got the lock! \n", pthread_self());
  lock_order[lock_index] = pthread_self();
  lock_index++;

  result -= 1;

  int i = 0;
  while (i < time_waste) {
    i++;
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void *mul(void *arg) {
  pthread_mutex_lock(&mutex);
  printf("Hey I am thread #%lu! I got the lock! \n", pthread_self());
  lock_order[lock_index] = pthread_self();
  lock_index++;

  result = result * 5;

  int i = 0;
  while (i < time_waste) {
    i++;
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void *divi(void *arg) {
  pthread_mutex_lock(&mutex);
  printf("Hey I am thread #%lu! I got the lock! \n", pthread_self());
  lock_order[lock_index] = pthread_self();
  lock_index++;

  result = result / 2;

  int i = 0;
  while (i < time_waste) {
    i++;
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main() {

  pthread_t tid[THREAD_COUNT];

  pthread_mutex_init(&mutex, NULL);

  pthread_create(&tid[0], NULL, &add, NULL);
  pthread_create(&tid[1], NULL, &divi, NULL);
  pthread_create(&tid[2], NULL, &mul, NULL);
  pthread_create(&tid[3], NULL, &sub, NULL);
  pthread_create(&tid[4], NULL, &sub, NULL);
  pthread_create(&tid[5], NULL, &add, NULL);
  pthread_create(&tid[6], NULL, &divi, NULL);
  pthread_create(&tid[7], NULL, &mul, NULL);
  pthread_create(&tid[8], NULL, &add, NULL);
  
  int j = 0;
  while (j < 1000000000) {
    j++;
  }

  printf("If the threads received the mutex in round-robin order, they would have received in the order 123456789\n");
  printf("The threads really received the mutex in the order ");
  for (int k=0; k < THREAD_COUNT; k++) {
    printf("%lu", lock_order[k]);
  }
  printf("\n");

  printf("Here is the count: %d\n", result);
  printf("Expected Result: %d\n", expected_result);

  TEST_ASSERT(result == expected_result);
  return 0;
}