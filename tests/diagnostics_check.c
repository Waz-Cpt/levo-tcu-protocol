#include "tcu_diagnostics.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void faults(void) {
    tcu_diag_t s;
    tcu_diag_frame_t out;
    tcu_diag_start(&s, 0);
    assert(tcu_diag_poll(&s, 0, &out));
    assert(!tcu_diag_poll(&s, 1000, &out));
    assert(s.phase == TCU_DIAG_FAILED);
    tcu_diag_start(&s, 100);
    assert(!tcu_diag_poll(&s, 99, &out));
    assert(s.phase == TCU_DIAG_FAILED);
    tcu_diag_start(&s, 0);
    assert(tcu_diag_poll(&s, 0, &out));
    uint8_t wrong[] = {5, 0x62, 2, 6, 0x70, 8};
    tcu_diag_receive(&s, 10, 0x101, wrong, sizeof(wrong));
    assert(s.phase == TCU_DIAG_WAIT);
    tcu_diag_poll(&s, 1000, &out);
    assert(s.phase == TCU_DIAG_FAILED);
    tcu_diag_start(&s, 0);
    assert(tcu_diag_poll(&s, 0, &out));
    uint8_t negative[] = {3, 0x7f, 0x22, 0x31};
    tcu_diag_receive(&s, 10, 0x101, negative, sizeof(negative));
    assert(s.phase == TCU_DIAG_FAILED && s.negative_code == 0x31);
    tcu_diag_start(&s, 0);
    assert(tcu_diag_poll(&s, 0, &out));
    uint8_t first[] = {0x10, 9, 0x62, 2, 3, 1, 2, 3};
    tcu_diag_receive(&s, 1, 0x101, first, 8);
    assert(s.phase == TCU_DIAG_FLOW);
    assert(tcu_diag_poll(&s, 2, &out));
    assert(out.len == 3 && out.data[0] == 0x30);
    uint8_t wrongseq[] = {0x22, 4, 5, 6, 0, 0, 0, 0};
    tcu_diag_receive(&s, 3, 0x101, wrongseq, 8);
    assert(s.phase == TCU_DIAG_FAILED);
    tcu_diag_start(&s, 0);
    assert(tcu_diag_poll(&s, 0, &out));
    first[1] = 65;
    tcu_diag_receive(&s, 1, 0x101, first, 8);
    assert(s.phase == TCU_DIAG_FAILED);
}
int main(void) {
    faults();
    tcu_diag_t s;
    tcu_diag_frame_t out;
    unsigned ms, id, len, v, frames = 0, tx = 0;
    tcu_diag_start(&s, 0);
    while (scanf("%u %x %u", &ms, &id, &len) == 3) {
        assert(len <= 8);
        uint8_t d[8] = {0};
        for (unsigned i = 0; i < len; i++) {
            assert(scanf("%x", &v) == 1);
            d[i] = (uint8_t)v;
        }
        if (id == 0x100) {
            assert(tcu_diag_poll(&s, ms, &out));
            assert(out.id == id && out.len == len);
            assert(!memcmp(out.data, d, len));
            tx++;
        } else {
            tcu_diag_receive(&s, ms, (uint16_t)id, d, len);
        }
        assert(s.phase != TCU_DIAG_FAILED);
        frames++;
    }
    assert(frames == 28 && tx == 11 && s.phase == TCU_DIAG_DONE && s.index == 8);
    assert(s.results[4].did == 0x0206 && s.results[4].len == 2);
    assert(s.results[4].data[0] == 0x70 && s.results[4].data[1] == 8);
    assert(!tcu_diag_poll(&s, s.last_ms + 1, &out));
    printf("PASS: replayed %u diagnostic fixture frames (identity values redacted), exactly "
           "matched all %u outgoing frames; all 8 results assembled. Timeout, clock regression, "
           "wrong DID, negative response, wrong sequence, oversized response rejected.\n",
           frames, tx);
}
