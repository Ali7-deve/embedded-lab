#include "drivers/button_gpio.h"
// ESP-IDF GPIO is used because this module is C-only and Arduino GPIO is C++.
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stddef.h>

// Debounce time in milliseconds (50ms is typical for mechanical switches)
#define DEBOUNCE_DELAY_MS 50

static bool button_is_pressed(void *context)
{
    ButtonContext *ctx = (ButtonContext *)context;

    if (ctx == NULL)
    {
        // Fail safe: no context means we can't read the pin.
        return false;
    }

    // Read current GPIO state
    int current_raw = gpio_get_level(ctx->gpio_pin);
    bool current_state = (current_raw != 0) == ctx->active_level;

    // Get current time in milliseconds
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Check if raw state has changed (potential button press/release)
    if (current_state != ctx->last_raw_state)
    {
        // Reset debounce timer
        ctx->last_debounce_time = current_time;
    }

    // Update last raw state
    ctx->last_raw_state = current_state;

    // If enough time has passed since last state change, update debounced state
    if ((current_time - ctx->last_debounce_time) > DEBOUNCE_DELAY_MS)
    {
        // State has been stable for debounce period
        bool previous_debounced = ctx->last_state;
        ctx->last_state = current_state;

        // Edge detection: return true only on rising edge (not pressed -> pressed)
        // This means the button was just pressed, not held down
        if (current_state && !previous_debounced)
        {
            return true; // Button was just pressed (edge detected)
        }
    }

    return false; // Button not pressed or still debouncing
}

Button button_gpio_create(
    ButtonContext *context,
    int gpio_pin,
    bool active_level)
{
    Button button = {
        .is_pressed = NULL,
        .context = NULL};

    if (context == NULL)
    {
        // Without caller-owned storage, multiple buttons would collide.
        return button;
    }

    context->gpio_pin = gpio_pin;
    context->active_level = active_level;
    context->last_state = false;
    context->last_debounce_time = 0;
    context->last_raw_state = false;

    // Configure button pin as INPUT
    // Most buttons use pull-up resistor (active LOW), but we make it configurable
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = active_level ? GPIO_PULLUP_DISABLE : GPIO_PULLUP_ENABLE,
        .pull_down_en = active_level ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&io_conf);

    // Initialize state by reading the pin
    int initial_level = gpio_get_level(gpio_pin);
    context->last_raw_state = (initial_level != 0) == active_level;
    context->last_state = context->last_raw_state;

    button.is_pressed = button_is_pressed;
    button.context = context;

    return button;
}
