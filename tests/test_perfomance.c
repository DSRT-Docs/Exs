#include <stdio.h>
#include <time.h>
#include "../include/exs_platform.h"

#define ITERATIONS 1000000

void benchmark_timer_functions() {
    printf("=== Timer Function Performance Benchmark ===\n");
    
    // Benchmark get_tick_count()
    clock_t start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        exs_platform_get_tick_count();
    }
    clock_t end = clock();
    double tick_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("get_tick_count(): %.6f sec for %d iterations (%.2f ns/call)\n",
           tick_time, ITERATIONS, (tick_time * 1e9) / ITERATIONS);
    
    // Benchmark get_high_res_timer()
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        exs_platform_get_high_res_timer();
    }
    end = clock();
    double hrtime = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("get_high_res_timer(): %.6f sec for %d iterations (%.2f ns/call)\n",
           hrtime, ITERATIONS, (hrtime * 1e9) / ITERATIONS);
    
    // Benchmark sleep accuracy
    printf("\n=== Sleep Accuracy Test ===\n");
    uint64_t total_error = 0;
    const int sleep_tests = 10;
    
    for (int i = 0; i < sleep_tests; i++) {
        uint64_t start_ticks = exs_platform_get_high_res_timer();
        double freq = exs_platform_get_timer_frequency();
        
        uint64_t sleep_ms = 10 * (i + 1); // 10, 20, 30, ... 100 ms
        exs_platform_sleep_ms(sleep_ms);
        
        uint64_t end_ticks = exs_platform_get_high_res_timer();
        double actual_ms = (end_ticks - start_ticks) / (freq / 1000.0);
        
        double error = actual_ms - sleep_ms;
        total_error += (error > 0 ? error : -error);
        
        printf("Sleep %llu ms: actual %.2f ms (error: %.2f ms)\n",
               (unsigned long long)sleep_ms, actual_ms, error);
    }
    
    printf("Average sleep error: %.2f ms\n", (double)total_error / sleep_tests);
}

void benchmark_memory_allocation() {
    printf("\n=== Memory Allocation Performance ===\n");
    
    const size_t sizes[] = {64, 256, 1024, 4096, 16384};
    const size_t alignments[] = {16, 64, 256, 4096};
    const int alloc_count = 1000;
    
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        for (size_t j = 0; j < sizeof(alignments)/sizeof(alignments[0]); j++) {
            if (alignments[j] <= sizes[i]) {
                clock_t start = clock();
                
                void* pointers[alloc_count];
                for (int k = 0; k < alloc_count; k++) {
                    pointers[k] = exs_platform_aligned_alloc(sizes[i], alignments[j]);
                }
                
                for (int k = 0; k < alloc_count; k++) {
                    if (pointers[k]) {
                        exs_platform_aligned_free(pointers[k]);
                    }
                }
                
                clock_t end = clock();
                double time_sec = ((double)(end - start)) / CLOCKS_PER_SEC;
                double time_per_alloc = (time_sec * 1e6) / (alloc_count * 2); // μs per alloc+free
                
                printf("Size: %5zu, Align: %4zu - Time: %.3f μs per operation\n",
                       sizes[i], alignments[j], time_per_alloc);
            }
        }
    }
}

void benchmark_cpu_info_functions() {
    printf("\n=== CPU Info Functions Performance ===\n");
    
    const int iterations = 10000;
    
    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        exs_platform_get_cpu_count();
    }
    clock_t end = clock();
    printf("get_cpu_count(): %.3f μs per call\n",
           ((double)(end - start)) / CLOCKS_PER_SEC / iterations * 1e6);
    
    start = clock();
    for (int i = 0; i < iterations; i++) {
        exs_platform_has_sse();
    }
    end = clock();
    printf("has_sse(): %.3f μs per call\n",
           ((double)(end - start)) / CLOCKS_PER_SEC / iterations * 1e6);
    
    start = clock();
    for (int i = 0; i < iterations; i++) {
        exs_platform_get_cache_line_size();
    }
    end = clock();
    printf("get_cache_line_size(): %.3f μs per call\n",
           ((double)(end - start)) / CLOCKS_PER_SEC / iterations * 1e6);
}

int main() {
    printf("================================\n");
    printf("Exs Platform Library Performance Tests\n");
    printf("================================\n\n");
    
    benchmark_timer_functions();
    benchmark_memory_allocation();
    benchmark_cpu_info_functions();
    
    printf("\n================================\n");
    printf("Performance Tests Complete\n");
    printf("================================\n");
    
    return 0;
}
