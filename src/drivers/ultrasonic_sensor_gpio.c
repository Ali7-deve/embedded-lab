#include "drivers/ultrasonic_sensor_gpio.h"
// ESP-IDF GPIO is used because this module is C-only and Arduino GPIO is C++.
#include "driver/gpio.h"
#include <stddef.h>

// Microsecond delay function using FreeRTOS tick
// Note: This is approximate but sufficient for ultrasonic sensor timing
static void delay_microseconds(uint32_t us)
{
    // For very short delays, use a simple loop
    // For ESP32 at 240MHz, approximate calibration
    volatile uint32_t count = us * 80; // Approximate cycles per microsecond
    while (count--)
    {
        __asm__ __volatile__("nop");
    }
}

// Read GPIO level with proper casting
static int read_gpio_level(int pin)
{
    return gpio_get_level((gpio_num_t)pin);
}

// Set GPIO level with proper casting
static void set_gpio_level(int pin, int level)
{
    gpio_set_level((gpio_num_t)pin, level);
}

static bool ultrasonic_is_detected(void *context)
{
    UltrasonicSensorContext *ctx = (UltrasonicSensorContext *)context;

    if (ctx == NULL)
    {
        // Fail safe: no context means we can't read the sensor.
        return false;
    }

    // Send trigger pulse: HIGH for 10 microseconds
    set_gpio_level(ctx->trig_pin, 0);
    delay_microseconds(2);
    set_gpio_level(ctx->trig_pin, 1);
    delay_microseconds(10);
    set_gpio_level(ctx->trig_pin, 0);

    // Wait for echo to start (timeout after 30ms)
    uint32_t timeout = 30000;
    uint32_t duration = 0;

    // Wait for echo pin to go HIGH
    while (read_gpio_level(ctx->echo_pin) == 0)
    {
        if (--timeout == 0)
        {
            return false; // Timeout - no echo received
        }
        delay_microseconds(1);
    }

    // Measure how long echo pin stays HIGH by counting microseconds
    // Each iteration of the delay loop approximates 1 microsecond
    timeout = 30000; // Max 30ms echo (about 5 meters)
    duration = 0;

    while (read_gpio_level(ctx->echo_pin) == 1)
    {
        if (--timeout == 0)
        {
            return false; // Timeout - echo too long
        }
        delay_microseconds(1);
        duration++;
    }

    // Calculate distance: time in microseconds / 58 = distance in cm
    // Speed of sound is approximately 343 m/s = 0.0343 cm/us
    // Round trip: distance = (time_us * 0.0343) / 2 = time_us / 58.2
    // Using 58 for simplicity (close enough for obstacle detection)
    float distance_cm = duration / 58.0f;

    // Return true if obstacle detected within threshold
    return (distance_cm > 0 && distance_cm <= ctx->detection_threshold_cm);
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
