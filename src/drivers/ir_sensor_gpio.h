// drivers/ir_sensor_gpio.h
#pragma once

#include "interfaces/obstacle_sensor.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IrSensorContext
 *
 * Why this exists:
 *   The context is caller-owned so you can create multiple sensors without
 *   hidden global state. The context must outlive the ObstacleSensor that
 *   points to it (e.g., static or global storage).
 */
typedef struct {
    int gpio_pin;
    bool active_level;
} IrSensorContext;

/*
 * Create a GPIO-backed IR sensor.
 *
 * active_level:
 *   - true  => GPIO HIGH means "obstacle detected"
 *   - false => GPIO LOW  means "obstacle detected"
 */
ObstacleSensor ir_sensor_gpio_create(
    IrSensorContext *context,
    int gpio_pin,
    bool active_level
);

#ifdef __cplusplus
} // extern "C"
#endif
