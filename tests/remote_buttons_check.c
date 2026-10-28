#include "remote_buttons.h"
#include <assert.h>
#include <stdio.h>
static uint64_t now;
static unsigned mode;
static remote_buttons_t s;
static bool sample(unsigned mask, bool ready) {
    unsigned next = mode;
    bool changed = remote_buttons_poll(&s, now, mask, ready, mode, &next);
    now += 31000;
    if (changed)
        mode = next;
    return changed;
}
static void release(void) {
    sample(0, true);
    sample(0, true);
}
static void press(unsigned mask) {
    sample(mask, true);
    sample(mask, true);
}
int main(void) {
    remote_buttons_reset(&s, 0);
    press(1);
    assert(mode == 0); /* held at startup */
    release();
    press(1);
    assert(mode == 1);
    sample(1, true);
    assert(mode == 1); /* no repeated steps */
    release();
    press(8);
    assert(mode == 2);
    release();
    press(8);
    assert(mode == 3);
    release();
    press(8);
    assert(mode == 1);
    release();
    press(2);
    assert(mode == 0);
    release();
    press(2);
    assert(mode == 0);
    release();
    press(8);
    assert(mode == 1);
    release();
    press(3);
    assert(mode == 1); /* conflicting buttons */
    press(1);
    assert(mode == 1); /* require full release */
    release();
    press(4);
    assert(mode == 1); /* walk has no effect in first test */
    release();
    sample(8, false);
    sample(8, false);
    sample(8, true);
    assert(mode == 1);
    release();
    press(8);
    assert(mode == 2);
    release();
    sample(1, true);
    now -= 30000;
    sample(0, true);
    assert(mode == 2); /* bounce */

    /* A legitimate minus press reduces assist once, then requests shutdown at 2s. */
    now = 0;
    mode = 3;
    remote_buttons_reset(&s, now);
    release();
    press(2);
    assert(mode == 2);
    now = s.since + 1999999;
    sample(2, true);
    assert(!s.shutdown);
    now = s.since + 2000000;
    sample(2, true);
    assert(s.shutdown);
    /* Boot-held and stale/disabled inputs cannot cause a later hold action. */
    now = 0;
    remote_buttons_reset(&s, now);
    press(2);
    now += 3000000;
    sample(2, true);
    assert(!s.shutdown);
    release();
    press(2);
    sample(2, false);
    now += 3000000;
    sample(2, true);
    assert(!s.shutdown);
    release();
    press(2);
    sample(3, true);
    now += 3000000;
    sample(3, true);
    assert(!s.shutdown);
    release();
    press(2);
    now = s.since - 1;
    sample(2, true);
    assert(!s.shutdown);
    puts("PASS: startup/disabled hold inhibition, debounce, cycle wrap, +/- limits, no autorepeat, "
         "conflicting presses, walk reserved; 2s minus shutdown and cancelled holds.");
}
