// app/obstacle_controller.h
#pragma once

// Keep public interfaces in "interfaces/" so callers include by role, not file location.
#include "interfaces/obstacle_sensor.h"
#include "interfaces/status_indicator.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ObstacleController
 *
 * Responsibility:
 *   Map sensor input to system state.
 *
 * Notes:
 *   - No hardware access
 *   - No delays
 *   - No policy beyond mapping
 */
typedef struct {
    ObstacleSensor sensor;
    StatusIndicator indicator;
    // Edge-triggered support:
    // We store the last published status so we only update outputs on changes.
    SystemStatus last_status;
    bool has_last_status;
} ObstacleController;

/*
 * Perform one control step.
 * Safe to call repeatedly from main loop.
 */
void obstacle_controller_update(ObstacleController *controller);

#ifdef __cplusplus
} // extern "C"
#endif
