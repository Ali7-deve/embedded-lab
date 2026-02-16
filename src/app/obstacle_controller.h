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
    SystemStatus last_status;
    bool has_last_status;
    bool enabled;
} ObstacleController;

void obstacle_controller_update(ObstacleController *controller);
void obstacle_controller_set_enabled(ObstacleController *controller, bool enabled);

#ifdef __cplusplus
} // extern "C"
#endif
