#include <stdio.h>
#include <stdbool.h>
#include "minunit.h"
#include "lamps_scheduler.h"

int tests_run = 0;

lamps_scheduler_T lamps_scheduler;

uint8_t lamps_on = 0;

timer_T current_time = {0, 0, 0};

uint8_t new_alarm_id = 0;

alarm_hook_t alarm_hook = NULL;

void lamps_seton(uint8_t lamp_pin) {
    lamps_on |= 1 << (lamp_pin - 2);
}

void lamps_setoff(uint8_t lamp_pin) {
    lamps_on &= lamps_on ^ (1 << (lamp_pin - 2));
}

uint8_t set_alarm(registered_lamp_timer_T timer, uint8_t old_alarm_id, alarm_hook_t a_hook) {
    alarm_hook = a_hook;
    return ++new_alarm_id;
}

void get_current_time(registered_lamp_timer_T *c_time) {
    c_time->hours = current_time.hours;
    c_time->minutes = current_time.minutes;
    c_time->mode = LAMP_ON;
}

bool _test_lamp_is_on(uint8_t *lamp_pins) {
    for (int i=0; i<NLAMPS; i++) {
        if (!((lamps_on & (1 << i) / i) == lamp_pins[i]))
            return false;
    }
    return true;
}

bool _test_disabled_from(lamps_scheduler_T *lamps_scheduler, int n) {
    for (int i=n;i < NLAMPS * NTIMERS * 2; i++) {
        if (lamps_scheduler->registered_timers[i].mode != LAMP_DISABLED)
            return false;
    }
    return true;
}

bool _test_on_or_off(lamps_scheduler_T *lamps_scheduler, uint8_t *modes, int n) {
    for (int i=0;i < n;i++) {
        if (lamps_scheduler->registered_timers[i].mode != modes[i])
            return false;
    }
    return true;
}

bool _test_lamps_scheduler_sorted(lamp_timer_T *lamp_timers, uint8_t *lamp_modes, int n_lamps, int n_timers) {
    lamps_scheduler_create(lamp_timers, n_lamps);
    return _test_disabled_from(&lamps_scheduler, n_timers) && \
       _test_on_or_off(&lamps_scheduler, lamp_modes, n_timers);
}

bool _test_lamps_scheduler_init(lamp_timer_T *lamp_timers, uint8_t n_lamps, uint8_t result) {
    lamps_scheduler_create(lamp_timers, n_lamps);
    lamps_scheduler_init(&lamps_scheduler);
    return lamps_on == result;
}

bool _test_lamps_scheduler_sorted_len0() {
    uint8_t lamp_modes[0] = {};
    lamp_timer_T lamp_timers[0] = {};
    return _test_lamps_scheduler_sorted(lamp_timers, lamp_modes, 0, 0);
    /* return _test_disabled_from(&lamps_scheduler, 0); */
}

bool _test_lamps_scheduler_sorted_len1_1() {
    uint8_t lamp_modes[6] = {LAMP_ON, LAMP_ON, LAMP_ON, LAMP_OFF, LAMP_OFF, LAMP_OFF};
    lamp_timer_T lamp_timers[1] = {
        {2, {{0, 1, 60}, {0, 1, 70}, {0, 1, 80}}}
    };
    return _test_lamps_scheduler_sorted(lamp_timers, lamp_modes, 1, 6);
}

bool _test_lamps_scheduler_sorted_len1_2() {
    uint8_t lamp_modes[6] = {LAMP_ON, LAMP_OFF, LAMP_ON, LAMP_OFF, LAMP_ON, LAMP_OFF};
    lamp_timer_T lamp_timers[1] = {
        {2, {{0, 1, 9}, {0, 12, 20}, {0, 35, 5}}}
    };
    return _test_lamps_scheduler_sorted(lamp_timers, lamp_modes, 1, 6);
}

bool _test_lamps_scheduler_init_1() {
    lamps_on = 0;
    current_time = (timer_T){0, 0, 0};
    lamp_timer_T lamp_timers[0] = {};
    return _test_lamps_scheduler_init(lamp_timers, 0, 0);
}

bool _test_lamps_scheduler_init_2() {
    lamps_on = 0;
    current_time = (timer_T){0, 59, 0};
    lamp_timer_T lamp_timers[1] = {
        {2, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}}
    };
    return _test_lamps_scheduler_init(lamp_timers, 1, 1);
}

bool _test_lamps_scheduler_init_3() {
    lamps_on = 0;
    current_time = (timer_T){1, 0, 0};
    lamp_timer_T lamp_timers[1] = {
        {2, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}}
    };
    return _test_lamps_scheduler_init(lamp_timers, 1, 0);
}

bool _test_lamps_scheduler_init_4() {
    lamps_on = 0;
    current_time = (timer_T){5, 45, 0};
    lamp_timer_T lamp_timers[1] = {
        {2, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}}
    };
    return _test_lamps_scheduler_init(lamp_timers, 1, 1);
}

bool _test_lamps_scheduler_init_5() {
    lamps_on = 0;
    current_time = (timer_T){5, 45, 0};
    lamp_timer_T lamp_timers[5] = {
        {2, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}},
        {3, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}},
        {4, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}},
        {5, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}},
        {6, {{0, 0, 60}, {3, 0, 60}, {5, 0, 60}}},
    };
    return _test_lamps_scheduler_init(lamp_timers, 5, 31);
}

