#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t count;
    uint32_t total_us;
    uint32_t min_us;
    uint32_t max_us;
    uint32_t last_us;
} ProfilerStats;

typedef enum {
    PROFILER_LOOP,
    PROFILER_CONTROLLER_UPDATE,
    PROFILER_BUTTON_CHECK,
    PROFILER_COMPOSITE_SENSOR,
    PROFILER_IR_SENSOR_1,
    PROFILER_IR_SENSOR_2,
    PROFILER_HALL_SENSOR,
    PROFILER_ULTRASONIC_SENSOR,
    PROFILER_LED_INDICATOR,
    PROFILER_COUNT
} ProfilerId;

void profiler_init(void);
void profiler_start(ProfilerId id);
void profiler_stop(ProfilerId id);
void profiler_reset(ProfilerId id);
void profiler_reset_all(void);
ProfilerStats* profiler_get_stats(ProfilerId id);
void profiler_print_report(void);
bool profiler_is_enabled(void);
void profiler_set_enabled(bool enabled);

#ifdef __cplusplus
}
#endif
