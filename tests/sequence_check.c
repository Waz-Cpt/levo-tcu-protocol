#include "tcu_sequence.h"
#include "levo_protocol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    tcu_seq_t s;
    tcu_diag_frame_t out;
    const uint8_t trail[7] = {3, 0x5a, 0x64, 0x5a, 50, 0, 70};
    assert(tcu_seq_start(&s, trail, 0));
    unsigned id, len, v, rows = 0, cycle = 0;
    /* Fixture is the actual startup's 426 observed 300/301 pairs. */
    while (scanf("%x %u", &id, &len) == 2) {
        uint8_t b[8];
        assert(len <= 8);
        for (unsigned i = 0; i < len; i++) {
            assert(scanf("%x", &v) == 1);
            b[i] = (uint8_t)v;
        }
        uint64_t at = (uint64_t)cycle * 50000 + (id == 0x301 ? 1000 : 0);
        assert(tcu_seq_poll(&s, at, &out));
        assert(out.id == id && out.len == len);
        assert(!memcmp(b, out.data, len));
        rows++;
        if (id == 0x301)
            cycle++;
    }
    assert(rows == 852 && s.phase == TCU_SEQ_RUN);
    assert(tcu_seq_shutdown(&s));
    for (unsigned i = 0; i < 41; i++) {
        uint64_t at = (uint64_t)(cycle + i) * 50000;
        assert(tcu_seq_poll(&s, at, &out));
        assert(out.id == 0x300 && levo_command_valid(out.data, 8));
        assert(out.data[2] == 0);
        if (i < 31) {
            assert(out.data[0] == 0 && out.data[3] == 0x5a && out.data[4] == 0 &&
                   out.data[6] == 10);
        } else {
            assert(out.data[0] == 3 && out.data[3] == 0xa5 && out.data[4] == 50 &&
                   out.data[6] == 70);
        }
        assert(tcu_seq_poll(&s, at + 1000, &out));
        assert(out.id == 0x301);
    }
    assert(!tcu_seq_poll(&s, (uint64_t)(cycle + 41) * 50000, &out));
    assert(s.phase == TCU_SEQ_DONE);
    assert(tcu_seq_start(&s, trail, 0));
    assert(!tcu_seq_poll(&s, 10001, &out));
    assert(s.phase == TCU_SEQ_FAULT);
    assert(tcu_seq_start(&s, trail, 100));
    assert(!tcu_seq_poll(&s, 99, &out));
    assert(s.phase == TCU_SEQ_FAULT);
    assert(tcu_seq_start(&s, trail, 0));
    assert(tcu_seq_poll(&s, 0, &out));
    assert(tcu_seq_shutdown(&s));
    assert(tcu_seq_poll(&s, 1000, &out) && out.id == 0x301);
    assert(s.shutdown_cycles == 0);
    assert(tcu_seq_poll(&s, 50000, &out) && out.id == 0x300 && s.shutdown_cycles == 1);
    puts("PASS: all 852 startup frames reproduced byte-for-byte; bounded shutdown sequence, "
         "counter wrap, pending companion, lateness and clock regression checked.");
}
