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

// Interface exported
extern void lamps_scheduler_create(lamp_timer_T *lamps, uint8_t n_timers);
extern void lamps_scheduler_init(lamps_scheduler_T *lamps_scheduler); 

// Functions needed
void lamps_seton(uint8_t lamp_pin);
void lamps_setoff(uint8_t lamp_pin);
uint8_t set_alarm(registered_lamp_timer_T timer, uint8_t old_alarm_id, alarm_hook_t alarm_hook);

#endif
