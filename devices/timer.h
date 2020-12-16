#ifndef DEVICES_TIMER_H
#define DEVICES_TIMER_H

#include <round.h>
#include <stdint.h>

/* Number of timer interrupts per second. */
#define TIMER_FREQ 100

void timer_init (void);
void timer_calibrate (void);

int64_t timer_ticks (void);
int64_t timer_elapsed (int64_t);

struct alarm_clock
{
    // time to wake up
    int64_t waking_time ;

    // thread to be awaken
    struct thread *thread ;
    
    // to add the timer in the global list of timers
    struct list_elem alarm_clock_elem;
};

bool alarm_clock_compare(struct list_elem *t1, struct list_elem *t2, void* aux);
bool alarm_clock_check(struct alarm_clock* alarm);
void alarm_clock_check_all(void);


/* Sleep and yield the CPU to other threads. */
void timer_sleep (int64_t ticks);
void timer_msleep (int64_t milliseconds);
void timer_usleep (int64_t microseconds);
void timer_nsleep (int64_t nanoseconds);

/* Busy waits. */
void timer_mdelay (int64_t milliseconds);
void timer_udelay (int64_t microseconds);
void timer_ndelay (int64_t nanoseconds);

void timer_print_stats (void);

#endif /* devices/timer.h */
