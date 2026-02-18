#include <Arduino.h>
#include "utils/profiler.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#define PROFILER_ENABLED_DEFAULT true

static ProfilerStats g_stats[PROFILER_COUNT];
static uint32_t g_start_times[PROFILER_COUNT];
static bool g_profiler_enabled = PROFILER_ENABLED_DEFAULT;

static const char* profiler_get_name(ProfilerId id) {
    switch(id) {
        case PROFILER_LOOP: return "Loop";
        case PROFILER_CONTROLLER_UPDATE: return "Controller Update";
        case PROFILER_BUTTON_CHECK: return "Button Check";
        case PROFILER_COMPOSITE_SENSOR: return "Composite Sensor";
        case PROFILER_IR_SENSOR_1: return "IR Sensor 1";
        case PROFILER_IR_SENSOR_2: return "IR Sensor 2";
        case PROFILER_HALL_SENSOR: return "Hall Sensor";
        case PROFILER_ULTRASONIC_SENSOR: return "Ultrasonic Sensor";
        case PROFILER_LED_INDICATOR: return "LED Indicator";
        default: return "Unknown";
    }
}

static uint32_t get_time_us(void) {
    return (uint32_t)(micros());
}

void profiler_init(void) {
    memset(g_stats, 0, sizeof(g_stats));
    memset(g_start_times, 0, sizeof(g_start_times));
    for (int i = 0; i < PROFILER_COUNT; i++) {
        g_stats[i].min_us = UINT32_MAX;
    }
}

void profiler_start(ProfilerId id) {
    if (!g_profiler_enabled || id >= PROFILER_COUNT) return;
    g_start_times[id] = get_time_us();
}

void profiler_stop(ProfilerId id) {
    if (!g_profiler_enabled || id >= PROFILER_COUNT) return;
    if (g_start_times[id] == 0) return;
    
    uint32_t elapsed = get_time_us() - g_start_times[id];
    ProfilerStats* stats = &g_stats[id];
    
    stats->count++;
    stats->total_us += elapsed;
    stats->last_us = elapsed;
    
    if (elapsed < stats->min_us) {
        stats->min_us = elapsed;
    }
    if (elapsed > stats->max_us) {
        stats->max_us = elapsed;
    }
    
    g_start_times[id] = 0;
}

void profiler_reset(ProfilerId id) {
    if (id >= PROFILER_COUNT) return;
    memset(&g_stats[id], 0, sizeof(ProfilerStats));
    g_stats[id].min_us = UINT32_MAX;
    g_start_times[id] = 0;
}

void profiler_reset_all(void) {
    profiler_init();
}

ProfilerStats* profiler_get_stats(ProfilerId id) {
    if (id >= PROFILER_COUNT) return NULL;
    return &g_stats[id];
}

bool profiler_is_enabled(void) {
    return g_profiler_enabled;
}

void profiler_set_enabled(bool enabled) {
    g_profiler_enabled = enabled;
}

void profiler_print_report(void) {
    if (!g_profiler_enabled) return;
    
    Serial.println("\n=== Performance Profile Report ===");
    Serial.println("Name                 | Count    | Total(us) | Avg(us)   | Min(us)   | Max(us)   | Last(us)");
    Serial.println("---------------------|----------|-----------|-----------|-----------|-----------|----------");
    
    for (int i = 0; i < PROFILER_COUNT; i++) {
        ProfilerStats* stats = &g_stats[i];
        if (stats->count == 0) continue;
        
        uint32_t avg_us = stats->count > 0 ? stats->total_us / stats->count : 0;
        uint32_t min_us = stats->min_us == UINT32_MAX ? 0 : stats->min_us;
        
        char name[21];
        snprintf(name, sizeof(name), "%-20s", profiler_get_name((ProfilerId)i));
        
        Serial.printf("%s | %8lu | %9lu | %9lu | %9lu | %9lu | %9lu\n",
            name,
            (unsigned long)stats->count,
            (unsigned long)stats->total_us,
            (unsigned long)avg_us,
            (unsigned long)min_us,
            (unsigned long)stats->max_us,
            (unsigned long)stats->last_us);
    }
    
    ProfilerStats* loop_stats = profiler_get_stats(PROFILER_LOOP);
    if (loop_stats && loop_stats->count > 0) {
        uint32_t avg_loop_us = loop_stats->total_us / loop_stats->count;
        float loop_freq_hz = 1000000.0f / avg_loop_us;
        Serial.printf("\nLoop Frequency: %.2f Hz (Target: 20 Hz)\n", loop_freq_hz);
        Serial.printf("Loop Efficiency: %.1f%% (50ms target)\n", 
            (50000.0f / avg_loop_us) * 100.0f);
    }
    
    Serial.println("===================================\n");
}
