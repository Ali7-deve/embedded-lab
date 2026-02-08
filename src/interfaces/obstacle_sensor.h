// interfaces/obstacle_sensor.h
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ObstacleSensor
 *
 * Responsibility:
 *   Detect presence of an obstacle at the current time.
 *
 * Contract:
 *   - Non-blocking
 *   - No delays
 *   - No side effects
 *   - Safe to call repeatedly from main loop
 */
typedef struct {
    bool (*is_detected)(void *context);
    void *context;
} ObstacleSensor;

#ifdef __cplusplus
} // extern "C"
#endif
