#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int ir_pin_1;
    int ir_pin_2;
    int hall_pin;
    int ultrasonic_echo_pin;
    int ultrasonic_trig_pin;
    float ultrasonic_threshold;
    int button_pin;
    int led_green_pin;
    int led_red_pin;
    bool enabled;
    bool last_status;
    bool has_last_status;
    
    uint32_t ultrasonic_state;
    uint32_t ultrasonic_state_start;
    uint32_t ultrasonic_last_measurement;
    bool ultrasonic_cached_result;
    
    bool button_last_state;
    uint32_t button_last_debounce_time;
    bool button_last_raw_state;
} MonolithicSystem;

void monolithic_init(MonolithicSystem* sys);
void monolithic_update(MonolithicSystem* sys);

#ifdef __cplusplus
}
#endif
