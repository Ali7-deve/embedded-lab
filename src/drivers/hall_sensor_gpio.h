// drivers/hall_sensor_gpio.h
#pragma once

#include "interfaces/obstacle_sensor.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * HallSensorContext
     *
     * Why this exists:
     *   The context is caller-owned so you can create multiple sensors without
     *   hidden global state. The context must outlive the ObstacleSensor that
     *   points to it (e.g., static or global storage).
     */
    typedef struct
    {
        int gpio_pin;
        bool active_level;
    } HallSensorContext;

    /*
     * Create a GPIO-backed Hall Effect sensor (KY-003).
     *
     * The KY-003 Hall Effect Sensor Module detects magnetic fields.
     * When a magnet is detected, the sensor outputs a digital signal.
     *
     * active_level:
     *   - true  => GPIO HIGH means "magnetic field detected"
     *   - false => GPIO LOW  means "magnetic field detected"
     *
     * Note: The KY-003 typically outputs LOW when a magnet is detected (active LOW).
     */
    ObstacleSensor hall_sensor_gpio_create(
        HallSensorContext *context,
        int gpio_pin,
        bool active_level);

#ifdef __cplusplus
} // extern "C"
#endif
