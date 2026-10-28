#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    uint64_t last_us, stop_us;
    unsigned mode;
    bool stopping;
} tcu_bench_control_t;
void tcu_bench_init(tcu_bench_control_t *s, uint64_t now);
void tcu_bench_tick(tcu_bench_control_t *s, uint64_t now, uint64_t motor_at);
void tcu_bench_stop(tcu_bench_control_t *s, uint64_t now);
bool tcu_bench_select(tcu_bench_control_t *s, uint64_t now, uint64_t motor_at, bool ready,
                      unsigned mode);
void tcu_bench_payload(const tcu_bench_control_t *s, uint8_t out[7]);
bool tcu_bench_shutdown_due(const tcu_bench_control_t *s, uint64_t now);
