#include "tcu_sequence.h"
#include "levo_protocol.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    tcu_seq_t s;
    tcu_diag_frame_t f;
    const uint8_t off[7] = {0, 0x5a, 0x64, 0x5a, 0, 0, 10};
    assert(tcu_seq_start(&s, off, 0));
    unsigned pairs = 0, commands = 0, boot = 0, running = 0;
    for (uint64_t now = 0; now < 25000000; now += 1000) {
        if (tcu_seq_poll(&s, now, &f)) {
            if (f.id == 0x300) {
                assert(f.len == 8 && levo_command_valid(f.data, 8));
                assert(f.data[0] == 0 && f.data[1] == 0x5a && f.data[4] == 0 && f.data[5] == 0 &&
                       f.data[6] == 10);
                if (commands < 99) {
                    assert(f.data[2] == 0);
                    boot++;
                } else if (commands < 400) {
                    assert(f.data[2] == 0x64);
                    running++;
                } else
                    assert(f.data[2] == 0);
                commands++;
            } else {
                assert(f.id == 0x301 && f.len == 3);
                if (++pairs == 400)
                    assert(tcu_seq_shutdown(&s));
            }
        }
        assert(s.phase != TCU_SEQ_FAULT);
        if (s.phase == TCU_SEQ_DONE)
            break;
    }
    assert(s.phase == TCU_SEQ_DONE && commands == 441 && pairs == 441 && boot == 99 &&
           running == 301);
    puts("PASS: 441 assist-OFF command/companion pairs; startup/run/shutdown remain OFF and "
         "complete within25s.");
}
