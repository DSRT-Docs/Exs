/*
 * Copyright [2024] [DSRT-Docs]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <stdio.h>
#include <time.h>
#include "../../include/exs_platform.h"

#define ITERATIONS 100000

int main() {
    printf("=== Exs Platform Performance Test ===\n\n");
    
    clock_t start, end;
    double elapsed;
    
    // Test 1: get_tick_count performance
    printf("1. exs_platform_get_tick_count():\n");
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        exs_platform_get_tick_count();
    }
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   %d iterations: %.6f sec (%.2f ns/call)\n", 
           ITERATIONS, elapsed, (elapsed * 1e9) / ITERATIONS);
    
    // Test 2: get_high_res_timer performance
    printf("\n2. exs_platform_get_high_res_timer():\n");
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        exs_platform_get_high_res_timer();
    }
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   %d iterations: %.6f sec (%.2f ns/call)\n",
           ITERATIONS, elapsed, (elapsed * 1e9) / ITERATIONS);
    
    // Test 3: get_cpu_count performance (cached)
    printf("\n3. exs_platform_get_cpu_count() (cached):\n");
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        exs_platform_get_cpu_count();
    }
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   %d iterations: %.6f sec (%.2f ns/call)\n",
           ITERATIONS, elapsed, (elapsed * 1e9) / ITERATIONS);
    
    // Test 4: Memory allocation performance
    printf("\n4. exs_platform_aligned_alloc/free():\n");
    const int alloc_count = 1000;
    start = clock();
    
    for (int i = 0; i < alloc_count; i++) {
        void* ptr = exs_platform_aligned_alloc(64, 16);
        if (ptr) {
            exs_platform_aligned_free(ptr);
        }
    }
    
    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   %d alloc/free pairs: %.6f sec (%.2f μs/pair)\n",
           alloc_count, elapsed, (elapsed * 1e6) / alloc_count);
    
    // Test 5: Sleep accuracy
    printf("\n5. exs_platform_sleep_ms() accuracy:\n");
    const int sleep_tests = 5;
    uint64_t total_error = 0;
    
    for (int i = 0; i < sleep_tests; i++) {
        uint64_t target = 50 * (i + 1); // 50, 100, 150, 200, 250 ms
        uint64_t start_time = exs_platform_get_tick_count();
        
        exs_platform_sleep_ms(target);
        
        uint64_t end_time = exs_platform_get_tick_count();
        uint64_t actual = end_time - start_time;
        int64_t error = (int64_t)actual - (int64_t)target;
        
        total_error += (error > 0 ? error : -error);
        
        printf("   Target: %3llu ms, Actual: %3llu ms, Error: %+lld ms\n",
               (unsigned long long)target,
               (unsigned long long)actual,
               (long long)error);
    }
    
    printf("   Average error: %.1f ms\n", (double)total_error / sleep_tests);
    
    printf("\n=== Performance Tests Completed ===\n");
    return 0;
}
