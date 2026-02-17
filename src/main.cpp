#include <Arduino.h>

#include "app/obstacle_controller.h"
#include "drivers/ir_sensor_gpio.h"
#include "drivers/hall_sensor_gpio.h"
#include "drivers/ultrasonic_sensor_gpio.h"
#include "drivers/composite_sensor.h"
#include "drivers/led_indicator_gpio.h"
#include "drivers/button_gpio.h"
#include "utils/profiler.h"

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
    Serial.begin(115200);
    delay(1000);

    profiler_init();

    g_sensors[0] = ir_sensor_gpio_create(&g_ir_context, 15, false);
    g_sensors[1] = ir_sensor_gpio_create(&g_ir_context2, 14, false);
    g_sensors[2] = hall_sensor_gpio_create(&g_hall_context, 13, false);
    g_sensors[3] = ultrasonic_sensor_gpio_create(&g_ultrasonic_context, 2, 4, 30.0f);

    ObstacleSensor composite_sensor =
        composite_sensor_create(&g_composite_context, g_sensors, SENSOR_COUNT);

    StatusIndicator indicator =
        led_indicator_gpio_create(&g_led_context, 18, 17, true);

    g_button = button_gpio_create(&g_button_context, 21, false);

    g_controller.sensor = composite_sensor;
    g_controller.indicator = indicator;
    g_controller.has_last_status = false;
    g_controller.enabled = true;

    Serial.println("System initialized. Profiling enabled.");
}

static uint32_t g_report_counter = 0;
#define PROFILER_REPORT_INTERVAL 200

void loop()
{
    profiler_start(PROFILER_LOOP);

    profiler_start(PROFILER_BUTTON_CHECK);
    if (g_button.is_pressed != NULL && g_button.is_pressed(g_button.context))
    {
        g_controller.enabled = !g_controller.enabled;
    }
    profiler_stop(PROFILER_BUTTON_CHECK);

    obstacle_controller_update(&g_controller);

    profiler_stop(PROFILER_LOOP);

    g_report_counter++;
    if (g_report_counter >= PROFILER_REPORT_INTERVAL)
    {
        profiler_print_report();
        profiler_reset_all();
        g_report_counter = 0;
    }

    delay(50);
}
