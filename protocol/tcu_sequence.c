#include "tcu_sequence.h"
#include "levo_protocol.h"
#include <string.h>
#define PERIOD_US UINT64_C(50000)
#define COMPANION_US UINT64_C(1000)
#define MAX_LATENESS_US UINT64_C(10000)
static bool valid_profile(const uint8_t p[7]) {
    return p && (p[0] == 0 || p[0] == 3) && p[1] == 0x5a && p[2] == 0x64 && p[3] == 0x5a &&
           p[4] <= 100 && p[5] == 0 && p[6] <= 100;
}
bool tcu_seq_set_profile(tcu_seq_t *s, const uint8_t p[7]) {
    if (!s || s->phase != TCU_SEQ_RUN || !valid_profile(p))
        return false;
    memcpy(s->payload, p, 7);
    return true;
}
bool tcu_seq_start(tcu_seq_t *s, const uint8_t p[7], uint64_t now) {
    if (!s)
        return false;
    memset(s, 0, sizeof(*s));
    s->phase = TCU_SEQ_FAULT;
    if (!p || (p[0] != 0 && p[0] != 3) || p[1] != 0x5a || p[3] != 0x5a || p[4] > 100 || p[5] != 0 ||
        p[6] > 100)
        return false;
    memcpy(s->payload, p, 7);
    s->payload[2] = 0x64;
    s->counter = 1;
    s->last_us = s->due_us = now;
    s->phase = TCU_SEQ_BOOT;
    return true;
}
bool tcu_seq_shutdown(tcu_seq_t *s) {
    if (!s || (s->phase != TCU_SEQ_BOOT && s->phase != TCU_SEQ_RUN))
        return false;
    s->phase = TCU_SEQ_SHUTDOWN;
    s->shutdown_cycles = 0;
    return true;
}
bool tcu_seq_poll(tcu_seq_t *s, uint64_t now, tcu_diag_frame_t *out) {
    if (!s || !out || s->phase == TCU_SEQ_IDLE || s->phase == TCU_SEQ_DONE ||
        s->phase == TCU_SEQ_FAULT)
        return false;
    if (now < s->last_us || now > UINT64_MAX - PERIOD_US) {
        s->phase = TCU_SEQ_FAULT;
        return false;
    }
    s->last_us = now;
    if (now < s->due_us)
        return false;
    if (now - s->due_us > MAX_LATENESS_US) {
        s->phase = TCU_SEQ_FAULT;
        return false;
    }
    memset(out, 0, sizeof(*out));
    if (s->companion_pending) {
        out->id = 0x301;
        out->len = 3;
        levo_companion_encode(s->counter, out->data);
        s->counter = (s->counter + 1) & 15;
        s->companion_pending = false;
        s->due_us += PERIOD_US - COMPANION_US;
        return true;
    }
    uint8_t p[7];
    memcpy(p, s->payload, 7);
    if (s->phase == TCU_SEQ_BOOT) {
        if (s->boot_cycles < 99)
            p[2] = 0;
        else
            s->phase = TCU_SEQ_RUN;
        s->boot_cycles++;
    } else if (s->phase == TCU_SEQ_SHUTDOWN) {
        if (s->shutdown_cycles >= 41) {
            s->phase = TCU_SEQ_DONE;
            return false;
        }
        if (s->shutdown_cycles < 31) {
            const uint8_t first[7] = {0, 0x5a, 0, 0x5a, 0, 0, 10};
            memcpy(p, first, 7);
        } else {
            p[2] = 0;
            p[3] = 0xa5;
        }
        s->shutdown_cycles++;
    }
    out->id = 0x300;
    out->len = 8;
    levo_command_encode(p, s->counter, out->data);
    s->companion_pending = true;
    s->due_us += COMPANION_US;
    return true;
}

bool tcu_seq_set_walk_profile(tcu_seq_t *s) {
    if (!s || s->phase != TCU_SEQ_RUN)
        return false;
    const uint8_t p[7] = {3, 0xa5, 100, 0x5a, 35, 0, 35};
    memcpy(s->payload, p, 7);
    return true;
}
