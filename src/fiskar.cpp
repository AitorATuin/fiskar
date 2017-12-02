#include <stdarg.h>
#include <TimeLib.h>
#include <Time.h>
#include <TimeAlarms.h>
#include "lamps_scheduler.h"

#define USED_LAMPS 4
lamp_timer_T lamp_timers[USED_LAMPS] = {
    {2, {{0, 1, 10}, {0, 0, 0}, {0, 0, 0}}},
    {3, {{0, 1, 20}, {0, 0, 0}, {0, 0, 0}}},
    {4, {{0, 20, 21}, {0, 0, 0}, {0, 0, 0}}},
    {5, {{2, 20, 30}, {0, 0, 0}, {0, 0, 0}}},
};


// Declare the scheduler taking care of our lamps
lamps_scheduler_T lamps_scheduler;

void SerialPrintF(const char *fmt, ... ){
    char buf[128]; // resulting string limited to 128 chars
    va_list args;
    va_start (args, fmt );
    vsnprintf(buf, 128, fmt, args);
    va_end (args);
    Serial.print(buf);
}

void get_current_time(registered_lamp_timer_T *current_time) {
    current_time->hours = 1;
    current_time->minutes = 12;
    current_time->mode = LAMP_OFF;
}

void lamps_seton(uint8_t lamp_pin) {
    SerialPrintF("Setting on lamp %d\n", lamp_pin);
    digitalWrite(lamp_pin, HIGH);
}

void lamps_setoff(uint8_t lamp_pin) {
    SerialPrintF("Setting off lamp %d\n", lamp_pin);
    digitalWrite(lamp_pin, LOW);
}

uint8_t set_alarm(registered_lamp_timer_T timer, uint8_t old_alarm_id, alarm_hook_t alarm_hook) {
    SerialPrintF("Setting alarm: %d:%d\n", timer.hours, timer.minutes);
    uint8_t alarm_id = Alarm.alarmOnce(timer.hours, timer.minutes, 0, alarm_hook);
    return alarm_id;
}

// void cancelTimer(timerT *timer) {
//     Alarm.free(timer->alarmId0);
//     Alarm.free(timer->alarmIdF);
//     timer->alarmId0 = dtINVALID_ALARM_ID;
//     timer->alarmIdF = dtINVALID_ALARM_ID;
// }

void setup() {
    for (int i=2;i<=8;i++)
        pinMode(i, OUTPUT);
    Serial.begin(9600);
    while (!Serial) ;
    Serial.println("Setting lamps ...");
    lamps_scheduler_create(lamp_timers, USED_LAMPS);
    lamps_scheduler_init(&lamps_scheduler);
    Serial.println("Ready ...");
}

void loop() {
    Alarm.delay(1000);
}


