#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex;
long g_var = 0;

// Note the argument and return types: void*
void* fn(void* args){
	long var = 0;
	for (int i = 0; i < 10*1000*1000; i++) {
		++var;   // Increment local variable.

		// Lock semaphore before incrementing.
		sem_wait(&mutex);
		++g_var; // Increment global variable.
		sem_post(&mutex);
	}
	printf("Local variable: %ld\r\n", var);

	sem_wait(&mutex);
	printf("Global variable: %ld\r\n", g_var);
	sem_post(&mutex);

    return NULL;
}

int main(){
	// Initialize semaphore to one.
	sem_init(&mutex, 0, 1);

    pthread_t thread1, thread2;

	// Create threads
    pthread_create(&thread1, NULL, fn, NULL);
    pthread_create(&thread2, NULL, fn, NULL);

    // Wait for threads to finish execution
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    sem_destroy(&mutex);
}