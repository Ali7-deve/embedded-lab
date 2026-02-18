#include <Arduino.h>
#include "utils/benchmark.h"
#include "utils/profiler.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static BenchmarkMode g_benchmark_mode = BENCHMARK_MODE_SOLID;
static ProfilerStats g_solid_stats[PROFILER_COUNT];
static ProfilerStats g_monolithic_stats[PROFILER_COUNT];
static bool g_solid_collected = false;
static bool g_monolithic_collected = false;

void benchmark_set_mode(BenchmarkMode mode) {
    g_benchmark_mode = mode;
    g_solid_collected = false;
    g_monolithic_collected = false;
}

BenchmarkMode benchmark_get_mode(void) {
    return g_benchmark_mode;
}

void benchmark_save_solid_stats(void) {
    for (int i = 0; i < PROFILER_COUNT; i++) {
        ProfilerStats* src = profiler_get_stats((ProfilerId)i);
        if (src != NULL) {
            memcpy(&g_solid_stats[i], src, sizeof(ProfilerStats));
        }
    }
    g_solid_collected = true;
}

void benchmark_save_monolithic_stats(void) {
    for (int i = 0; i < PROFILER_COUNT; i++) {
        ProfilerStats* src = profiler_get_stats((ProfilerId)i);
        if (src != NULL) {
            memcpy(&g_monolithic_stats[i], src, sizeof(ProfilerStats));
        }
    }
    g_monolithic_collected = true;
}

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

void benchmark_print_comparison(void) {
    if (!g_solid_collected || !g_monolithic_collected) {
        Serial.println("Error: Both SOLID and Monolithic stats must be collected first");
        return;
    }
    
    Serial.println("\n=== SOLID vs Monolithic Performance Comparison ===");
    Serial.println("Component            | SOLID Avg(us) | Mono Avg(us) | Overhead(us) | Overhead(%)");
    Serial.println("---------------------|---------------|--------------|---------------|------------");
    
    for (int i = 0; i < PROFILER_COUNT; i++) {
        ProfilerStats* solid = &g_solid_stats[i];
        ProfilerStats* mono = &g_monolithic_stats[i];
        
        if (solid->count == 0 && mono->count == 0) continue;
        
        uint32_t solid_avg = solid->count > 0 ? solid->total_us / solid->count : 0;
        uint32_t mono_avg = mono->count > 0 ? mono->total_us / mono->count : 0;
        
        int32_t overhead_us = (int32_t)solid_avg - (int32_t)mono_avg;
        float overhead_pct = mono_avg > 0 ? ((float)overhead_us / mono_avg) * 100.0f : 0.0f;
        
        char name[21];
        snprintf(name, sizeof(name), "%-20s", profiler_get_name((ProfilerId)i));
        
        Serial.printf("%s | %13lu | %12lu | %13ld | %11.2f%%\n",
            name,
            (unsigned long)solid_avg,
            (unsigned long)mono_avg,
            (long)overhead_us,
            overhead_pct);
    }
    
    ProfilerStats* solid_loop = &g_solid_stats[PROFILER_LOOP];
    ProfilerStats* mono_loop = &g_monolithic_stats[PROFILER_LOOP];
    
    if (solid_loop->count > 0 && mono_loop->count > 0) {
        uint32_t solid_avg = solid_loop->total_us / solid_loop->count;
        uint32_t mono_avg = mono_loop->total_us / mono_loop->count;
        float solid_freq = 1000000.0f / solid_avg;
        float mono_freq = 1000000.0f / mono_avg;
        
        Serial.printf("\nLoop Frequency: SOLID=%.2f Hz, Monolithic=%.2f Hz\n", solid_freq, mono_freq);
        Serial.printf("SOLID Overhead: %.2f%% slower\n", ((solid_avg - mono_avg) / (float)mono_avg) * 100.0f);
    }
    
    Serial.println("==================================================\n");
}
