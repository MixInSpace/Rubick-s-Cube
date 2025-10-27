#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>
#include <stdbool.h>

int run_benchmark(int runs, int scramble_len, const char* csv_path, unsigned int seed, bool quiet);

#endif /* BENCHMARK_H */
