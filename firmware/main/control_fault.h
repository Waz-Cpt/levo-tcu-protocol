#pragma once
#include <stdint.h>

typedef enum {
    CONTROL_OK,
    CONTROL_CAN_INIT,
    CONTROL_CAN_START,
    CONTROL_TCU_CONFLICT,
    CONTROL_CAN_TX,
    CONTROL_CAN_BUS,
    CONTROL_CAN_RX,
    CONTROL_TIMING,
    CONTROL_DIAGNOSTIC,
    CONTROL_MOTOR_TIMEOUT,
    CONTROL_CLOCK,
    CONTROL_PROFILE,
    CONTROL_CAN_STATUS,
    CONTROL_STORAGE,
    CONTROL_RESET,
    CONTROL_FAULT_COUNT
} control_fault_code_t;
typedef struct {
    control_fault_code_t code;
    uint16_t detail;
} control_fault_t;

/* First cause is retained until restart. This reports faults, not control policy. */
void control_fault_set(control_fault_code_t code, uint16_t detail);
control_fault_t control_fault_get(void);
const char *control_fault_name(control_fault_code_t code);
const char *control_fault_label(control_fault_code_t code);
