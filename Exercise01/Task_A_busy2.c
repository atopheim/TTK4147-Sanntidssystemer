#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/times.h>


#include <x86intrin.h>

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

struct timespec timespec_sub(struct timespec lhs, struct timespec rhs){
    return timespec_normalized(lhs.tv_sec - rhs.tv_sec, lhs.tv_nsec - rhs.tv_nsec);
}

struct timespec timespec_add(struct timespec lhs, struct timespec rhs){
    return timespec_normalized(lhs.tv_sec + rhs.tv_sec, lhs.tv_nsec + rhs.tv_nsec);
}

int timespec_cmp(struct timespec lhs, struct timespec rhs){
    if (lhs.tv_sec < rhs.tv_sec)
        return -1;
    if (lhs.tv_sec > rhs.tv_sec)
        return 1;
    return lhs.tv_nsec - rhs.tv_nsec;
}



void busy_wait(int num_sec){ 
    long clk = sysconf(_SC_CLK_TCK); 
    printf("clk = %ld\r\n", clk);
    clock_t rt;
    struct tms buf;
    int stime_sec;   
 
    rt = times(&buf);
   
    while((buf.tms_stime + buf.tms_utime) / clk  < 1){ 
  //      for(int i = 0; i < 10000; i++){} 
        rt = times(&buf); 
    } 
} 
int main (){

   busy_wait(1);
   return 0;
}
