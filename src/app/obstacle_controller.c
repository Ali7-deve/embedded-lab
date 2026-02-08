// app/obstacle_controller.c
#include "obstacle_controller.h"
#include <stddef.h>

void obstacle_controller_update(ObstacleController *controller)
{
    // Guard against partially wired controllers so the system fails safe,
    // not by crashing when you add new components.
    if (controller == NULL ||
        controller->sensor.is_detected == NULL ||
        controller->indicator.set_status == NULL) {
        return;
    }

    bool detected = controller->sensor.is_detected(
        controller->sensor.context
    );

    SystemStatus status = detected
        ? SYSTEM_STATUS_OBSTACLE
        : SYSTEM_STATUS_CLEAR;

    // Level-triggered (old behavior): always call set_status every loop.
    // Edge-triggered (current behavior): only call when the state changes.
    if (!controller->has_last_status || status != controller->last_status) {
        controller->indicator.set_status(
            controller->indicator.context,
            status
        );
        controller->last_status = status;
        controller->has_last_status = true;
    }
}

