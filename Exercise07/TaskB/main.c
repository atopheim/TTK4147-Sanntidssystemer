#include <native/task.h>
#include <native/timer.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <native/sem.h>

RT_SEM sem1, resource;

#define 	CPU_ID				1
#define TIME_UNIT_US		50
#define TIME_UNIT_NS		TIME_UNIT_US * 1000

void busy_wait_us(unsigned long delay){	
	for(; delay > 0; delay--){
		rt_timer_spin(1000);
	}
}

void func1(void * args){
	rt_printf("Task HIGH wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);
	//rt_printf("Task HIGH resumed\r\n");

	rt_task_sleep(2*TIME_UNIT_NS);
	rt_sem_p(&resource, TM_INFINITE);
	rt_printf("Start HIGH PRI work\r\n");
	busy_wait_us(2*TIME_UNIT_US);
	rt_printf("HIGH PRI work done\r\n");	
	rt_sem_v(&resource);
	rt_task_delete(NULL);
}

void func2(void * args) {
	rt_printf("Task MED wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);
	//rt_printf("Task MED resumed\r\n");

	rt_task_sleep(TIME_UNIT_NS);
	rt_printf("Start MED PRI work\r\n");
	busy_wait_us(5*TIME_UNIT_US);
	rt_printf("MED PRI work done\r\n");
	rt_task_delete(NULL);
}

void func3(void * args) {
	rt_printf("Task LOW wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);
	//rt_printf("Task LOW resumed\r\n");

	rt_sem_p(&resource, TM_INFINITE); // Lock
	rt_printf("Start LOW PRI work\r\n");
	busy_wait_us(3*TIME_UNIT_US);
	rt_printf("LOW PRI done\r\n");	
	rt_sem_v(&resource);							// Unlock
	rt_task_delete(NULL);
}


int main() {
  // Call these at the start of main
  rt_print_auto_init(1);
  mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_sem_create(&sem1, "Semaphore1", 0, S_PRIO);
	rt_sem_create(&resource, "PreciousResource", 1, S_PRIO);
  RT_TASK task1, task2, task3;
	
  rt_task_create(&task1, "Task1", 0, 70, T_CPU(CPU_ID));
  rt_task_create(&task2, "Task2", 0, 50, T_CPU(CPU_ID));
	rt_task_create(&task3, "Task3", 0, 30, T_CPU(CPU_ID));
	rt_task_shadow(NULL, "TaskMain", 99, T_CPU(CPU_ID));

  rt_task_start(&task1, func1, NULL);
  rt_task_start(&task2, func2, NULL);
  rt_task_start(&task3, func3, NULL);
	
	rt_task_sleep(100*1000*1000);
	
	rt_sem_broadcast(&sem1);

	rt_task_sleep(1000*1000*1000);
	
	rt_sem_delete(&sem1);
	rt_sem_delete(&resource);
	return 0;
}

  
