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

void cancel_alarm(uint8_t alarm_id) {
    Alarm.free(alarm_id);
}

uint8_t set_alarm(registered_lamp_timer_T timer, alarm_hook_t alarm_hook) {
    SerialPrintF("Setting alarm: %d:%d\n", timer.hours, timer.minutes);
    uint8_t alarm_id = Alarm.alarmOnce(timer.hours, timer.minutes, 0, alarm_hook);
    return alarm_id;
}

void setup() {
    for (int i=2;i<=8;i++)
        pinMode(i, OUTPUT);
    Serial.begin(9600);
    while (!Serial) ;
    Serial.println("Setting lamps ...");
    lamps_scheduler_create(lamp_timers, USED_LAMPS);
    lamps_scheduler_init(&lamps_scheduler);
    Serial.println("Ready ...");
    Alarm.delay(1000);
}

void prompt_show_timer(registered_lamp_timer_T t) {
    SerialPrintF("\n%d:%d[%d-%d]\n", t.hours, t.minutes, t.lamp_pin, t.mode);
}

void prompt_show_timer_by_index(uint8_t n) {
    prompt_show_timer(lamps_scheduler.registered_timers[n]);
}

void prompt_show_next_timer() {
    prompt_show_timer_by_index(lamps_scheduler.current_timer_index);
}

void prompt_show_error(const char *err_str) {
    SerialPrintF("%s\n", err_str);
}

void prompt_show_lamp(uint8_t lamp) {
    registered_lamp_timer_T t;
    for (uint8_t i=0;i<NLAMPS*NTIMERS*2;i++) {
        t = lamps_scheduler.registered_timers[i];
        if (t.lamp_pin == lamp && t.mode != LAMP_DISABLED)
            prompt_show_timer_by_index(i);
    }
}

void prompt_show_current_time() {
    registered_lamp_timer_T t;
    get_current_time(&t);
    prompt_show_timer(t);
}

void prompt_process_read_command(char subcmd) {
    char lamp[1];
    uint8_t lamp_n;
    switch(subcmd) {
        case 'l':
            Serial.readBytes(lamp, 1);
            if (lamp[0] >= '0' && lamp[0] <= '9') {
                lamp_n = lamp[0] - '0';
                prompt_show_lamp(lamp_n);
            }
            else
                prompt_show_error("E");
            break;
        case 'n':
            prompt_show_next_timer();
            break;
        case 'c':
            prompt_show_current_time();
            break;
        default:
            prompt_show_error("E");
    }
}

void prompt_process_set_command(char subcmd) {
}

void prompt_process_command() {
    char cmd[2];
    Serial.readBytes(cmd, 2);
    switch(cmd[0]) {
        case 'r':
            prompt_process_read_command(cmd[1]);
            break;
        case 's':
            prompt_process_set_command(cmd[1]);
            break;
        default:
            prompt_show_error("E");
    }
}

void loop() {
    if (Serial.available() > 0) {
        prompt_process_command();
    }
}


