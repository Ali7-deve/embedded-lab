// drivers/composite_sensor.h
#pragma once

#include "interfaces/obstacle_sensor.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * CompositeSensorContext
     *
     * Combines multiple sensors into a single ObstacleSensor interface.
     * Returns true if ANY of the sensors detect something (OR logic).
     *
     * Why this exists:
     *   Allows the controller to treat multiple sensors as one, maintaining
     *   the existing controller interface while supporting multiple sensors.
     */
    typedef struct
    {
        const ObstacleSensor *sensors;
        size_t sensor_count;
    } CompositeSensorContext;

    /*
     * Create a composite sensor that combines multiple sensors.
     *
     * The composite sensor returns true if ANY sensor detects something.
     * This allows multiple sensors (e.g., IR and Hall Effect) to work together.
     *
     * sensors: Array of ObstacleSensor instances (must outlive the composite)
     * count: Number of sensors in the array
     */
    ObstacleSensor composite_sensor_create(
        CompositeSensorContext *context,
        const ObstacleSensor *sensors,
        size_t count);

#ifdef __cplusplus
} // extern "C"
#endif
