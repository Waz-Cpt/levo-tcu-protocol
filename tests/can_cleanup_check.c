#include "can_cleanup.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    can_cleanup_state_t state;
    bool fail_status, fail_stop, fail_uninstall, fail_isolate, bus_off_during_stop;
    bool isolated, released;
    char calls[8];
    unsigned count;
} fake_t;

static void call(fake_t *f, char name) {
    assert(!f->released); /* No driver calls after uninstall frees the handle. */
    assert(f->count + 1 < sizeof(f->calls));
    f->calls[f->count++] = name;
}
static bool isolate(void *context) {
    fake_t *f = context;
    call(f, 'I');
    f->isolated = !f->fail_isolate;
    return f->isolated;
}
static bool status(void *context, can_cleanup_state_t *state) {
    fake_t *f = context;
    call(f, 'Q');
    assert(f->calls[0] == 'I');
    *state = f->state;
    return !f->fail_status;
}
static bool stop(void *context) {
    fake_t *f = context;
    call(f, 'S');
    if (f->bus_off_during_stop)
        f->state = CAN_CLEANUP_BUS_OFF;
    if (f->fail_stop || f->state != CAN_CLEANUP_RUNNING)
        return false; /* Matches IDF's rejection of BUS_OFF and STOPPED. */
    f->state = CAN_CLEANUP_STOPPED;
    return true;
}
static bool uninstall(void *context) {
    fake_t *f = context;
    call(f, 'U');
    if (f->fail_uninstall || (f->state != CAN_CLEANUP_STOPPED && f->state != CAN_CLEANUP_BUS_OFF))
        return false;
    f->released = true;
    return true;
}
int main(void) {
    const can_cleanup_ops_t ops = {isolate, status, stop, uninstall};
    fake_t f = {0};
    can_cleanup_result_t r = can_cleanup(false, &ops, &f);
    assert(r.isolated && r.driver_released && !r.stop_attempted);
    assert(!strcmp(f.calls, "I")); /* Install failed: never use a null handle. */

    for (unsigned state = CAN_CLEANUP_RUNNING; state <= CAN_CLEANUP_BUS_OFF; state++) {
        f = (fake_t){.state = state};
        r = can_cleanup(true, &ops, &f);
        assert(r.isolated && r.driver_released && f.released);
        assert(!strcmp(f.calls, state == CAN_CLEANUP_RUNNING ? "IQSU" : "IQU"));
        assert(r.stop_attempted == (state == CAN_CLEANUP_RUNNING));
    } /* STOPPED includes successful install followed by failed start. */

    f = (fake_t){.state = CAN_CLEANUP_RUNNING, .bus_off_during_stop = true};
    r = can_cleanup(true, &ops, &f);
    assert(r.isolated && r.driver_released && r.stop_attempted && !r.stop_succeeded);
    assert(!strcmp(f.calls, "IQSU")); /* Bus-off race must not abort before uninstall. */

    f = (fake_t){.state = CAN_CLEANUP_RUNNING, .fail_status = true};
    r = can_cleanup(true, &ops, &f);
    assert(r.isolated && r.driver_released && r.stop_succeeded);

    f = (fake_t){.state = CAN_CLEANUP_RUNNING, .fail_stop = true};
    r = can_cleanup(true, &ops, &f);
    assert(r.isolated && !r.driver_released && !f.released);
    assert(!strcmp(f.calls, "IQSU")); /* Bounded attempt, isolated instead of reboot. */

    f = (fake_t){.state = CAN_CLEANUP_RUNNING, .fail_uninstall = true};
    r = can_cleanup(true, &ops, &f);
    assert(r.isolated && !r.driver_released && r.stop_succeeded);
    assert(f.state == CAN_CLEANUP_STOPPED && !f.released);

    f = (fake_t){.state = CAN_CLEANUP_UNKNOWN};
    r = can_cleanup(true, &ops, &f);
    assert(r.isolated && !r.driver_released && !r.stop_succeeded);
    assert(!strcmp(f.calls, "IQSU")); /* Unexpected/recovering state stays isolated. */

    f = (fake_t){.state = CAN_CLEANUP_BUS_OFF, .fail_isolate = true};
    r = can_cleanup(true, &ops, &f);
    assert(!r.isolated && r.driver_released); /* Do not hide GPIO failures. */
    puts("PASS bus-off cleanup, running-to-bus-off race, partial startup, unknown state, "
         "failed driver calls and TX isolation before cleanup");
}
