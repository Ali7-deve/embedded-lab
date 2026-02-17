#include "drivers/composite_sensor.h"
#include "utils/profiler.h"
#include <stddef.h>

static ProfilerId sensor_profiler_ids[] = {
    PROFILER_IR_SENSOR_1,
    PROFILER_IR_SENSOR_2,
    PROFILER_HALL_SENSOR,
    PROFILER_ULTRASONIC_SENSOR};

static bool composite_is_detected(void *context)
{
    profiler_start(PROFILER_COMPOSITE_SENSOR);

    CompositeSensorContext *ctx = (CompositeSensorContext *)context;

    if (ctx == NULL || ctx->sensors == NULL || ctx->sensor_count == 0)
    {
        profiler_stop(PROFILER_COMPOSITE_SENSOR);
        return false;
    }

    for (size_t i = 0; i < ctx->sensor_count; i++)
    {
        if (ctx->sensors[i].is_detected != NULL)
        {
            if (i < sizeof(sensor_profiler_ids) / sizeof(sensor_profiler_ids[0]))
            {
                profiler_start(sensor_profiler_ids[i]);
            }
            bool result = ctx->sensors[i].is_detected(ctx->sensors[i].context);
            if (i < sizeof(sensor_profiler_ids) / sizeof(sensor_profiler_ids[0]))
            {
                profiler_stop(sensor_profiler_ids[i]);
            }
            if (result)
            {
                profiler_stop(PROFILER_COMPOSITE_SENSOR);
                return true;
            }
        }
    }

    profiler_stop(PROFILER_COMPOSITE_SENSOR);
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
