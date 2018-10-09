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

struct ICPP_mutex {
	RT_MUTEX mutex;
	unsigned int prio;
}; 

struct ICPP_task {
	RT_TASK task;
	unsigned int base_prio;
	unsigned int lock_count;
};

struct ICPP_mutex A, B;

RT_SEM sem1;

struct ICPP_task high, low;

#define 	CPU_ID				1
#define TIME_UNIT_US		50
#define TIME_UNIT_NS		TIME_UNIT_US * 1000

void busy_wait_us(unsigned long delay){	
	for(; delay > 0; delay--){
		rt_timer_spin(1000);
	}
}

void icpp_lock(struct ICPP_mutex mtx, struct ICPP_task *tsk) {
	rt_task_set_priority(&(tsk->task), mtx.prio);	
	tsk->lock_count++;
	rt_mutex_acquire(&mtx.mutex, TM_INFINITE); // Lock
}

void icpp_unlock(struct ICPP_mutex mtx, struct ICPP_task *tsk) {
	rt_mutex_release(&mtx.mutex);
	
	if (tsk->lock_count <= 1) { // Last lock
		rt_printf("Reverting to base priority\r\n");
		rt_task_set_priority(&(tsk->task), tsk->base_prio);	
		tsk->lock_count = 0;
	} else {
		tsk->lock_count--;
	}
}


void low_func(void * args) {
	rt_printf("Task LOW wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);
	//rt_printf("Task LOW resumed\r\n");
	
	
	rt_printf("LOW: Attempting to lock A\r\n");
	icpp_lock(A, &low);
	rt_printf("LOW: Successfully locked A\r\n");

	rt_printf("Start first LOW PRI work\r\n");
	busy_wait_us(3*TIME_UNIT_US);
	rt_printf("first LOW PRI done\r\n");	
	
	rt_printf("LOW: Attempting to lock B\r\n");
	icpp_lock(B, &low);
	rt_printf("LOW: Successfully locked B\r\n");

	rt_printf("Start second LOW PRI work\r\n");
	busy_wait_us(3*TIME_UNIT_US);
	rt_printf("second LOW PRI done\r\n");	
	
	rt_printf("LOW: Unlocking B\r\n");
	icpp_unlock(B, &low);
	rt_printf("LOW: Unlocking A\r\n");
	icpp_unlock(A, &low);			// Unlock

	rt_printf("Last LOW PRI work\r\n");
	busy_wait_us(3*TIME_UNIT_US);
	rt_printf("Last LOW PRI done\r\n");	

	rt_task_delete(NULL);
}

void high_func(void * args){
	rt_printf("Task HIGH wait\r\n");
	rt_sem_p(&sem1, TM_INFINITE);

	rt_task_sleep(1*TIME_UNIT_NS);
	
	rt_printf("HIGH: Attempting to lock B\r\n");
	icpp_lock(B, &high);
	rt_printf("HIGH: Successfully locked B\r\n");

	rt_printf("Start first HIGH PRI work\r\n");	
	busy_wait_us(1*TIME_UNIT_US);
	rt_printf("first HIGH PRI work done\r\n");
	
	rt_printf("HIGH: Attempting to lock A\r\n");
	icpp_lock(A, &high);
	rt_printf("HIGH: Successfully locked A\r\n");

	rt_printf("Start second HIGH PRI work\r\n");
	busy_wait_us(2*TIME_UNIT_US);
	rt_printf("second HIGH PRI work done\r\n");	

	rt_printf("HIGH: Unlocking A\r\n");
	icpp_unlock(A, &high);
	rt_printf("HIGH: Unlocking B\r\n");
	icpp_unlock(B, &high);
	
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
	
	A.prio = 90;
	B.prio = 90;
	rt_mutex_create(&(A.mutex), "A");
	rt_mutex_create(&(B.mutex), "B");
	
	high.base_prio = 70;
	high.lock_count = 0;
	low.base_prio  = 50;
	low.lock_count = 0;
	
  rt_task_create(&(high.task), "High", 0, high.base_prio, T_CPU(CPU_ID));
  rt_task_create(&(low.task) , "Low" , 0, low.base_prio , T_CPU(CPU_ID));
	rt_task_shadow(NULL, "TaskMain", 99, T_CPU(CPU_ID));

  rt_task_start(&(high.task), high_func, NULL);
  rt_task_start(&(low.task), low_func, NULL);
	
	rt_task_sleep(100*1000*1000);
	
	rt_sem_broadcast(&sem1);
	
	for (int i = 0; i < 5; i++) {
		rt_task_sleep(1000*1000*1000);		
	}
	
	rt_sem_delete(&sem1);
	rt_mutex_delete(&A.mutex);
	rt_mutex_delete(&B.mutex);
	return 0;
}

  
