#ifndef LAMPS_SCHEDULER_H_
#define LAMPS_SCHEDULER_H_

#include <stdint.h>

#define LAMP_OFF 0
#define LAMP_ON 1
#define LAMP_DISABLED 3

#define NLAMPS 8
#define NTIMERS 3

typedef void (*alarm_hook_t)();

typedef struct timer_T {
    uint8_t hours;
    uint8_t minutes;
    uint16_t duration;
} timer_T;

typedef struct lamp_timer_T {
    uint8_t lamp_pin;
    timer_T timers[NTIMERS];
} lamp_timer_T;

typedef struct registered_lamp_timer_T {
    uint8_t lamp_pin;
    uint8_t hours;
    uint8_t minutes;
    uint8_t mode;
} registered_lamp_timer_T;

typedef struct lamps_scheduler_T {
    registered_lamp_timer_T registered_timers[NLAMPS * NTIMERS * 2];
    uint8_t current_timer_index;
    uint8_t alarm_id;
} lamps_scheduler_T;

#ifdef __cplusplus
extern "C" {
#endif
// Interface exported
void lamps_scheduler_create(lamp_timer_T *lamps, uint8_t n_timers);
void lamps_scheduler_init(lamps_scheduler_T *lamps_scheduler); 
void lamps_scheduler_set_clock_time(timer_T new_time);

// Functions needed
void get_current_time(registered_lamp_timer_T *current_time);
void set_clock_time(timer_T new_time);
void lamps_seton(uint8_t lamp_pin);
void lamps_setoff(uint8_t lamp_pin);
uint8_t set_alarm(registered_lamp_timer_T timer, alarm_hook_t alarm_hook);
void cancel_alarm(uint8_t alarm_id);
#ifdef __cplusplus
}
#endif

#endif
