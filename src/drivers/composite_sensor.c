#include "drivers/composite_sensor.h"
#include <stddef.h>

static bool composite_is_detected(void *context)
{
    CompositeSensorContext *ctx = (CompositeSensorContext *)context;

    if (ctx == NULL || ctx->sensors == NULL || ctx->sensor_count == 0)
    {
        // Fail safe: no context or sensors means nothing detected.
        return false;
    }

    // Check all sensors: return true if ANY sensor detects something (OR logic).
    for (size_t i = 0; i < ctx->sensor_count; i++)
    {
        if (ctx->sensors[i].is_detected != NULL)
        {
            if (ctx->sensors[i].is_detected(ctx->sensors[i].context))
            {
                return true;
            }
        }
    }

    return false;
}

ObstacleSensor composite_sensor_create(
    CompositeSensorContext *context,
    const ObstacleSensor *sensors,
    size_t count)
{
    ObstacleSensor sensor = {
        .is_detected = NULL,
        .context = NULL};

    if (context == NULL || sensors == NULL || count == 0)
    {
        // Without valid context and sensors, we can't create a composite.
        return sensor;
    }

    context->sensors = sensors;
    context->sensor_count = count;

    sensor.is_detected = composite_is_detected;
    sensor.context = context;

    return sensor;
}