bool _test_lamps_scheduler_evaluate_1() {
    lamps_on = 0;
    new_alarm_id = 0;
    current_time = (timer_T){2, 0, 0};
    lamp_timer_T lamp_timers[4] = {
        {2, {{0, 0, 60}, {0, 0, 0}, {0, 0, 0}}},
        {3, {{2, 0, 60}, {0, 0, 0}, {0, 0, 0}}},
        {4, {{2, 0, 120}, {0, 0, 0}, {0, 0, 0}}},
        {5, {{2, 0, 180}, {0, 0, 0}, {0, 0, 0}}},
    };
    bool res1 = _test_lamps_scheduler_init(lamp_timers, 4, 14);
    bool res2 = lamps_scheduler.alarm_id == 1;
    bool res3 = lamps_scheduler.current_timer_index == 5;
    uint8_t lamps1 = lamps_on; // 14

    // First timer triggered
    if (alarm_hook)
        alarm_hook();
    else
        return false;
    bool res4 = lamps_scheduler.alarm_id == 2;
    bool res5 = lamps_scheduler.current_timer_index == 6;
    uint8_t lamps2 = lamps_on; // 12

    // Second timer triggered
    if (alarm_hook)
        alarm_hook();
    else
        return false;
    bool res6 = lamps_scheduler.alarm_id == 3;
    bool res7 = lamps_scheduler.current_timer_index == 7;
    uint8_t lamps3 = lamps_on; // 8

    // Third timer triggered, next index should be 0
    if (alarm_hook)
        alarm_hook();
    else
        return false;
    bool res8 = lamps_scheduler.alarm_id == 4;
    bool res9 = lamps_scheduler.current_timer_index == 0;
    uint8_t lamps4 = lamps_on; // 0

    // Fourth timer triggered, first alarm in time
    if (alarm_hook)
        alarm_hook();
    else
        return false;
    bool res10 = lamps_scheduler.alarm_id == 5;
    bool res11 = lamps_scheduler.current_timer_index == 1;
    uint8_t lamps5 = lamps_on; // 1

    // Fith timer triggered, end of first alarm
    if (alarm_hook)
        alarm_hook();
    else
        return false;
    bool res12 = lamps_scheduler.alarm_id == 6;
    bool res13 = lamps_scheduler.current_timer_index == 2;
    uint8_t lamps6 = lamps_on; // 0

    // Sixth timer triggered, we have 3 alarm in this time so index
    // should be +3
    // From here, the whols cycle repeats
    if (alarm_hook)
        alarm_hook();
    else
        return false;
    bool res14 = lamps_scheduler.alarm_id == 7;
    bool res15 = lamps_scheduler.current_timer_index == 5;
    uint8_t lamps7 = lamps_on; // 14

    return res1 && res2 && res3 && res4 && res5 && res6 && res7 && res8 && \
        res9 && res10 && res11 && res12 && res13 && res14 && res15 && \
        lamps1 == 14 && lamps2 == 12 && lamps3 == 8 && lamps4 == 0 && \
        lamps5 == 1 && lamps6 == 0 && lamps7 == 14;
}

static char * test_lamps_scheduler_sorted_len0() {
    mu_assert("error, test_lamps_scheduler_sorted_len0", _test_lamps_scheduler_sorted_len0());
    return 0;
}

static char * test_lamps_scheduler_sorted_len1_1() {
    mu_assert("error, test_lamps_scheduler_sorted_len1_1", _test_lamps_scheduler_sorted_len1_1());
    return 0;
}

static char * test_lamps_scheduler_sorted_len1_2() {
    mu_assert("error, test_lamps_scheduler_sorted_len1_2", _test_lamps_scheduler_sorted_len1_2());
    return 0;
}

static char * test_lamps_scheduler_init_1() {
    mu_assert("error, test_lamps_scheduler_init_1", _test_lamps_scheduler_init_1());
    return 0;
}

static char * test_lamps_scheduler_init_2() {
    mu_assert("error, test_lamps_scheduler_init_2", _test_lamps_scheduler_init_2());
    return 0;
}

static char * test_lamps_scheduler_init_3() {
    mu_assert("error, test_lamps_scheduler_init_3", _test_lamps_scheduler_init_3());
    return 0;
}

static char * test_lamps_scheduler_init_4() {
    mu_assert("error, test_lamps_scheduler_init_4", _test_lamps_scheduler_init_4());
    return 0;
}

static char * test_lamps_scheduler_init_5() {
    mu_assert("error, test_lamps_scheduler_init_5", _test_lamps_scheduler_init_5());
    return 0;
}

static char * test_lamps_scheduler_evaluate_1() {
    mu_assert("error, test_lamps_scheduler_evaluate_1", _test_lamps_scheduler_evaluate_1());
    return 0;
}

static char * all_tests() {
    mu_run_test(test_lamps_scheduler_sorted_len0);
    mu_run_test(test_lamps_scheduler_sorted_len1_1);
    mu_run_test(test_lamps_scheduler_sorted_len1_2);
    mu_run_test(test_lamps_scheduler_init_1);
    mu_run_test(test_lamps_scheduler_init_2);
    mu_run_test(test_lamps_scheduler_init_3);
    mu_run_test(test_lamps_scheduler_init_4);
    mu_run_test(test_lamps_scheduler_init_5);
    mu_run_test(test_lamps_scheduler_evaluate_1);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}

