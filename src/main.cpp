// main.cpp (Arduino entrypoint)
// Arduino requires setup() and loop() in C++.
// We keep the entrypoint tiny so the rest of the system stays in C modules.
#include <Arduino.h>

#include "app/obstacle_controller.h"
#include "drivers/ir_sensor_gpio.h"
#include "drivers/hall_sensor_gpio.h"
#include "drivers/composite_sensor.h"
#include "drivers/led_indicator_gpio.h"

// Use static storage so contexts outlive the controller for the life of the app.
static IrSensorContext g_ir_context;
static IrSensorContext g_ir_context2;
static HallSensorContext g_hall_context;
static CompositeSensorContext g_composite_context;
static LedIndicatorContext g_led_context;
static ObstacleController g_controller;

void setup()
{
    // Hardware wiring happens once in Arduino's setup(), not in loop().
    // This keeps policy (controller) separate from hardware setup.

    // Create IR sensor (GPIO15) - detects obstacles
    ObstacleSensor ir_sensor =
        ir_sensor_gpio_create(
            &g_ir_context,
            15,   // GPIO15
            false // active LOW = obstacle detected
        );

    // Create second IR sensor
    ObstacleSensor ir_sensor2 =
        ir_sensor_gpio_create(
            &g_ir_context2,
            14,   // GPIO14
            false // active LOW = obstacle detected
        );

    // Create KY-003 Hall Effect sensor, detects magnetic fields
    ObstacleSensor hall_sensor =
        hall_sensor_gpio_create(
            &g_hall_context,
            13,   // GPIO13
            false // active LOW = magnetic field detected
        );

    // Combine all sensors: system triggers if ANY sensor detects something
    ObstacleSensor sensors[] = {ir_sensor, ir_sensor2, hall_sensor};
    ObstacleSensor composite_sensor =
        composite_sensor_create(
            &g_composite_context,
            sensors,
            3 // number of sensors
        );

    StatusIndicator indicator =
        led_indicator_gpio_create(
            &g_led_context,
            18,  // Green LED
            17,  // Red LED
            true // active HIGH LEDs
        );

    g_controller.sensor = composite_sensor;
    g_controller.indicator = indicator;
    // Clear the cached state so the first loop publishes a status.
    g_controller.has_last_status = false;
}

void loop()
{
    // One control step per loop; delay yields to other FreeRTOS tasks.
    obstacle_controller_update(&g_controller);
    delay(50); // 20 Hz update rate
}
