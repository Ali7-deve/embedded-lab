// drivers/button_gpio.h
#pragma once

#include "interfaces/button.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * ButtonContext
     *
     * Why this exists:
     *   The context is caller-owned so you can create multiple buttons without
     *   hidden global state. The context must outlive the Button that points
     *   to it (e.g., static or global storage).
     */
    typedef struct
    {
        int gpio_pin;
        bool active_level;           // true = HIGH when pressed, false = LOW when pressed
        bool last_state;             // Last debounced state
        uint32_t last_debounce_time; // Last time the state changed (for debouncing)
        bool last_raw_state;         // Last raw GPIO reading
    } ButtonContext;

    /*
     * Create a GPIO-backed push button.
     *
     * The button driver includes debouncing to prevent false triggers from
     * electrical noise or mechanical switch bounce.
     *
     * gpio_pin: GPIO pin connected to the button
     * active_level:
     *   - true  => GPIO HIGH means "button pressed"
     *   - false => GPIO LOW  means "button pressed"
     *
     * Note: Most push buttons are active LOW (pressed = LOW, released = HIGH with pull-up).
     */
    Button button_gpio_create(
        ButtonContext *context,
        int gpio_pin,
        bool active_level);

#ifdef __cplusplus
} // extern "C"
#endif
