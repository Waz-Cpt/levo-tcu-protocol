#include "levo_protocol.h"
#include <string.h>
static uint16_t u16(const uint8_t *b) {
    return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}
static uint32_t u32(const uint8_t *b) {
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}
bool levo_command_encode(const uint8_t p[7], uint8_t counter, uint8_t out[8]) {
    if (!p || !out || counter > 15)
        return false;
    uint8_t check = p[0] & 15;
    for (unsigned i = 1; i < 7; i++)
        check ^= (p[i - 1] >> 4) | (p[i] & 15);
    check ^= (p[6] >> 4) | counter;
    memmove(out, p, 7);
    out[7] = ((check & 15) << 4) | counter;
    return true;
}
bool levo_command_valid(const uint8_t *data, size_t len) {
    if (!data || len != 8)
        return false;
    uint8_t encoded[8];
    return levo_command_encode(data, data[7] & 15, encoded) && encoded[7] == data[7];
}
bool levo_telemetry_decode(uint32_t id, const uint8_t *b, size_t len, levo_telemetry_t *s) {
    if (!b || !s)
        return false;
    if (id == 0x201 && len == 6) {
        s->speed_kmh = u16(b) / 100.0f;
        s->load_raw = u16(b + 2);
        s->status_raw = u16(b + 4);
        return true;
    }
    if (id == 0x203 && len == 8) {
        s->cadence_rpm = u16(b) / 40.0f;
        s->power_product_raw = u32(b + 2);
        return true;
    }
    if (id == 0x202 && len == 8) {
        s->odometer_metres = u32(b + 4);
        return true;
    }
    return false;
}
bool levo_companion_encode(uint8_t counter, uint8_t out[3]) {
    static const uint8_t table[16] = {0xe2, 0xb3, 0x6b, 0x96, 0x03, 0x54, 0xf5, 0xb8,
                                      0x80, 0x88, 0xc6, 0x43, 0xb5, 0x4e, 0x1d, 0xe8};
    if (!out || counter > 15)
        return false;
    out[0] = table[counter];
    out[1] = counter;
    out[2] = 0;
    return true;
}
