#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>

#include "io.h"

#define CHANNEL_A 		1
#define CHANNEL_B		2
#define CHANNEL_C 		3

#define IO_LOW			0
#define IO_HIGH			1

#define RESPONSE_THREAD_COUNT		3
#define DISTURBANCE_THREAD_COUNT	8

int channels[3] = {CHANNEL_A, CHANNEL_B, CHANNEL_C};


/// Assigning CPU core ///
int set_cpu(int cpu_number){
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);

	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

/// 'struct timespec' functions ///
struct timespec timespec_normalized(time_t sec, long nsec){
    while(nsec >= 1000000000){
        nsec -= 1000000000;
        ++sec;
    }
    while(nsec < 0){
        nsec += 1000000000;
        --sec;
    }
    return (struct timespec){sec, nsec};
}

struct timespec timespec_add(struct timespec lhs, struct timespec rhs){
    return timespec_normalized(lhs.tv_sec + rhs.tv_sec, lhs.tv_nsec + rhs.tv_nsec);
}

static void responseTask(int* args){
	int channel = *args;

	set_cpu(1);
	struct timespec waketime;
	clock_gettime(CLOCK_REALTIME, &waketime);

	struct timespec period = {.tv_sec = 0, .tv_nsec = 1*1000*1000};
	while(1){
		if(io_read(channel) == IO_LOW){
			io_write(channel, IO_LOW);
			usleep(5);
			io_write(channel, IO_HIGH);
		}
		// Sleep
	 	waketime = timespec_add(waketime, period);
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &waketime, NULL);
	}
}

static void disturbance() {
	set_cpu(1);
	while(1){
		asm volatile("" ::: "memory");
	}
}


/// Periodic execution example ///
static void periodWork(){
	struct timespec waketime;
	clock_gettime(CLOCK_REALTIME, &waketime);

	struct timespec period = {.tv_sec = 0, .tv_nsec = 500*1000*1000};

	while(1){
	    printf("tick\n");
	    
	    // sleep
	    waketime = timespec_add(waketime, period);
	    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &waketime, NULL);
	}
}

int main(){

	io_init();
    pthread_t threads [RESPONSE_THREAD_COUNT];
    pthread_t disturbance_threads [DISTURBANCE_THREAD_COUNT];

	// Create threads
	for (int i = 0; i < RESPONSE_THREAD_COUNT; i++){
		pthread_create(&(threads[i]), NULL, responseTask, &channels[i]);
	}

	// Create disturbing threads
	for (int i = 0; i < DISTURBANCE_THREAD_COUNT; i++){
		pthread_create(&(disturbance_threads[i]), NULL, disturbance, NULL);
	}

	// Wait for threads to finish execution
	for (int i = 0; i < DISTURBANCE_THREAD_COUNT; i++){
		pthread_join(disturbance_threads[i], NULL);
	}

	// Wait for threads to finish execution
	for (int i = 0; i < RESPONSE_THREAD_COUNT; i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}
