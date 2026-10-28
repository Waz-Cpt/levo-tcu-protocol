#include "reset_policy.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

int main(void) {
    const esp_reset_reason_t allowed[] = {
        ESP_RST_POWERON, ESP_RST_EXT, ESP_RST_SW, ESP_RST_DEEPSLEEP, ESP_RST_USB, ESP_RST_JTAG,
    };
    const esp_reset_reason_t blocked[] = {
        ESP_RST_UNKNOWN,  ESP_RST_PANIC, ESP_RST_INT_WDT, ESP_RST_TASK_WDT,   ESP_RST_WDT,
        ESP_RST_BROWNOUT, ESP_RST_SDIO,  ESP_RST_EFUSE,   ESP_RST_PWR_GLITCH, ESP_RST_CPU_LOCKUP,
    };
    for (unsigned i = 0; i < sizeof(allowed) / sizeof(allowed[0]); i++)
        assert(!reset_requires_stop(allowed[i]));
    for (unsigned i = 0; i < sizeof(blocked) / sizeof(blocked[0]); i++)
        assert(reset_requires_stop(blocked[i]));

    /* Unknown/future values must stay blocked until explicitly reviewed. */
    assert(reset_requires_stop((esp_reset_reason_t)-1));
    assert(reset_requires_stop((esp_reset_reason_t)(ESP_RST_CPU_LOCKUP + 1)));
    assert(reset_requires_stop((esp_reset_reason_t)127));
    assert(reset_requires_stop((esp_reset_reason_t)INT_MAX));
    for (int reason = -4; reason <= 255; reason++) {
        bool expected = true;
        for (unsigned i = 0; i < sizeof(allowed) / sizeof(allowed[0]); i++)
            if (reason == (int)allowed[i])
                expected = false;
        assert(reset_requires_stop((esp_reset_reason_t)reason) == expected);
    }
    puts("PASS: six deliberate reset reasons allowed; faults, unknown and future reasons require "
         "a deliberate power cycle.");
    return 0;
}
