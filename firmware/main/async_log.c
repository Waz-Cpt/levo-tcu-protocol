// CAN never waits for USB. Diagnostic output may be dropped when USB is absent.
#include "async_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
typedef struct {
    unsigned len;
    char data[320];
} log_line_t;
static QueueHandle_t queue;
static void writer(void *unused) {
    (void)unused;
    log_line_t line;
    for (;;)
        if (xQueueReceive(queue, &line, portMAX_DELAY) == pdTRUE)
            fwrite(line.data, 1, line.len, stdout);
}
void async_log_start(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    queue = xQueueCreate(64, sizeof(log_line_t));
    configASSERT(queue);
    configASSERT(xTaskCreate(writer, "usb_log", 3072, NULL, 1, NULL) == pdPASS);
}
int async_log_printf(const char *format, ...) {
    log_line_t line;
    va_list args;
    va_start(args, format);
    int n = vsnprintf(line.data, sizeof(line.data), format, args);
    va_end(args);
    if (n < 0)
        return n;
    line.len = n < (int)sizeof(line.data) ? (unsigned)n : sizeof(line.data) - 1;
    if (queue)
        xQueueSend(queue, &line, 0);
    return n;
}
int async_log_puts(const char *s) {
    return async_log_printf("%s\n", s);
}
