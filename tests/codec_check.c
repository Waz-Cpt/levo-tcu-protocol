#include "levo_protocol.h"
#include <stdio.h>
#include <assert.h>
int main(void) {
    uint8_t out[8];
    const uint8_t known[8] = {0x03, 0x5a, 0x64, 0x5a, 0x23, 0, 0x23, 0xea};
    assert(levo_command_valid(known, 8));
    assert(!levo_command_valid(known, 7));
    assert(!levo_command_encode(known, 16, out));
    levo_telemetry_t s = {0};
    const uint8_t speed[] = {0x10, 0x04, 0, 0, 0x62, 0};
    assert(levo_telemetry_decode(0x201, speed, 6, &s));
    assert(s.speed_kmh > 10.399f && s.speed_kmh < 10.401f);
    const uint8_t cadence[] = {0xbd, 0x11, 0, 0, 0, 0, 0, 0};
    assert(levo_telemetry_decode(0x203, cadence, 8, &s));
    assert(s.cadence_rpm > 113.524f && s.cadence_rpm < 113.526f);
    unsigned v[8];
    unsigned count = 0;
    while (scanf("%2x%2x%2x%2x%2x%2x%2x%2x", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6],
                 &v[7]) == 8) {
        uint8_t b[8];
        for (unsigned i = 0; i < 8; i++)
            b[i] = v[i];
        if (!levo_command_valid(b, 8))
            return 2;
        for (unsigned bit = 4; bit < 8; bit++) {
            b[7] ^= 1u << bit;
            assert(!levo_command_valid(b, 8));
            b[7] ^= 1u << bit;
        }
        count++;
    }
    printf("Validated %u captured commands and high-nibble corruption rejection.\n", count);
    return count ? 0 : 3;
}
