#include <stdint.h>
#include <inttypes.h>
#include "benchmark/monolithic.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/profiler.h"
#include <stddef.h>

#define ULTRASONIC_MEASUREMENT_INTERVAL_MS 100
#define ULTRASONIC_MAX_ECHO_TIME_MS 30
#define ULTRASONIC_DISTANCE_DIVISOR 58.0f
#define DEBOUNCE_DELAY_MS 50

enum
{
    ULTRASONIC_READY,
    ULTRASONIC_WAITING_ECHO_START,
    ULTRASONIC_MEASURING_ECHO
};

static void delay_microseconds(uint32_t us)
{
    volatile uint32_t count = us * 80;
    while (count--)
    {
        __asm__ __volatile__("nop");
    }
}

static uint32_t get_time_ms(void)
{
    return (xTaskGetTickCount() * 1000) / configTICK_RATE_HZ;
}

static bool read_ir_sensor(int pin)
{
    return gpio_get_level(pin) == 0;
}

static bool read_hall_sensor(int pin)
{
    return gpio_get_level(pin) == 0;
}

static bool read_ultrasonic_sensor(MonolithicSystem *sys)
{
    uint32_t now = get_time_ms();

    if (sys->ultrasonic_state == ULTRASONIC_READY)
    {
        if ((now - sys->ultrasonic_last_measurement) >= ULTRASONIC_MEASUREMENT_INTERVAL_MS)
        {
            gpio_set_level(sys->ultrasonic_trig_pin, 0);
            delay_microseconds(2);
            gpio_set_level(sys->ultrasonic_trig_pin, 1);
            delay_microseconds(10);
            gpio_set_level(sys->ultrasonic_trig_pin, 0);
            sys->ultrasonic_state = ULTRASONIC_WAITING_ECHO_START;
            sys->ultrasonic_state_start = now;
        }
        return sys->ultrasonic_cached_result;
    }

    if (sys->ultrasonic_state == ULTRASONIC_WAITING_ECHO_START)
    {
        if (gpio_get_level(sys->ultrasonic_echo_pin) == 1)
        {
            sys->ultrasonic_state = ULTRASONIC_MEASURING_ECHO;
            sys->ultrasonic_state_start = now;
        }
        else if ((now - sys->ultrasonic_state_start) > ULTRASONIC_MAX_ECHO_TIME_MS)
        {
            sys->ultrasonic_cached_result = false;
            sys->ultrasonic_state = ULTRASONIC_READY;
            sys->ultrasonic_last_measurement = now;
        }
        return sys->ultrasonic_cached_result;
    }

    if (sys->ultrasonic_state == ULTRASONIC_MEASURING_ECHO)
    {
        if (gpio_get_level(sys->ultrasonic_echo_pin) == 0)
        {
            uint32_t duration_ms = now - sys->ultrasonic_state_start;
            float distance_cm = (duration_ms * 1000.0f) / ULTRASONIC_DISTANCE_DIVISOR;
            sys->ultrasonic_cached_result = (distance_cm > 0 && distance_cm <= sys->ultrasonic_threshold);
            sys->ultrasonic_state = ULTRASONIC_READY;
            sys->ultrasonic_last_measurement = now;
        }
        else if ((now - sys->ultrasonic_state_start) > ULTRASONIC_MAX_ECHO_TIME_MS)
        {
            sys->ultrasonic_cached_result = false;
            sys->ultrasonic_state = ULTRASONIC_READY;
            sys->ultrasonic_last_measurement = now;
        }
        return sys->ultrasonic_cached_result;
    }

    return sys->ultrasonic_cached_result;
}

static bool read_button(MonolithicSystem *sys)
{
    int current_raw = gpio_get_level(sys->button_pin);
    bool current_state = (current_raw == 0);
    uint32_t current_time = get_time_ms();

    if (current_state != sys->button_last_raw_state)
    {
        sys->button_last_debounce_time = current_time;
    }

    sys->button_last_raw_state = current_state;

    if ((current_time - sys->button_last_debounce_time) > DEBOUNCE_DELAY_MS)
    {
        bool previous = sys->button_last_state;
        sys->button_last_state = current_state;
        if (current_state && !previous)
        {
            return true;
        }
    }

    return false;
}

