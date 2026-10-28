#include "tcu_bench_control.h"
#include <string.h>
void tcu_bench_init(tcu_bench_control_t *s, uint64_t now) {
    memset(s, 0, sizeof(*s));
    s->last_us = now;
}
void tcu_bench_stop(tcu_bench_control_t *s, uint64_t now) {
    if (!s->stopping) {
        s->stop_us = now;
        s->stopping = true;
    }
    s->mode = 0;
}
void tcu_bench_tick(tcu_bench_control_t *s, uint64_t now, uint64_t motor_at) {
    if (now < s->last_us || motor_at > now) {
        tcu_bench_stop(s, now);
        return;
    }
    s->last_us = now;
    if (now - motor_at >= 500000)
        tcu_bench_stop(s, now);
}
bool tcu_bench_select(tcu_bench_control_t *s, uint64_t now, uint64_t motor_at, bool ready,
                      unsigned mode) {
    tcu_bench_tick(s, now, motor_at);
    if (s->stopping || mode > 3 || (!ready && mode != 0))
        return false;
    s->mode = mode;
    return true;
}
void tcu_bench_payload(const tcu_bench_control_t *s, uint8_t out[7]) {
    static const uint8_t profiles[4][7] = {{0, 0x5a, 100, 0x5a, 0, 0, 10},
                                           {3, 0x5a, 100, 0x5a, 35, 0, 35},
                                           {3, 0x5a, 100, 0x5a, 50, 0, 70},
                                           {3, 0x5a, 100, 0x5a, 100, 0, 100}};
    memcpy(out, profiles[s->stopping || s->mode > 3 ? 0 : s->mode], 7);
}
bool tcu_bench_shutdown_due(const tcu_bench_control_t *s, uint64_t now) {
    return s->stopping && now >= s->stop_us && now - s->stop_us >= 5000000;
}
