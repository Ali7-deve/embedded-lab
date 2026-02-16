// main.cpp (Arduino entrypoint)
// Arduino requires setup() and loop() in C++.
// We keep the entrypoint tiny so the rest of the system stays in C modules.
#include <Arduino.h>

#include "app/obstacle_controller.h"
#include "drivers/ir_sensor_gpio.h"
#include "drivers/hall_sensor_gpio.h"
#include "drivers/ultrasonic_sensor_gpio.h"
#include "drivers/composite_sensor.h"
#include "drivers/led_indicator_gpio.h"
#include "drivers/button_gpio.h"

// Use static storage so contexts outlive the controller for the life of the app.
static IrSensorContext g_ir_context;
static IrSensorContext g_ir_context2;
static HallSensorContext g_hall_context;
static UltrasonicSensorContext g_ultrasonic_context;
static CompositeSensorContext g_composite_context;
static LedIndicatorContext g_led_context;
static ButtonContext g_button_context;
static Button g_button;
static ObstacleController g_controller;

#define SENSOR_COUNT 4
static ObstacleSensor g_sensors[SENSOR_COUNT];

void setup()
{
    // Hardware wiring happens once in Arduino's setup(), not in loop().
    // This keeps policy (controller) separate from hardware setup.

    g_sensors[0] = ir_sensor_gpio_create(&g_ir_context, 15, false);
    g_sensors[1] = ir_sensor_gpio_create(&g_ir_context2, 14, false);
    g_sensors[2] = hall_sensor_gpio_create(&g_hall_context, 13, false);
    g_sensors[3] = ultrasonic_sensor_gpio_create(&g_ultrasonic_context, 2, 4, 30.0f);

    ObstacleSensor composite_sensor =
        composite_sensor_create(&g_composite_context, g_sensors, SENSOR_COUNT);

    StatusIndicator indicator =
        led_indicator_gpio_create(
            &g_led_context,
            18,  // Green LED
            17,  // Red LED
            true // active HIGH LEDs
        );

    // Create push button
    g_button =
        button_gpio_create(
            &g_button_context,
            21,   // GPIO21
            false // active LOW = button pressed
        );

    g_controller.sensor = composite_sensor;
    g_controller.indicator = indicator;
    g_controller.has_last_status = false;
    g_controller.enabled = true;
}

void loop()
{
    if (g_button.is_pressed != NULL && g_button.is_pressed(g_button.context))
    {
        g_controller.enabled = !g_controller.enabled;
    }
    obstacle_controller_update(&g_controller);
    delay(50);
}
