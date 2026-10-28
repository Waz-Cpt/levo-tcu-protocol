#pragma once
#include <stdbool.h>
#include "esp_system.h"

/* Deliberate reset paths may boot the controller. Faults and new/unknown reasons
 * require a stopped boot without starting CAN. Use IDF's enum, not numeric IDs. */
static inline bool reset_requires_stop(esp_reset_reason_t reason) {
    switch (reason) {
    case ESP_RST_POWERON:
    case ESP_RST_EXT:
    case ESP_RST_SW:
    case ESP_RST_DEEPSLEEP:
    case ESP_RST_USB:
    case ESP_RST_JTAG:
        return false;
    default:
        return true;
    }
}
