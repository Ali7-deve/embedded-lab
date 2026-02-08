// drivers/led_indicator_gpio.h
#pragma once

#include "interfaces/status_indicator.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * LedIndicatorContext
 *
 * Why this exists:
 *   The context is caller-owned so you can create multiple indicators
 *   without hidden global state. The context must outlive the
 *   StatusIndicator that points to it (e.g., static or global storage).
 */
typedef struct {
    int green_gpio;
    int red_gpio;
    bool active_level;
} LedIndicatorContext;

/*
 * Create a GPIO-backed status indicator using two LEDs.
 *
 * active_level:
 *   - true  => GPIO HIGH turns an LED on
 *   - false => GPIO LOW  turns an LED on
 */
StatusIndicator led_indicator_gpio_create(
    LedIndicatorContext *context,
    int green_gpio,
    int red_gpio,
    bool active_level
);

#ifdef __cplusplus
} // extern "C"
#endif
