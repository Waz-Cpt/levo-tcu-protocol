#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Read-only UDS/ISO-TP startup client observed on SBC-D03.
 * Caller supplies monotonic milliseconds and explicitly sends returned frames.
 * No CAN hardware access, configuration writes, retries, or motor commands.
 */
#define TCU_DIAG_COUNT 8
#define TCU_DIAG_MAX_RESPONSE 64
#define TCU_DIAG_TIMEOUT_MS 1000

typedef enum {
    TCU_DIAG_IDLE,
    TCU_DIAG_REQUEST,
    TCU_DIAG_WAIT,
    TCU_DIAG_FLOW,
    TCU_DIAG_DONE,
    TCU_DIAG_FAILED
} tcu_diag_phase_t;
typedef struct {
    uint16_t id;
    uint8_t len, data[8];
} tcu_diag_frame_t;
typedef struct {
    uint16_t did;
    uint8_t len, data[TCU_DIAG_MAX_RESPONSE]; /* DID value, excluding 62/DID */
} tcu_diag_result_t;
typedef struct {
    tcu_diag_phase_t phase;
    uint64_t last_ms, deadline_ms;
    uint8_t index, used, expected, next_sequence, negative_code;
    uint8_t response[TCU_DIAG_MAX_RESPONSE];
    tcu_diag_result_t results[TCU_DIAG_COUNT];
} tcu_diag_t;
void tcu_diag_start(tcu_diag_t *s, uint64_t now_ms);
/* Poll regularly, even when RX is silent. Every true return is one outgoing frame.
 * A send failure must cause caller to abort (no successful send is presumed safe).
 */
bool tcu_diag_poll(tcu_diag_t *s, uint64_t now_ms, tcu_diag_frame_t *out);
void tcu_diag_receive(tcu_diag_t *s, uint64_t now_ms, uint16_t id, const uint8_t *data, size_t len);
void tcu_diag_abort(tcu_diag_t *s);
