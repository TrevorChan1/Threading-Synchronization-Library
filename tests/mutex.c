#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THEARD 4
#define NUM_MUTEX 5

pthread_mutex_t mutex[NUM_MUTEX];
pthread_t threads[NUM_THEARD];

void *foo() {
    pthread_t tid = pthread_self();

    if (tid == 1) {
        printf("thread %ld waiting for 0\n", tid);
        pthread_mutex_lock(&mutex[0]);
        pthread_mutex_unlock(&mutex[0]);
        printf("thread %ld done!\n", tid);
    } else if (tid == 2) {
        printf("thread %ld waiting for 0\n", tid);
        pthread_mutex_lock(&mutex[0]);
        pthread_mutex_unlock(&mutex[0]);
        printf("thread %ld done!\n", tid);
    } else if (tid == 3) {
        printf("thread %ld waiting for 1 and 2\n", tid);
        pthread_mutex_lock(&mutex[1]);
        pthread_mutex_unlock(&mutex[1]);
        pthread_mutex_lock(&mutex[2]);
        pthread_mutex_unlock(&mutex[2]);
        printf("thread %ld done!\n", tid);
    } else {
        printf("thread %ld waiting for 2 and 3\n", tid);
        pthread_mutex_lock(&mutex[2]);
        pthread_mutex_unlock(&mutex[2]);
        pthread_mutex_lock(&mutex[3]);
        pthread_mutex_unlock(&mutex[3]);
        printf("thread %ld done!\n", tid);
    }
    pthread_mutex_unlock(&mutex[tid]);
    return NULL;
}

int main(int argc, char **argv) {
    printf("here\n");
    for (int i = 0; i < NUM_MUTEX; i++) {
        if (pthread_mutex_init(&mutex[i], NULL) != 0) {
            printf("mutex init failed\n");
        }
    }
    printf("here\n");
    for (int i = 0; i < NUM_MUTEX; i++) {
        if (pthread_mutex_lock(&mutex[i]) != 0) {
            printf("mutex lock failed\n");
        }
    }
    printf("here\n");
    for (int i = 0; i < NUM_THEARD; i++) {
        if (pthread_create(&threads[i], NULL, foo, NULL) != 0) {
            printf("mutex create failed\n");
        }
    }
    printf("here\n");

    sleep(2);

    pthread_mutex_unlock(&mutex[pthread_self()]);

    printf("thread %ld waiting for 4\n", pthread_self());
    pthread_mutex_lock(&mutex[4]);
    pthread_mutex_unlock(&mutex[4]);

    printf("DONE!\n");
    return 0;
}