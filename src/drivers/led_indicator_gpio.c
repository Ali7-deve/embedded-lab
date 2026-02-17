#include "drivers/led_indicator_gpio.h"
#include "driver/gpio.h"
#include "utils/profiler.h"
#include <stddef.h>

static void led_set_status(void *context, SystemStatus status)
{
    profiler_start(PROFILER_LED_INDICATOR);
    LedIndicatorContext *ctx = (LedIndicatorContext *)context;

    if (ctx == NULL) {
        // Fail safe: no context means we can't drive the pins.
        return;
    }

    bool obstacle = (status == SYSTEM_STATUS_OBSTACLE);
    int active = ctx->active_level ? 1 : 0;
    int inactive = ctx->active_level ? 0 : 1;

    // Map system state to LEDs: green = clear, red = obstacle.
    int green_level = obstacle ? inactive : active;
    int red_level = obstacle ? active : inactive;

    gpio_set_level(ctx->green_gpio, green_level);
    gpio_set_level(ctx->red_gpio, red_level);
    
    profiler_stop(PROFILER_LED_INDICATOR);
}

StatusIndicator led_indicator_gpio_create(
    LedIndicatorContext *context,
    int green_gpio,
    int red_gpio,
    bool active_level
)
{
    StatusIndicator indicator = {
        .set_status = NULL,
        .context = NULL
    };

    if (context == NULL) {
        // Without caller-owned storage, multiple indicators would collide.
        return indicator;
    }

    context->green_gpio = green_gpio;
    context->red_gpio = red_gpio;
    context->active_level = active_level;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << green_gpio) | (1ULL << red_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);

    indicator.set_status = led_set_status;
    indicator.context = context;

    // Initialize to a known state so the user sees "clear" on boot.
    led_set_status(context, SYSTEM_STATUS_CLEAR);

    return indicator;
}
