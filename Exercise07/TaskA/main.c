#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

RT_SEM sem1;

#define 	CPU_ID		1

void func1(void *args){
	rt_printf("Task 1 wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);
	rt_printf("Task 1 resumed\r\n");
	rt_task_delete(NULL);
}

void func2(void * args) {
	rt_printf("Task 2 wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);
	rt_printf("Task 2 resumed\r\n");
	rt_task_delete(NULL);
}


int main() {
  // Call these at the start of main
  rt_print_auto_init(1);
  mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_sem_create(&sem1, "Semaphore1", 0, S_PRIO);
  RT_TASK task1, task2;
	
  rt_task_create(&task1, "Task1", 0, 60, T_CPU(CPU_ID));
  rt_task_create(&task2, "Task2", 0, 50, T_CPU(CPU_ID));
	rt_task_shadow(NULL, "TaskMain", 99, T_CPU(CPU_ID));

  rt_task_start(&task1, func1, NULL);
  rt_task_start(&task2, func2, NULL);
	
	rt_task_sleep(100*1000*1000);
	
	rt_sem_broadcast(&sem1);

	rt_task_sleep(100*1000*1000);
	
	rt_sem_delete(&sem1);

	return 0;
}

  
