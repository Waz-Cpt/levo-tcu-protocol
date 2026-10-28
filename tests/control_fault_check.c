#include "control_fault.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    control_fault_t fault = control_fault_get();
    assert(fault.code == CONTROL_OK && fault.detail == 0);
    control_fault_set(CONTROL_OK, 123);
    control_fault_set(CONTROL_FAULT_COUNT, 123);
    assert(control_fault_get().code == CONTROL_OK); /* Normal shutdown has no error. */
    for (unsigned i = 0; i < CONTROL_FAULT_COUNT; i++) {
        assert(strcmp(control_fault_name(i), "unknown"));
        assert(strlen(control_fault_label(i)) <= 16);
        for (unsigned j = 0; j < i; j++)
            assert(strcmp(control_fault_name(i), control_fault_name(j)));
    }
    assert(!strcmp(control_fault_name(CONTROL_DIAGNOSTIC), "startup_diagnostic"));
    assert(!strcmp(control_fault_name(CONTROL_CAN_RX), "can_rx"));
    assert(!strcmp(control_fault_name(CONTROL_TIMING), "can_timing"));
    control_fault_set(CONTROL_DIAGNOSTIC, 4);
    control_fault_set(CONTROL_CAN_TX, 0); /* Cleanup failures must not hide first cause. */
    control_fault_set(CONTROL_OK, 0);
    fault = control_fault_get();
    assert(fault.code == CONTROL_DIAGNOSTIC && fault.detail == 4);
    assert(!strcmp(control_fault_name((control_fault_code_t)-1), "unknown"));
    assert(!strcmp(control_fault_label(CONTROL_FAULT_COUNT), "UNKNOWN ERROR"));
    puts("PASS boot fault latch, first-cause retention, deliberate-stop distinction, detail and "
         "label mappings");
}
