#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool armed, active, released;
    uint64_t last, release_at, start, max_hold_us;
} walk_test_t;
/* First stand test only: explicit arm, fresh single-button hold, 3 s maximum. */
void walk_test_arm(walk_test_t *s, uint64_t now);
/* Normal riding: hold to run, release/fault to stop; no test-duration cutoff. */
void walk_hold_arm(walk_test_t *s, uint64_t now);
void walk_test_tick(walk_test_t *s, uint64_t now, bool allowed, bool fresh, unsigned raw,
                    unsigned stable);
