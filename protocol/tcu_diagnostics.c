#include "tcu_diagnostics.h"
#include <string.h>
static const uint16_t dids[TCU_DIAG_COUNT] = {0x0203, 0x0202, 0x0204, 0x0207,
                                              0x0206, 0x020d, 0x0213, 0x0217};
void tcu_diag_abort(tcu_diag_t *s) {
    if (s)
        s->phase = TCU_DIAG_FAILED;
}
static bool tick(tcu_diag_t *s, uint64_t now) {
    if (!s || s->phase == TCU_DIAG_IDLE || s->phase == TCU_DIAG_DONE || s->phase == TCU_DIAG_FAILED)
        return false;
    if (now < s->last_ms || now > UINT64_MAX - TCU_DIAG_TIMEOUT_MS) {
        tcu_diag_abort(s);
        return false;
    }
    s->last_ms = now;
    if ((s->phase == TCU_DIAG_WAIT || s->phase == TCU_DIAG_FLOW) && now >= s->deadline_ms) {
        tcu_diag_abort(s);
        return false;
    }
    return true;
}
void tcu_diag_start(tcu_diag_t *s, uint64_t now) {
    if (!s)
        return;
    memset(s, 0, sizeof(*s));
    s->last_ms = now;
    s->phase = TCU_DIAG_REQUEST;
}
bool tcu_diag_poll(tcu_diag_t *s, uint64_t now, tcu_diag_frame_t *out) {
    if (!tick(s, now) || !out)
        return false;
    if (s->phase != TCU_DIAG_REQUEST && s->phase != TCU_DIAG_FLOW)
        return false;
    memset(out, 0, sizeof(*out));
    out->id = 0x100;
    if (s->phase == TCU_DIAG_REQUEST) {
        out->len = 8;
        out->data[0] = 3;
        out->data[1] = 0x22;
        out->data[2] = (uint8_t)(dids[s->index] >> 8);
        out->data[3] = (uint8_t)dids[s->index];
        s->used = s->expected = 0;
        s->next_sequence = 1;
    } else {
        out->len = 3;
        out->data[0] = 0x30;
    }
    s->deadline_ms = now + TCU_DIAG_TIMEOUT_MS;
    s->phase = TCU_DIAG_WAIT;
    return true;
}
static bool header_ok(tcu_diag_t *s, const uint8_t *p, size_t n) {
    return n >= 3 && p[0] == 0x62 && p[1] == (uint8_t)(dids[s->index] >> 8) &&
           p[2] == (uint8_t)dids[s->index];
}
static void complete(tcu_diag_t *s) {
    if (!header_ok(s, s->response, s->used)) {
        tcu_diag_abort(s);
        return;
    }
    tcu_diag_result_t *r = &s->results[s->index];
    r->did = dids[s->index];
    r->len = s->used - 3;
    memcpy(r->data, s->response + 3, r->len);
    s->index++;
    s->phase = s->index == TCU_DIAG_COUNT ? TCU_DIAG_DONE : TCU_DIAG_REQUEST;
}
void tcu_diag_receive(tcu_diag_t *s, uint64_t now, uint16_t id, const uint8_t *d, size_t len) {
    if (!tick(s, now) || id != 0x101 || s->phase != TCU_DIAG_WAIT)
        return;
    if (!d || len < 1 || len > 8) {
        tcu_diag_abort(s);
        return;
    }
    unsigned kind = d[0] >> 4;
    if (kind == 0) {
        size_t n = d[0] & 15;
        if (!n || n > len - 1 || s->expected) {
            tcu_diag_abort(s);
            return;
        }
        if (n >= 3 && d[1] == 0x7f && d[2] == 0x22) {
            s->negative_code = d[3];
            tcu_diag_abort(s);
            return;
        }
        if (!header_ok(s, d + 1, n))
            return; /* Unrelated/stale DID; never extends deadline. */
        memcpy(s->response, d + 1, n);
        s->used = (uint8_t)n;
        complete(s);
    } else if (kind == 1) {
        if (len < 5 || s->expected) {
            tcu_diag_abort(s);
            return;
        }
        size_t n = ((size_t)(d[0] & 15) << 8) | d[1];
        if (n <= 7 || n > TCU_DIAG_MAX_RESPONSE || !header_ok(s, d + 2, len - 2)) {
            tcu_diag_abort(s);
            return;
        }
        s->expected = (uint8_t)n;
        s->used = (uint8_t)(len - 2);
        memcpy(s->response, d + 2, s->used);
        s->phase = TCU_DIAG_FLOW;
        s->deadline_ms = now + TCU_DIAG_TIMEOUT_MS;
    } else if (kind == 2) {
        if (!s->expected || len < 2 || (d[0] & 15) != s->next_sequence) {
            tcu_diag_abort(s);
            return;
        }
        size_t n = len - 1, remain = s->expected - s->used;
        if (n > remain)
            n = remain;
        memcpy(s->response + s->used, d + 1, n);
        s->used += (uint8_t)n;
        s->next_sequence = (s->next_sequence + 1) & 15;
        s->deadline_ms = now + TCU_DIAG_TIMEOUT_MS;
        if (s->used == s->expected)
            complete(s);
    } else
        tcu_diag_abort(s);
}
