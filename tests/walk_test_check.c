#include "walk_test.h"
#include "tcu_sequence.h"
#include "levo_protocol.h"
#include <assert.h>
#include <stdio.h>
static void start(walk_test_t *s) {
    walk_test_arm(s, 1);
    walk_test_tick(s, 2, true, true, 0, 0);
    walk_test_tick(s, 100002, true, true, 0, 0);
    walk_test_tick(s, 150002, true, true, 4, 4);
    assert(s->active);
}
int main(void) {
    walk_test_t s;
    walk_test_arm(&s, 1);
    walk_test_tick(&s, 100000, true, true, 4, 4);
    assert(!s.active);
    start(&s);
    walk_test_tick(&s, 160000, true, true, 0, 4);
    assert(!s.active && !s.armed);
    walk_test_tick(&s, 200000, true, true, 4, 4);
    assert(!s.active);
    start(&s);
    walk_test_tick(&s, 3150002, true, true, 4, 4);
    assert(!s.active && !s.armed);
    start(&s);
    walk_test_tick(&s, 160000, true, false, 4, 4);
    assert(!s.active);
    start(&s);
    walk_test_tick(&s, 160000, false, true, 4, 4);
    assert(!s.active);
    start(&s);
    walk_test_tick(&s, 160000, true, true, 12, 4);
    assert(!s.active);
    start(&s);
    walk_test_tick(&s, 10, true, true, 4, 4);
    assert(!s.active);
    walk_hold_arm(&s, 1);
    walk_test_tick(&s, 2, true, true, 0, 0);
    walk_test_tick(&s, 100002, true, true, 0, 0);
    walk_test_tick(&s, 150002, true, true, 4, 4);
    assert(s.active);
    for (uint64_t n = 150002; n < 120000000; n += 1000)
        walk_test_tick(&s, n, true, true, 4, 4);
    assert(s.active);
    walk_test_tick(&s, 120000001, true, true, 0, 4);
    assert(!s.active && !s.armed);
    walk_hold_arm(&s, 1);
    walk_test_tick(&s, 2, true, true, 0, 0);
    walk_test_tick(&s, 100002, true, true, 0, 0);
    walk_test_tick(&s, 150002, true, true, 4, 4);
    walk_test_tick(&s, 160000, true, false, 4, 4);
    assert(!s.active);
    tcu_seq_t q;
    tcu_diag_frame_t f;
    uint8_t off[7] = {0, 0x5a, 100, 0x5a, 0, 0, 10};
    assert(tcu_seq_start(&q, off, 0));
    assert(!tcu_seq_set_walk_profile(&q));
    unsigned walk_frames = 0, off_frames = 0;
    for (uint64_t now = 0; now < 7000000; now += 1000) {
        if (now == 5100000)
            assert(tcu_seq_set_walk_profile(&q));
        if (now == 6100000)
            assert(tcu_seq_set_profile(&q, off));
        if (tcu_seq_poll(&q, now, &f) && f.id == 0x300) {
            assert(levo_command_valid(f.data, f.len));
            if (now >= 5100000 && now < 6100000) {
                assert(f.data[0] == 3 && f.data[1] == 0xa5 && f.data[4] == 35 && f.data[6] == 35);
                walk_frames++;
            }
            if (now >= 6100000) {
                assert(f.data[0] == 0 && f.data[1] == 0x5a);
                off_frames++;
            }
        }
    }
    assert(walk_frames == 20 && off_frames > 0);
    puts("PASS: hold-before-arm rejected; release, stale input, timeout, conflict, stop and "
         "backwards time cancel; no automatic restart; exact Eco walk frames/checksum and return "
         "OFF.");
}
