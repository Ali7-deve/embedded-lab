#include "obstacle_controller.h"
#include "interfaces/button.h"
#include "utils/profiler.h"
#include <stddef.h>

void obstacle_controller_update(ObstacleController *controller)
{
    profiler_start(PROFILER_CONTROLLER_UPDATE);
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
    
    profiler_stop(PROFILER_CONTROLLER_UPDATE);
}

void obstacle_controller_set_enabled(ObstacleController *controller, bool enabled)
{
    if (controller != NULL) {
        controller->enabled = enabled;
    }
}
