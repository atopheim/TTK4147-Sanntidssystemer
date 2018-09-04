#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define NUM_PHILOSOPHERS	5

pthread_mutex_t waiter;
pthread_mutex_t forks[NUM_PHILOSOPHERS];
int num_forks = sizeof(forks)/sizeof(*forks);

int g_seat_num = 0;

// Assign each philosopher a seat from 0 to NUM_PHILOSOPHERS - 1
int assign_seat() {
	int seat;
	pthread_mutex_lock(&waiter);

	// Assign seat number
	seat = g_seat_num++;

	if (g_seat_num > NUM_PHILOSOPHERS) {
		seat = -1;
	}
	pthread_mutex_unlock(&waiter);

	return seat;
}

void think(int philosopher) {
	printf("Philosopher %d is thinking.\r\n", philosopher);
	sleep(1);
}

void eat(int philosopher){
	printf("Philosopher %d is eating.\r\n", philosopher);
	sleep(2);
}

void* dining_philosophers_handler(void* args) {
	// Let waiter assign seat number to philosopher
	int philosopher = assign_seat();

	// Left fork is philosopher - 1 (wraps around)
	int l_fork = philosopher == 0 ? num_forks - 1 : philosopher - 1;
	// Right fork is philosopher
	int r_fork = philosopher;

	while (1) {
		think(philosopher);

		// Ask waiter for permission to pick up forks
		pthread_mutex_lock(&waiter);

		// Pick up left fork
	    pthread_mutex_lock(&forks[l_fork]);
		// Pick up right fork
		pthread_mutex_lock(&forks[r_fork]);
  
	    // Tell waiter philosopher has picked up both forks
	    pthread_mutex_unlock(&waiter);

    	eat(philosopher);

	    // Put down right fork
	    pthread_mutex_unlock(&forks[r_fork]);
	    // Put down left fork
	    pthread_mutex_unlock(&forks[l_fork]);
	}

	return NULL;
}

int main(){
    pthread_t philos[NUM_PHILOSOPHERS];
	int philo_size = sizeof(philos)/sizeof(*philos);

	// Init mutexes
	pthread_mutex_init(&waiter, NULL);
	for (int i = 0; i < num_forks; i++) {
    	pthread_mutex_init(forks + i, NULL);
    }

	// Create threads
	for (int i = 0; i < philo_size; i++) {
		// Let waiter assign seat/number to philosopher
		pthread_create(philos + i, NULL, dining_philosophers_handler, NULL);
	}

    // Wait for threads to finish execution
	for (int i = 0; i < philo_size; i++) {
		pthread_join(philos[i], NULL);
	}

    // Destroy mutexes
    pthread_mutex_destroy(&waiter);
	for (int i = 0; i < num_forks; i++) {
    	pthread_mutex_destroy(forks + i);
    }
}