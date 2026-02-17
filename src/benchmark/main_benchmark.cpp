#include <Arduino.h>

#include "app/obstacle_controller.h"
#include "drivers/ir_sensor_gpio.h"
#include "drivers/hall_sensor_gpio.h"
#include "drivers/ultrasonic_sensor_gpio.h"
#include "drivers/composite_sensor.h"
#include "drivers/led_indicator_gpio.h"
#include "drivers/button_gpio.h"
#include "benchmark/monolithic.h"
#include "utils/profiler.h"
#include "utils/benchmark.h"

#define BENCHMARK_MODE BENCHMARK_MODE_COMPARE

static IrSensorContext g_ir_context;
static IrSensorContext g_ir_context2;
static HallSensorContext g_hall_context;
static UltrasonicSensorContext g_ultrasonic_context;
static CompositeSensorContext g_composite_context;
static LedIndicatorContext g_led_context;
static ButtonContext g_button_context;
static Button g_button;
static ObstacleController g_controller;
static MonolithicSystem g_monolithic;

#define SENSOR_COUNT 4
static ObstacleSensor g_sensors[SENSOR_COUNT];

static uint32_t g_report_counter = 0;
static uint32_t g_phase_counter = 0;
#define PROFILER_REPORT_INTERVAL 200
#define PHASE_SWITCH_INTERVAL 400

void setup_solid(void) {
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
}

void setup_monolithic(void) {
    g_monolithic.ir_pin_1 = 15;
    g_monolithic.ir_pin_2 = 14;
    g_monolithic.hall_pin = 13;
    g_monolithic.ultrasonic_echo_pin = 2;
    g_monolithic.ultrasonic_trig_pin = 4;
    g_monolithic.ultrasonic_threshold = 30.0f;
    g_monolithic.button_pin = 21;
    g_monolithic.led_green_pin = 18;
    g_monolithic.led_red_pin = 17;
    monolithic_init(&g_monolithic);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    profiler_init();
    benchmark_set_mode(BENCHMARK_MODE);
    
    setup_solid();
    setup_monolithic();
    
    Serial.println("=== SOLID vs Monolithic Benchmark ===");
    Serial.println("Running comparison mode: alternating between implementations");
    Serial.println("Each phase runs for 400 iterations (~20 seconds)\n");
}

void loop_solid(void) {
    profiler_start(PROFILER_LOOP);
    
    profiler_start(PROFILER_BUTTON_CHECK);
    if (g_button.is_pressed != NULL && g_button.is_pressed(g_button.context)) {
        g_controller.enabled = !g_controller.enabled;
    }
    profiler_stop(PROFILER_BUTTON_CHECK);
    
    obstacle_controller_update(&g_controller);
    
    profiler_stop(PROFILER_LOOP);
}

void loop_monolithic(void) {
    monolithic_update(&g_monolithic);
}

void loop()
{
    BenchmarkMode mode = benchmark_get_mode();
    
    if (mode == BENCHMARK_MODE_SOLID) {
        loop_solid();
    } else if (mode == BENCHMARK_MODE_MONOLITHIC) {
        loop_monolithic();
    } else {
        if (g_phase_counter < PHASE_SWITCH_INTERVAL) {
            loop_solid();
        } else if (g_phase_counter < PHASE_SWITCH_INTERVAL * 2) {
            loop_monolithic();
        } else {
            if (g_phase_counter == PHASE_SWITCH_INTERVAL * 2) {
                benchmark_save_solid_stats();
                profiler_reset_all();
                Serial.println("\n[Phase 1 Complete] SOLID implementation stats saved. Switching to Monolithic...\n");
            }
            loop_monolithic();
        }
    }
    
    g_phase_counter++;
    g_report_counter++;
    
    if (g_report_counter >= PROFILER_REPORT_INTERVAL) {
        if (mode == BENCHMARK_MODE_COMPARE) {
            if (g_phase_counter < PHASE_SWITCH_INTERVAL) {
                Serial.println("[SOLID Phase]");
            } else if (g_phase_counter < PHASE_SWITCH_INTERVAL * 2) {
                Serial.println("[Monolithic Phase]");
            } else {
                Serial.println("[Monolithic Phase - Collecting]");
            }
        }
        profiler_print_report();
        profiler_reset_all();
        g_report_counter = 0;
    }
    
    if (mode == BENCHMARK_MODE_COMPARE && g_phase_counter >= PHASE_SWITCH_INTERVAL * 3) {
        benchmark_save_monolithic_stats();
        benchmark_print_comparison();
        g_phase_counter = 0;
        profiler_reset_all();
        Serial.println("\n[Restarting Benchmark Cycle]\n");
    }
    
    delay(50);
}
