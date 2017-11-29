#include <stdio.h>
#include "lamps_scheduler.h"

void onAlarmHook();
void lamps_scheduler_evaluate_registered_timers();
lamps_scheduler_T *lamps_scheduler_sort(lamps_scheduler_T *lamps_scheduler);
int8_t timer_compare(registered_lamp_timer_T t1, registered_lamp_timer_T t2);

extern lamps_scheduler_T lamps_scheduler;

/*
 * Returns the amount of seconds in a timer
 */
uint16_t minutes_in_timer(registered_lamp_timer_T t)
{
    return t.hours * 60 + t.minutes;
}

/*
 * Computes the initial time for timer and lamp_pin
 */
void timer_start(registered_lamp_timer_T *registered_lamp_timer, timer_T timer, uint8_t lamp_pin) {
    registered_lamp_timer->lamp_pin = lamp_pin;
    registered_lamp_timer->hours = timer.hours;
    registered_lamp_timer->minutes = timer.minutes;
    registered_lamp_timer->mode = LAMP_ON;
}

/*
 * Computes the final time for timer and lamp_pin
 */
void timer_end(registered_lamp_timer_T *registered_lamp_timer, timer_T timer, uint8_t lamp_pin) {
    registered_lamp_timer->lamp_pin = lamp_pin;
    registered_lamp_timer->hours = timer.hours + (timer.duration / 60);
    registered_lamp_timer->minutes = timer.minutes + (timer.duration % 60);
    registered_lamp_timer->mode = LAMP_OFF;
}

/*
 * Compares 2 timers:
 * returns:
 *    0: both timers are equal
 *    1: t1 > t2
 *   -1: t1 < t2
 */
int8_t timer_compare(registered_lamp_timer_T t1, registered_lamp_timer_T t2)
{
    // Disabled lamps are always bigger
    if (t2.mode == LAMP_DISABLED)
        return -1;
    uint16_t total_minutes_t1 = minutes_in_timer(t1);
    uint16_t total_minutes_t2 = minutes_in_timer(t2);
    if (total_minutes_t1 > total_minutes_t2) {
        return 1;
    }
    else if(total_minutes_t1 < total_minutes_t2) {
        return -1;
    }
    return 0;
}

void lamps_scheduler_create(lamp_timer_T *lamp_timers, uint8_t n_lamps) {
    int i = 0;
    for (i=0;i < n_lamps;i++) {
        uint8_t lamp_pin = lamp_timers[i].lamp_pin;
        timer_T *timers = lamp_timers[i].timers;
        for (int j=0;j < NTIMERS;j++) {
            int n = (i * NTIMERS * 2) + j; 
            registered_lamp_timer_T *t0 = &lamps_scheduler.registered_timers[n];
            registered_lamp_timer_T *tf = &lamps_scheduler.registered_timers[n + NTIMERS];
            timer_start(t0, timers[j], lamp_pin);
            timer_end(tf, timers[j], lamp_pin);

        }
    }
    for (i=n_lamps; i < NLAMPS;i++) {
        for (int j=0;j < NTIMERS;j++) {
            int n = (i * NTIMERS * 2) + j; 
            registered_lamp_timer_T *t0 = lamps_scheduler.registered_timers + n;
            registered_lamp_timer_T *tf = lamps_scheduler.registered_timers + n + NTIMERS;
            t0->hours = 0;
            t0->minutes = 0;
            t0->lamp_pin = 0;
            t0->mode = LAMP_DISABLED;
            tf->hours = 0;
            tf->minutes = 0;
            tf->lamp_pin = 0;
            tf->mode = LAMP_DISABLED;
        }
    }

    lamps_scheduler_sort(&lamps_scheduler);
    /* for (i=0;i<NLAMPS * NTIMERS * 2;i++) { */
    /*     registered_lamp_timer_T l = lamps_scheduler.registered_timers[i]; */
    /*     printf("%d - %d - %d:%d[%d]\n", i, l.lamp_pin, l.hours, l.minutes, l.mode); */
    /* } */
}

lamps_scheduler_T * lamps_scheduler_sort(lamps_scheduler_T *lamps_scheduler) {
    int i = 0;
    while(i++ < NLAMPS * NTIMERS) {
        int j = i;
        registered_lamp_timer_T *t1 = &lamps_scheduler->registered_timers[j-1];
        registered_lamp_timer_T *t2 = &lamps_scheduler->registered_timers[j];
        while (j-- > 0 && timer_compare(*t1, *t2) > 0) {
            registered_lamp_timer_T t3 = *t1;
            t1->hours = t2->hours;
            t1->minutes = t2->minutes;
            t1->mode = t2->mode;
            t1->lamp_pin = t2->lamp_pin;
            t2->hours = t3.hours;
            t2->minutes = t3.minutes;
            t2->mode = t3.mode;
            t2->lamp_pin = t3.lamp_pin;
            t1 = &lamps_scheduler->registered_timers[j-1];
            t2 = &lamps_scheduler->registered_timers[j];
        }
    }
    return lamps_scheduler;
}

void lamps_scheduler_evaluate_registered_timers(lamps_scheduler_T * lamps_scheduler) {
    uint8_t i = lamps_scheduler->current_timer_index;
    registered_lamp_timer_T t;
    while (i < NLAMPS * NTIMERS * 2 && lamps_scheduler->registered_timers[i].mode != LAMP_DISABLED) {
        if (lamps_scheduler->registered_timers[i].mode == LAMP_ON)
            lamps_seton(lamps_scheduler->registered_timers[i].lamp_pin);
        else
            lamps_setoff(lamps_scheduler->registered_timers[i].lamp_pin);
        i++;
    }

    if (lamps_scheduler->registered_timers[i].mode == LAMP_DISABLED || i >= NLAMPS * NTIMERS * 2) {
        lamps_scheduler->current_timer_index = 0;
    }

    t = lamps_scheduler->registered_timers[lamps_scheduler->current_timer_index];
    lamps_scheduler->alarm_id = set_alarm(t, lamps_scheduler->alarm_id, onAlarmHook);
}

void onAlarmHook() {
    lamps_scheduler_evaluate_registered_timers(&lamps_scheduler);
}

void lamps_scheduler_init(lamps_scheduler_T *lamps_scheduler) {
    registered_lamp_timer_T current_time;
    uint8_t initial_lamp_pins; 
    // Set lamps according to current_time
    for (int i=0;i<NLAMPS * NTIMERS * 2;i++) {
        registered_lamp_timer_T t = lamps_scheduler->registered_timers[i];

        // Check if this timer is expired
        if (timer_compare(current_time, t) >= 0) {
            initial_lamp_pins |= (t.mode == LAMP_ON ? 1 : 0) << i;
        } else {
            // first t in the future, next alarm
            lamps_scheduler->alarm_id = set_alarm(t, 0, onAlarmHook);
            lamps_scheduler->current_timer_index = i - 1;
            for (int j=0;j<i;j++) {
                if ((initial_lamp_pins >> j) & 1) {
                    lamps_seton(j+2);
                }
            }
            break;
        }
    }
}
