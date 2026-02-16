// app/obstacle_controller.c
#include "obstacle_controller.h"
#include "interfaces/button.h"
#include <stddef.h>

void obstacle_controller_update(ObstacleController *controller)
{
    if (controller == NULL ||
        controller->sensor.is_detected == NULL ||
        controller->indicator.set_status == NULL) {
        return;
    }

    bool detected = controller->enabled && controller->sensor.is_detected(
        controller->sensor.context
    );

    SystemStatus status = detected
        ? SYSTEM_STATUS_OBSTACLE
        : SYSTEM_STATUS_CLEAR;

    if (!controller->has_last_status || status != controller->last_status) {
        controller->indicator.set_status(
            controller->indicator.context,
            status
        );
        controller->last_status = status;
        controller->has_last_status = true;
    }
}

void obstacle_controller_set_enabled(ObstacleController *controller, bool enabled)
{
    if (controller != NULL) {
        controller->enabled = enabled;
    }
}
