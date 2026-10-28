#pragma once
#ifndef LEVO_RESET_POLICY_HOST_TEST
#error "This enum stub is restricted to reset_policy_check; production uses ESP-IDF."
#endif

/* Host-only names/values from ESP-IDF 5.4.2 esp_system.h. No peripheral stubs.
 * The test runner adds this include directory only for reset_policy_check. */
typedef enum {
    ESP_RST_UNKNOWN,
    ESP_RST_POWERON,
    ESP_RST_EXT,
    ESP_RST_SW,
    ESP_RST_PANIC,
    ESP_RST_INT_WDT,
    ESP_RST_TASK_WDT,
    ESP_RST_WDT,
    ESP_RST_DEEPSLEEP,
    ESP_RST_BROWNOUT,
    ESP_RST_SDIO,
    ESP_RST_USB,
    ESP_RST_JTAG,
    ESP_RST_EFUSE,
    ESP_RST_PWR_GLITCH,
    ESP_RST_CPU_LOCKUP,
} esp_reset_reason_t;
