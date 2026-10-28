#pragma once
void async_log_start(void);
int async_log_printf(const char *format, ...) __attribute__((format(printf, 1, 2)));
int async_log_puts(const char *s);
