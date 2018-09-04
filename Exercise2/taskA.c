#include <pthread.h>

long g_var = 0;

// Note the argument and return types: void*
void* fn(void* args){
	long var = 0;
	for (int i = 0; i < 50*1000*1000; i++) {
		++var;   // Increment local variable.
		++g_var; // Increment global variable.
	}
	printf("Global variable: %ld\r\n
			Local variable: %ld\r\n", var, g_var);

    return NULL;
}

int main(){
    pthread_t thread1;
	pthread_t thread2;	

	// Create threads
    pthread_create(&thread1, NULL, fn, NULL);
    pthread_create(&thread2, NULL, fn, NULL);

    // Wait for threads to finish execution
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
}