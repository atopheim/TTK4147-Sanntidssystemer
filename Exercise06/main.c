#include <native/task.h>
#include <native/timer.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include "io.h"

#define NUM_TASKS 3
#define NUM_DISTURBANCES   7
#define CPU_ID    1

#define CHANNEL_A 		1
#define CHANNEL_B		2
#define CHANNEL_C 		3

#define IO_LOW			0
#define IO_HIGH			1

void responseTask(int* args) {
  // Busy-polling that won't crash your computer (hopefully)

  //rt_task_set_periodic(NULL, TM_NOW, 1*1000*1000);

  unsigned long duration = 30000000000;  // 30 second timeout
  unsigned long endTime = rt_timer_read() + duration;
	int channel = *args;

  while(1){
    if(io_read(channel) == IO_LOW){
			io_write(channel, IO_LOW);
			rt_timer_spin(5*1000);
			io_write(channel, IO_HIGH);
    }
		// Yield thread
		//rt_task_wait_period(NULL);   
		
    if(rt_timer_read() > endTime){
      rt_printf("Time expired\n");
      rt_task_delete(NULL);
    }
    if(rt_task_yield()){
      rt_printf("Task failed to yield\n");
      rt_task_delete(NULL);
    }
  }
}

// Set single CPU for pthread threads (NOT Xenomai threads!)
int set_cpu(int cpu_number){
  cpu_set_t cpu;
  CPU_ZERO(&cpu);
  CPU_SET(cpu_number, &cpu);

  return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

void disturbance(void* args) {
  set_cpu(CPU_ID);
  while(1){	
    asm volatile("" ::: "memory");
  }
}

int main() {
  // Call these at the start of main
  rt_print_auto_init(1);
  mlockall(MCL_CURRENT | MCL_FUTURE);
	io_init();

  RT_TASK tasks[NUM_TASKS];
  const char* names[NUM_TASKS] = {"TaskA", "TaskB", "TaskC"};
	int channels[3] = {CHANNEL_A, CHANNEL_B, CHANNEL_C};

  pthread_t disturbance_threads [NUM_DISTURBANCES];

  for (int i = 0; i < NUM_TASKS; i++) {
    rt_task_create(&(tasks[i]), names[i], 0, 50, T_CPU(CPU_ID));
  }

  for (int i = 0; i < NUM_TASKS; i++) {
    rt_task_start(&(tasks[i]), responseTask, &channels[i]);
  }

  // Create disturbing threads
	
  for (int i = 0; i < NUM_DISTURBANCES; i++){
    pthread_create(&(disturbance_threads[i]), NULL, disturbance, NULL);
  }

  while(1){
    sleep(-1);
  }
}
