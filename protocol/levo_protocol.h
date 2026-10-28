#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Empirical SBC-D03 codec. No hardware I/O. Used by the replacement tested on
 * one Levo; see docs/can-protocol.md for scope and remaining unknowns. */
bool levo_command_encode(const uint8_t payload[7], uint8_t counter, uint8_t out[8]);
bool levo_command_valid(const uint8_t *data, size_t len);
typedef struct {
    float speed_kmh, cadence_rpm;
    uint16_t load_raw;
    uint32_t power_product_raw, odometer_metres;
    uint16_t status_raw;
} levo_telemetry_t;
/* Returns true only for a known frame with the exact observed length. */
bool levo_telemetry_decode(uint32_t id, const uint8_t *data, size_t len, levo_telemetry_t *state);
bool levo_companion_encode(uint8_t counter, uint8_t out[3]);
