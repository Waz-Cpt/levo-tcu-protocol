#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "tcu_diagnostics.h"
/* Capture-derived command sequencer. No hardware I/O. Used by the tested
 * replacement on one Levo; timings are implementation choices, not receiver limits.
 * Caller handles electrical wake, readiness, transmit results and assist policy.
 */
typedef enum {
    TCU_SEQ_IDLE,
    TCU_SEQ_BOOT,
    TCU_SEQ_RUN,
    TCU_SEQ_SHUTDOWN,
    TCU_SEQ_DONE,
    TCU_SEQ_FAULT
} tcu_seq_phase_t;
typedef struct {
    tcu_seq_phase_t phase;
    uint64_t last_us, due_us;
    uint32_t boot_cycles, shutdown_cycles;
    uint8_t payload[7], counter;
    bool companion_pending;
} tcu_seq_t;
/* First command is due immediately, starting counter 1. */
bool tcu_seq_start(tcu_seq_t *s, const uint8_t running_payload[7], uint64_t now_us);
/* Change the running profile without restarting counters or startup timing.
 * Accepted only in RUN; applies to the next 300 frame, never its pending 301. */
bool tcu_seq_set_profile(tcu_seq_t *s, const uint8_t running_payload[7]);
/* Takes effect at the next command cycle; an already pending companion is emitted first. */
bool tcu_seq_shutdown(tcu_seq_t *s);
/* Tick at <=1ms intervals. More than10ms lateness faults instead of bursting frames. */
bool tcu_seq_poll(tcu_seq_t *s, uint64_t now_us, tcu_diag_frame_t *out);

/* Observed Eco walk payload, RUN phase only. Caller must enforce hold/release. */
bool tcu_seq_set_walk_profile(tcu_seq_t *s);
