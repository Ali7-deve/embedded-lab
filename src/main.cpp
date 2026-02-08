// main.cpp (Arduino entrypoint)
// Arduino requires setup() and loop() in C++.
// We keep the entrypoint tiny so the rest of the system stays in C modules.
#include <Arduino.h>

#include "app/obstacle_controller.h"
#include "drivers/ir_sensor_gpio.h"
#include "drivers/led_indicator_gpio.h"

// Use static storage so contexts outlive the controller for the life of the app.
static IrSensorContext g_ir_context;
static LedIndicatorContext g_led_context;
static ObstacleController g_controller;

void setup()
{
    // Hardware wiring happens once in Arduino's setup(), not in loop().
    // This keeps policy (controller) separate from hardware setup.

    ObstacleSensor sensor =
        ir_sensor_gpio_create(
            &g_ir_context,
            15,     // GPIO15
            false   // active LOW = obstacle detected
        );

    StatusIndicator indicator =
        led_indicator_gpio_create(
            &g_led_context,
            18,     // Green LED GPIO on the ESP32 board
            17,     // Red LED GPIO on the ESP32 board
            true    // active HIGH LEDs
        );

    g_controller.sensor = sensor;
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
