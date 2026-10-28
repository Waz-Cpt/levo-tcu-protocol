#pragma once
#include <stdbool.h>

typedef enum {
    CAN_CLEANUP_RUNNING,
    CAN_CLEANUP_STOPPED,
    CAN_CLEANUP_BUS_OFF,
    CAN_CLEANUP_UNKNOWN
} can_cleanup_state_t;

typedef struct {
    bool (*isolate)(void *context);
    bool (*status)(void *context, can_cleanup_state_t *state);
    bool (*stop)(void *context);
    bool (*uninstall)(void *context);
} can_cleanup_ops_t;

typedef struct {
    bool isolated;
    bool driver_released;
    bool stop_attempted;
    bool stop_succeeded;
} can_cleanup_result_t;

/* Isolate TX first, even when driver state cannot be read. A RUNNING -> BUS_OFF
 * transition can reject stop but still allow uninstall. Never recover/restart or
 * abort here: on an uninstall failure the isolated driver remains allocated. */
static inline can_cleanup_result_t can_cleanup(bool installed, const can_cleanup_ops_t *ops,
                                               void *context) {
    can_cleanup_result_t result = {.isolated = ops->isolate(context),
                                   .driver_released = !installed};
    if (!installed)
        return result;
    can_cleanup_state_t state = CAN_CLEANUP_UNKNOWN;
    bool have_state = ops->status(context, &state);
    if (!have_state || (state != CAN_CLEANUP_STOPPED && state != CAN_CLEANUP_BUS_OFF)) {
        result.stop_attempted = true;
        result.stop_succeeded = ops->stop(context);
    }
    result.driver_released = ops->uninstall(context);
    return result;
}
