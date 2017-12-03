#include <stdio.h>
#include "lamps_scheduler.h"

static void lamps_scheduler_evaluate_registered_timers();
static lamps_scheduler_T *lamps_scheduler_sort(lamps_scheduler_T *lamps_scheduler);
static int8_t timer_compare(registered_lamp_timer_T t1, registered_lamp_timer_T t2);
static uint16_t minutes_in_timer(registered_lamp_timer_T t);
static void timer_start(registered_lamp_timer_T *registered_lamp_timer_T, timer_T timer_T, uint8_t lamp_pin);
static void timer_end(registered_lamp_timer_T *registered_lamp_timer_T, timer_T timer_T, uint8_t lamp_pin);
static void onAlarmHook();
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
    if (timer.duration == 0)
        registered_lamp_timer->mode = LAMP_DISABLED;
    else
        registered_lamp_timer->mode = LAMP_ON;
}

/*
 * Computes the final time for timer and lamp_pin
 */
void timer_end(registered_lamp_timer_T *registered_lamp_timer, timer_T timer, uint8_t lamp_pin) {
    registered_lamp_timer->lamp_pin = lamp_pin;
    registered_lamp_timer->hours = timer.hours + (timer.duration / 60);
    registered_lamp_timer->minutes = timer.minutes + (timer.duration % 60);
    if (timer.duration == 0)
        registered_lamp_timer->mode = LAMP_DISABLED;
    else
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
    if (t1.mode == LAMP_DISABLED && t2.mode != LAMP_DISABLED)
        return 1;
    else if (t1.mode != LAMP_DISABLED && t2.mode == LAMP_DISABLED)
        return -1;
    else if (t1.mode == LAMP_DISABLED && t2.mode == LAMP_DISABLED)
        return 0;
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

lamps_scheduler_T *lamps_scheduler_sort(lamps_scheduler_T *lamps_scheduler) {
    int i = 1;
    while(i++ < NLAMPS * NTIMERS) {
        int j = i;
        registered_lamp_timer_T *t1 = &lamps_scheduler->registered_timers[j];
        registered_lamp_timer_T *t2 = &lamps_scheduler->registered_timers[j-1];
        while (j-- > 0 && timer_compare(*t1, *t2) < 0) {
            registered_lamp_timer_T t3 = *t1;
            t1->hours = t2->hours;
            t1->minutes = t2->minutes;
            t1->mode = t2->mode;
            t1->lamp_pin = t2->lamp_pin;
            t2->hours = t3.hours;
            t2->minutes = t3.minutes;
            t2->mode = t3.mode;
            t2->lamp_pin = t3.lamp_pin;
            t1 = &lamps_scheduler->registered_timers[j];
            t2 = &lamps_scheduler->registered_timers[j-1];
        }
    }
    return lamps_scheduler;
}

void lamps_scheduler_evaluate_registered_timers(lamps_scheduler_T * lamps_scheduler) {
    uint8_t next_index = lamps_scheduler->current_timer_index;
    registered_lamp_timer_T current_time;
    registered_lamp_timer_T t;
    t = lamps_scheduler->registered_timers[next_index];
    current_time.hours = t.hours;
    current_time.minutes = t.minutes;
    current_time.mode = LAMP_OFF;
    if (lamps_scheduler->registered_timers[next_index].mode == LAMP_ON)
        lamps_seton(lamps_scheduler->registered_timers[next_index].lamp_pin);
    else
        lamps_setoff(lamps_scheduler->registered_timers[next_index].lamp_pin);
    lamps_scheduler->current_timer_index = next_index + 1;
    for (int i=next_index+1;i<NLAMPS * NTIMERS * 2;i++) {
        t = lamps_scheduler->registered_timers[i];
        if (t.mode == LAMP_DISABLED) {
            lamps_scheduler->current_timer_index = 0;
            break;
        }
        else if (timer_compare(t, current_time) > 0)
            break;
        else 
            lamps_scheduler->current_timer_index = i + 1;
        if (lamps_scheduler->registered_timers[i].mode == LAMP_ON)
            lamps_seton(lamps_scheduler->registered_timers[i].lamp_pin);
        else
            lamps_setoff(lamps_scheduler->registered_timers[i].lamp_pin);
    }

    // Prepare next timer!
    t = lamps_scheduler->registered_timers[lamps_scheduler->current_timer_index];
    cancel_alarm(lamps_scheduler->alarm_id);
    lamps_scheduler->alarm_id = set_alarm(t, onAlarmHook);
}

void onAlarmHook() {
    lamps_scheduler_evaluate_registered_timers(&lamps_scheduler);
}

void lamps_scheduler_set_clock_time(timer_T new_time) {
    set_clock_time(new_time);
    lamps_scheduler_init(&lamps_scheduler);
}

void lamps_scheduler_init(lamps_scheduler_T *lamps_scheduler) {
    registered_lamp_timer_T current_time;
    get_current_time(&current_time);
    uint8_t initial_lamp_pins = 0; 
    // Set lamps according to current_time
    for (int i=0;i<NLAMPS * NTIMERS * 2;i++) {
        registered_lamp_timer_T t = lamps_scheduler->registered_timers[i];

        // Check if this timer is expired
        if (timer_compare(current_time, t) >= 0) {
            if (t.mode == LAMP_ON) {
                initial_lamp_pins |= 1 << (t.lamp_pin - 2);
            }
            else if (t.mode == LAMP_OFF) {
                initial_lamp_pins &= initial_lamp_pins ^ (1 << (t.lamp_pin - 2));
            }
        } else {
            // first t in the future, next alarm
            lamps_scheduler->alarm_id = set_alarm(t, onAlarmHook);
            lamps_scheduler->current_timer_index = i;
            break;
        }
    }
    // Turn on / off the lamps
    for (int i=0;i<NLAMPS;i++) {
        if ((initial_lamp_pins >> i) & 1) 
            lamps_seton(i+2);
        else
            lamps_setoff(i+2);
    }
}
