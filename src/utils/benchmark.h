#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BENCHMARK_MODE_SOLID,
    BENCHMARK_MODE_MONOLITHIC,
    BENCHMARK_MODE_COMPARE
} BenchmarkMode;

void benchmark_set_mode(BenchmarkMode mode);
BenchmarkMode benchmark_get_mode(void);
void benchmark_save_solid_stats(void);
void benchmark_save_monolithic_stats(void);
void benchmark_print_comparison(void);

#ifdef __cplusplus
}
#endif
