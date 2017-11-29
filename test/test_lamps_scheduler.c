#include <stdio.h>
#include <stdbool.h>
#include "minunit.h"
#include "lamps_scheduler.h"

int tests_run = 0;

lamps_scheduler_T lamps_scheduler;

uint8_t lamps_on = 0;

extern void lamps_seton(uint8_t lamp_pin) {
    lamps_on |= 1 << (lamp_pin - 2);
}

extern void lamps_setoff(uint8_t lamp_pin) {

}

extern uint8_t set_alarm(registered_lamp_timer_T timer, uint8_t old_alarm_id, alarm_hook_t alarm_hook) {
    return 1;
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

bool _test_lamps_scheduler_sorted_len0() {
    lamp_timer_T lamp_timers[0] = {};
    lamps_scheduler_create(lamp_timers, 0);
    return _test_disabled_from(&lamps_scheduler, 0);
}

bool _test_lamps_scheduler_sorted_len1_1() {
    uint8_t lamp_modes[6] = {LAMP_ON, LAMP_ON, LAMP_ON, LAMP_OFF, LAMP_OFF, LAMP_OFF};
    lamp_timer_T lamp_timers[1] = {
        {2, {{0, 1, 60}, {0, 1, 70}, {0, 1, 80}}}
    };
    lamps_scheduler_create(lamp_timers, 1);
    return _test_disabled_from(&lamps_scheduler, 6) && \
        _test_on_or_off(&lamps_scheduler, lamp_modes, 6); 
}

bool _test_lamps_scheduler_sorted_len1_2() {
    uint8_t lamp_modes[6] = {LAMP_ON, LAMP_OFF, LAMP_ON, LAMP_OFF, LAMP_ON, LAMP_OFF};
    lamp_timer_T lamp_timers[1] = {
        {2, {{0, 1, 9}, {0, 12, 20}, {0, 35, 5}}}
    };
    lamps_scheduler_create(lamp_timers, 1);
    return _test_disabled_from(&lamps_scheduler, 6) && \
        _test_on_or_off(&lamps_scheduler, lamp_modes, 6);
}

bool _test_lamps_scheduler_init_1() {
    lamp_timer_T lamp_timers[0] = {};
    lamps_scheduler_create(lamp_timers, 0);
    lamps_scheduler_init(&lamps_scheduler);
    return lamps_on == 0;
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

static char * all_tests() {
    mu_run_test(test_lamps_scheduler_sorted_len0);
    mu_run_test(test_lamps_scheduler_sorted_len1_1);
    mu_run_test(test_lamps_scheduler_sorted_len1_2);
    mu_run_test(test_lamps_scheduler_init_1);
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

