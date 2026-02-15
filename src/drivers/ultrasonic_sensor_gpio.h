// drivers/ultrasonic_sensor_gpio.h
#pragma once

#include "interfaces/obstacle_sensor.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * UltrasonicSensorContext
     *
     * Why this exists:
     *   The context is caller-owned so you can create multiple sensors without
     *   hidden global state. The context must outlive the ObstacleSensor that
     *   points to it (e.g., static or global storage).
     */
    typedef struct
    {
        int echo_pin;
        int trig_pin;
        float detection_threshold_cm; // Distance threshold in centimeters
    } UltrasonicSensorContext;

    /*
     * Create a GPIO-backed ultrasonic sensor (HC-SR04).
     *
     * The HC-SR04 ultrasonic sensor measures distance by sending ultrasonic
     * pulses and measuring the echo return time.
     *
     * echo_pin: GPIO pin connected to the Echo pin of the sensor
     * trig_pin: GPIO pin connected to the Trig pin of the sensor
     * detection_threshold_cm: Maximum distance (in cm) to consider as "obstacle detected"
     *                         Objects closer than this distance will trigger detection.
     *
     * Note: The sensor returns true if an obstacle is detected within the threshold distance.
     */
    ObstacleSensor ultrasonic_sensor_gpio_create(
        UltrasonicSensorContext *context,
        int echo_pin,
        int trig_pin,
        float detection_threshold_cm);

#ifdef __cplusplus
} // extern "C"
#endif
