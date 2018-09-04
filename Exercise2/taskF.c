#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

pthread_mutex_t waiter;
pthread_mutex_t forks[5];
int forks_size = sizeof(forks)/sizeof(*forks);

void think(int philosopher) {
	printf("Philosopher %d is thinking.\r\n", philosopher);
	sleep(1);
}

void eat(int philosopher){
	printf("Philosopher %d is eating.\r\n", philosopher);
	sleep(3);
}

void* dining_philosophers_handler(void* args) {
	// Args is philosopher number
	int philosopher = *((int*) args);
	int l_fork_i = (args == 0) ? forks_size - 1 : philosopher - 1;
	int r_fork_i = philosopher;

	while (1) {
		think(philosopher);

		// Ask waiter for permission to pick up forks
		pthread_mutex_lock(&waiter);

		// Pick up left fork
	    pthread_mutex_lock(forks + l_fork_i);
		// Pick up right fork
		pthread_mutex_lock(forks + r_fork_i);
  
    	eat(philosopher);

	    // Put down right fork
	    pthread_mutex_unlock(forks + r_fork_i);
	    // Put down left fork
	    pthread_mutex_unlock(forks + l_fork_i);

	    // Philosopher is done eating
	    pthread_mutex_unlock(&waiter);
	}


}

int main(){
    pthread_t philos[5];
	int philo_size = sizeof(philos)/sizeof(*philos);

	// Init mutexes
	pthread_mutex_init(&waiter, NULL);
	for (int i = 0; i < forks_size; i++) {
    	pthread_mutex_init(forks + i, NULL);
    }

	// Create threads
	for (int i = 0; i < philo_size; i++) {
		pthread_create(philos + i, NULL, dining_philosophers_handler, &i);
	}

    // Wait for threads to finish execution
	for (int i = 0; i < philo_size; i++) {
		pthread_join(philos[i], NULL);
	}

    // Destroy mutexes
    pthread_mutex_destroy(&waiter);
	for (int i = 0; i < forks_size; i++) {
    	pthread_mutex_destroy(forks + i);
    }
}