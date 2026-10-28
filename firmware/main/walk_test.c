#include "walk_test.h"
#include <string.h>
void walk_test_arm(walk_test_t *s, uint64_t now) {
    memset(s, 0, sizeof(*s));
    s->armed = true;
    s->last = now;
    s->max_hold_us = 3000000;
}
void walk_hold_arm(walk_test_t *s, uint64_t now) {
    walk_test_arm(s, now);
    s->max_hold_us = 0;
}
void walk_test_tick(walk_test_t *s, uint64_t now, bool allowed, bool fresh, unsigned raw,
                    unsigned stable) {
    if (!allowed || !fresh || now < s->last) {
        s->active = s->armed = false;
        s->last = now;
        return;
    }
    s->last = now;
    if (!s->armed)
        return;
    if (s->active) {
        if (raw != 4 || stable != 4 || (s->max_hold_us && now - s->start >= s->max_hold_us)) {
            s->active = s->armed = false;
        }
        return;
    }
    if (raw == 0 && stable == 0) {
        if (!s->release_at)
            s->release_at = now;
        if (now - s->release_at >= 100000)
            s->released = true;
    } else {
        s->release_at = 0;
        if (raw == 4 && stable == 4 && s->released) {
            s->active = true;
            s->start = now;
        } else if (raw != 4)
            s->released = false;
    }
}