static void set_leds(MonolithicSystem *sys, bool obstacle)
{
    gpio_set_level(sys->led_green_pin, obstacle ? 0 : 1);
    gpio_set_level(sys->led_red_pin, obstacle ? 1 : 0);
}

void monolithic_init(MonolithicSystem *sys)
{
    if (sys == NULL)
        return;

    gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << sys->ir_pin_1) | (1ULL << sys->ir_pin_2) |
                        (1ULL << sys->hall_pin) | (1ULL << sys->ultrasonic_echo_pin) |
                        (1ULL << sys->button_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&input_conf);

    gpio_config_t output_conf = {
        .pin_bit_mask = (1ULL << sys->ultrasonic_trig_pin) |
                        (1ULL << sys->led_green_pin) | (1ULL << sys->led_red_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&output_conf);

    gpio_set_level(sys->ultrasonic_trig_pin, 0);
    gpio_set_level(sys->led_green_pin, 1);
    gpio_set_level(sys->led_red_pin, 0);

    sys->ultrasonic_state = ULTRASONIC_READY;
    sys->ultrasonic_last_measurement = 0;
    sys->ultrasonic_cached_result = false;
    sys->button_last_state = false;
    sys->button_last_debounce_time = 0;
    sys->button_last_raw_state = false;
    sys->enabled = true;
    sys->has_last_status = false;
}

void monolithic_update(MonolithicSystem *sys)
{
    if (sys == NULL)
        return;

    profiler_start(PROFILER_LOOP);

    profiler_start(PROFILER_BUTTON_CHECK);
    if (read_button(sys))
    {
        sys->enabled = !sys->enabled;
    }
    profiler_stop(PROFILER_BUTTON_CHECK);

    if (!sys->enabled)
    {
        profiler_stop(PROFILER_LOOP);
        return;
    }

    profiler_start(PROFILER_COMPOSITE_SENSOR);

    profiler_start(PROFILER_IR_SENSOR_1);
    bool ir1 = read_ir_sensor(sys->ir_pin_1);
    profiler_stop(PROFILER_IR_SENSOR_1);

    if (ir1)
    {
        profiler_stop(PROFILER_COMPOSITE_SENSOR);
        profiler_start(PROFILER_LED_INDICATOR);
        set_leds(sys, true);
        profiler_stop(PROFILER_LED_INDICATOR);
        profiler_stop(PROFILER_LOOP);
        return;
    }

    profiler_start(PROFILER_IR_SENSOR_2);
    bool ir2 = read_ir_sensor(sys->ir_pin_2);
    profiler_stop(PROFILER_IR_SENSOR_2);

    if (ir2)
    {
        profiler_stop(PROFILER_COMPOSITE_SENSOR);
        profiler_start(PROFILER_LED_INDICATOR);
        set_leds(sys, true);
        profiler_stop(PROFILER_LED_INDICATOR);
        profiler_stop(PROFILER_LOOP);
        return;
    }

    profiler_start(PROFILER_HALL_SENSOR);
    bool hall = read_hall_sensor(sys->hall_pin);
    profiler_stop(PROFILER_HALL_SENSOR);

    if (hall)
    {
        profiler_stop(PROFILER_COMPOSITE_SENSOR);
        profiler_start(PROFILER_LED_INDICATOR);
        set_leds(sys, true);
        profiler_stop(PROFILER_LED_INDICATOR);
        profiler_stop(PROFILER_LOOP);
        return;
    }

    profiler_start(PROFILER_ULTRASONIC_SENSOR);
    bool ultrasonic = read_ultrasonic_sensor(sys);
    profiler_stop(PROFILER_ULTRASONIC_SENSOR);

    profiler_stop(PROFILER_COMPOSITE_SENSOR);

    bool detected = ultrasonic;
    bool obstacle = detected;

    if (!sys->has_last_status || obstacle != sys->last_status)
    {
        profiler_start(PROFILER_LED_INDICATOR);
        set_leds(sys, obstacle);
        profiler_stop(PROFILER_LED_INDICATOR);
        sys->last_status = obstacle;
        sys->has_last_status = true;
    }

    profiler_stop(PROFILER_LOOP);
}
