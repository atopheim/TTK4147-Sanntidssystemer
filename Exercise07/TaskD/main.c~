#include <native/task.h>
#include <native/timer.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <native/sem.h>
#include <native/mutex.h>

RT_MUTEX A, B;
RT_SEM sem1;

RT_TASK high, low;

#define 	CPU_ID				1
#define TIME_UNIT_US		50
#define TIME_UNIT_NS		TIME_UNIT_US * 1000

void busy_wait_us(unsigned long delay){	
	for(; delay > 0; delay--){
		rt_timer_spin(1000);
	}
}


void low_func(void * args) {
	rt_printf("Task LOW wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);
	//rt_printf("Task LOW resumed\r\n");

	rt_mutex_acquire(&A, TM_INFINITE); // Lock
	rt_printf("Start first LOW PRI work\r\n");
	busy_wait_us(3*TIME_UNIT_US);
	rt_printf("first LOW PRI done\r\n");	
	
	rt_printf("LOW: Attempting to lock B\r\n");	
	rt_mutex_acquire(&B, TM_INFINITE);
	rt_printf("LOW: Successfully locked B\r\n");

	rt_printf("Start second LOW PRI work\r\n");
	busy_wait_us(3*TIME_UNIT_US);
	rt_printf("second LOW PRI done\r\n");	
	
	rt_mutex_release(&B);
	rt_mutex_release(&A);							// Unlock

	rt_printf("Last LOW PRI work\r\n");
	busy_wait_us(3*TIME_UNIT_US);
	rt_printf("Last LOW PRI done\r\n");	

	rt_task_delete(NULL);
}

void high_func(void * args){
	rt_printf("Task HIGH wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);

	rt_task_sleep(1*TIME_UNIT_NS);
	
	rt_mutex_acquire(&B, TM_INFINITE);

	rt_printf("Start first HIGH PRI work\r\n");	
	busy_wait_us(1*TIME_UNIT_US);
	rt_printf("first HIGH PRI work done\r\n");
	
	rt_printf("HIGH: Attempting to lock A\r\n");
	rt_mutex_acquire(&A, TM_INFINITE);
	rt_printf("HIGH: Successfully locked A\r\n");

	rt_printf("Start second HIGH PRI work\r\n");
	busy_wait_us(2*TIME_UNIT_US);
	rt_printf("second HIGH PRI work done\r\n");	

	rt_mutex_release(&A);
	rt_mutex_release(&B);
	
	rt_printf("last HIGH PRI work\r\n");	
	busy_wait_us(1*TIME_UNIT_US);
	rt_printf("last HIGH PRI work done\r\n");
	
	rt_task_delete(NULL);
}



int main() {
  // Call these at the start of main
  rt_print_auto_init(1);
  mlockall(MCL_CURRENT | MCL_FUTURE);
	
	rt_sem_create(&sem1, "Semaphore1", 0, S_PRIO);
	rt_mutex_create(&A, "A");
	rt_mutex_create(&B, "B");
	
  rt_task_create(&high, "High", 0, 70, T_CPU(CPU_ID));
  rt_task_create(&low, "Low", 0, 50, T_CPU(CPU_ID));
	rt_task_shadow(NULL, "TaskMain", 99, T_CPU(CPU_ID));

  rt_task_start(&high, high_func, NULL);
  rt_task_start(&low, low_func, NULL);
	
	rt_task_sleep(100*1000*1000);
	
	rt_sem_broadcast(&sem1);
	
	for (int i = 0; i < 5; i++) {
		rt_task_sleep(1000*1000*1000);		
	}
	
	rt_sem_delete(&sem1);
	rt_mutex_delete(&A);
	rt_mutex_delete(&B);
	return 0;
}

  
