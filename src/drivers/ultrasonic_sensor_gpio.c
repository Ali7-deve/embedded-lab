#include "drivers/ultrasonic_sensor_gpio.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/profiler.h"
#include <stddef.h>
#include <stdint.h>

#define ULTRASONIC_MEASUREMENT_INTERVAL_MS 100
#define ULTRASONIC_TRIGGER_PULSE_US 10
#define ULTRASONIC_MAX_ECHO_TIME_MS 30
#define ULTRASONIC_DISTANCE_DIVISOR 58.0f

static void delay_microseconds(uint32_t us)
{
    volatile uint32_t count = us * 80;
    while (count--)
    {
        __asm__ __volatile__("nop");
    }
}

static int read_gpio_level(int pin)
{
    return gpio_get_level((gpio_num_t)pin);
}

static void set_gpio_level(int pin, int level)
{
    gpio_set_level((gpio_num_t)pin, level);
}

static uint32_t get_time_ms(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

static bool ultrasonic_is_detected(void *context)
{
    profiler_start(PROFILER_ULTRASONIC_SENSOR);

    UltrasonicSensorContext *ctx = (UltrasonicSensorContext *)context;
    if (ctx == NULL)
    {
        profiler_stop(PROFILER_ULTRASONIC_SENSOR);
        return false;
    }

    uint32_t now = get_time_ms();

    if (ctx->state == ULTRASONIC_READY)
    {
        if ((now - ctx->last_measurement_time) >= ctx->measurement_interval_ms)
        {
            ctx->state = ULTRASONIC_TRIGGERING;
            ctx->state_start_time = now;
            set_gpio_level(ctx->trig_pin, 0);
            delay_microseconds(2);
            set_gpio_level(ctx->trig_pin, 1);
            delay_microseconds(ULTRASONIC_TRIGGER_PULSE_US);
            set_gpio_level(ctx->trig_pin, 0);
            ctx->state = ULTRASONIC_WAITING_ECHO_START;
            ctx->state_start_time = now;
        }
        return ctx->cached_result;
    }

    if (ctx->state == ULTRASONIC_WAITING_ECHO_START)
    {
        if (read_gpio_level(ctx->echo_pin) == 1)
        {
            ctx->state = ULTRASONIC_MEASURING_ECHO;
            ctx->state_start_time = now;
        }
        else if ((now - ctx->state_start_time) > ULTRASONIC_MAX_ECHO_TIME_MS)
        {
            ctx->cached_result = false;
            ctx->state = ULTRASONIC_READY;
            ctx->last_measurement_time = now;
        }
        return ctx->cached_result;
    }

    if (ctx->state == ULTRASONIC_MEASURING_ECHO)
    {
        if (read_gpio_level(ctx->echo_pin) == 0)
        {
            uint32_t duration_ms = now - ctx->state_start_time;
            float distance_cm = (duration_ms * 1000.0f) / ULTRASONIC_DISTANCE_DIVISOR;
            ctx->cached_result = (distance_cm > 0 && distance_cm <= ctx->detection_threshold_cm);
            ctx->state = ULTRASONIC_READY;
            ctx->last_measurement_time = now;
        }
        else if ((now - ctx->state_start_time) > ULTRASONIC_MAX_ECHO_TIME_MS)
        {
            ctx->cached_result = false;
            ctx->state = ULTRASONIC_READY;
            ctx->last_measurement_time = now;
        }
        profiler_stop(PROFILER_ULTRASONIC_SENSOR);
        return ctx->cached_result;
    }

    profiler_stop(PROFILER_ULTRASONIC_SENSOR);
    return ctx->cached_result;
}

ObstacleSensor ultrasonic_sensor_gpio_create(
    UltrasonicSensorContext *context,
    int echo_pin,
    int trig_pin,
    float detection_threshold_cm)
{
    ObstacleSensor sensor = {
        .is_detected = NULL,
        .context = NULL};

    if (context == NULL)
    {
        // Without caller-owned storage, multiple sensors would collide.
        return sensor;
    }

    context->echo_pin = echo_pin;
    context->trig_pin = trig_pin;
    context->detection_threshold_cm = detection_threshold_cm;
    context->state = ULTRASONIC_READY;
    context->state_start_time = 0;
    context->last_measurement_time = 0;
    context->cached_result = false;
    context->measurement_interval_ms = ULTRASONIC_MEASUREMENT_INTERVAL_MS;

    // Configure Echo pin as INPUT
    gpio_config_t echo_conf = {
        .pin_bit_mask = (1ULL << echo_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&echo_conf);

    // Configure Trig pin as OUTPUT
    gpio_config_t trig_conf = {
        .pin_bit_mask = (1ULL << trig_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&trig_conf);

    // Initialize Trig pin to LOW
    set_gpio_level(trig_pin, 0);

    sensor.is_detected = ultrasonic_is_detected;
    sensor.context = context;

    return sensor;
}
