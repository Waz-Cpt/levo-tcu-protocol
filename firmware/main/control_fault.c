#include "control_fault.h"
#include <stdatomic.h>

static atomic_uint latched;
void control_fault_set(control_fault_code_t code, uint16_t detail) {
    if (code <= CONTROL_OK || code >= CONTROL_FAULT_COUNT)
        return;
    unsigned expected = 0;
    atomic_compare_exchange_strong(&latched, &expected, (unsigned)code | ((unsigned)detail << 8));
}
control_fault_t control_fault_get(void) {
    unsigned value = atomic_load(&latched);
    return (control_fault_t){.code = value & 255, .detail = value >> 8};
}
static const char *const names[] = {"none",
                                    "can_init",
                                    "can_start",
                                    "stock_tcu_conflict",
                                    "can_tx",
                                    "can_bus",
                                    "can_rx",
                                    "can_timing",
                                    "startup_diagnostic",
                                    "motor_timeout",
                                    "clock",
                                    "control_profile",
                                    "can_status",
                                    "storage",
                                    "unexpected_reset"};
static const char *const labels[] = {
    "NO FAULT",    "CAN INIT",        "CAN START",  "TCU CONFLICT",  "CAN SEND",
    "CAN BUS",     "CAN RECEIVE",     "CAN TIMING", "STARTUP READ",  "MOTOR DATA LOST",
    "CLOCK ERROR", "CONTROL PROFILE", "CAN STATUS", "STORAGE ERROR", "RESET STOP"};
_Static_assert(sizeof(names) / sizeof(*names) == CONTROL_FAULT_COUNT, "fault names");
_Static_assert(sizeof(labels) / sizeof(*labels) == CONTROL_FAULT_COUNT, "fault labels");
const char *control_fault_name(control_fault_code_t code) {
    return (unsigned)code < CONTROL_FAULT_COUNT ? names[code] : "unknown";
}
const char *control_fault_label(control_fault_code_t code) {
    return (unsigned)code < CONTROL_FAULT_COUNT ? labels[code] : "UNKNOWN ERROR";
}
