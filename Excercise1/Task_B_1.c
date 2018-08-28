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

#define NANOSECOND 1000000000


int main (){
    long clk = sysconf(_SC_CLK_TCK);
    long tick_to_ns = NANOSECOND / clk;

    struct timespec now, then;
    clock_gettime(CLOCK_MONOTONIC, &then);

    int i;
    for(i = 0; i < 10*1000*1000; i++){
        // read timer
        clock_gettime(CLOCK_MONOTONIC, &now);
    }
    struct timespec diff = timespec_sub(now, then);
    long acc_time_gettime = diff.tv_nsec / i;
    
    clock_t rt;
    struct tms buf;
    rt = times(&buf);
    long init_ticks = buf.tms_stime + buf.tms_utime;
    for(i = 0; i < 10*1000*1000; i++){
        // read timer
        rt = times(&buf);
    }
    long ticks = buf.tms_stime + buf.tms_utime - init_ticks;
    long acc_time_times = (ticks * tick_to_ns) / i;



    long clk_freq = 2660000000;
    long tsc_init_ticks = __rdtsc();
    long tsc_ticks;
    // printf("tsc_init_ticks: %ld\r\n", tsc_init_ticks);
    for(i = 0; i < 10*1000*1000; i++){
        // read timer
        tsc_ticks = __rdtsc();
    }
    tsc_ticks -= tsc_init_ticks;
    long acc_time_tsc = (tsc_ticks * NANOSECOND) / 2660000000 / i; 

    // Find resolution

    struct tms buf1;

    int ns_max = 1000;
    int histogram[ns_max];
    memset(histogram, 0, sizeof(int)*ns_max);

    for(int i = 0; i < 10*1000*1000; i++){
        
        
        clock_gettime(CLOCK_MONOTONIC, &then);
        sched_yield();
        clock_gettime(CLOCK_MONOTONIC, &now);
        int ns = now.tv_nsec - then.tv_nsec;
        
        /*
        rt = times(&buf);
        rt = times(&buf1);

        init_ticks = buf.tms_stime + buf.tms_utime;
        ticks = buf1.tms_stime + buf1.tms_utime;

        int ns = (ticks - init_ticks);// * tick_to_ns;
        */

        /*
        tsc_init_ticks = __rdtsc();
        tsc_ticks = __rdtsc();

        tsc_ticks -= tsc_init_ticks;
        int ns = (tsc_ticks * NANOSECOND) / 2660000000;
        */
        
        if(ns >= 0 && ns < ns_max){
            histogram[ns]++;
        }
    }

    for(int i = 0; i < ns_max; i++){
        printf("%d\n", histogram[i]);
    }

    //printf("Access time using gettime() [ns]: %ld\r\n", acc_time_gettime);
    //printf("Access time using times() [ns]: %ld\r\n", acc_time_times);
    //printf("Access time using __rdtsc() [ns]: %ld\r\n", acc_time_tsc);
    
    return 0;
}
