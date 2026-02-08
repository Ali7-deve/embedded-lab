// interfaces/status_indicator.h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYSTEM_STATUS_CLEAR,
    SYSTEM_STATUS_OBSTACLE
} SystemStatus;

/*
 * StatusIndicator
 *
 * Responsibility:
 *   Represent the current system state.
 *
 * Contract:
 *   - No logic or decisions
 *   - No blocking or delays
 *   - Safe to call repeatedly
 */
typedef struct {
    void (*set_status)(void *context, SystemStatus status);
    void *context;
} StatusIndicator;

#ifdef __cplusplus
} // extern "C"
#endif
