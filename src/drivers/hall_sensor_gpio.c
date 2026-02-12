#include "drivers/hall_sensor_gpio.h"
// ESP-IDF GPIO is used because this module is C-only and Arduino GPIO is C++.
#include "driver/gpio.h"
#include <stddef.h>

static bool hall_is_detected(void *context)
{
    HallSensorContext *ctx = (HallSensorContext *)context;

    if (ctx == NULL)
    {
        // Fail safe: no context means we can't read the pin.
        return false;
    }

    int level = gpio_get_level(ctx->gpio_pin);

    return (level != 0) == ctx->active_level;
}

ObstacleSensor hall_sensor_gpio_create(
    HallSensorContext *context,
    int gpio_pin,
    bool active_level)
{
    ObstacleSensor sensor = {
        .is_detected = NULL,
        .context = NULL};

    if (context == NULL)
    {
        // Without caller-owned storage, multiple sensors would collide.
        return sensor;
    }

    context->gpio_pin = gpio_pin;
    context->active_level = active_level;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&io_conf);

    sensor.is_detected = hall_is_detected;
    sensor.context = context;

    return sensor;
}
