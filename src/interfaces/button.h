// interfaces/button.h
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * Button
     *
     * Responsibility:
     *   Detect button press events (edge-triggered, debounced).
     *
     * Contract:
     *   - Non-blocking
     *   - Debounced to avoid false triggers
     *   - Edge-triggered: returns true only when button transitions from not-pressed to pressed
     *   - Safe to call repeatedly from main loop
     */
    typedef struct
    {
        bool (*is_pressed)(void *context);
        void *context;
    } Button;

#ifdef __cplusplus
} // extern "C"
#endif
