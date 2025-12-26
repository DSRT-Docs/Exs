#include <stdio.h>
#include <string.h>
#include "../include/exs_platform.h"

void print_system_info() {
    printf("========================================\n");
    printf("        SYSTEM INFORMATION REPORT\n");
    printf("========================================\n\n");
    
    // Platform Info
    printf("[PLATFORM]\n");
    printf("  Name:           %s\n", exs_platform_get_name());
    printf("  Architecture:   %s\n", exs_platform_get_architecture());
    printf("  Compiler:       %s\n", exs_platform_get_compiler());
    printf("  64-bit:         %s\n", exs_platform_is_64bit() ? "Yes" : "No");
    printf("  Debugger:       %s\n", exs_platform_is_debugger_present() ? "Yes" : "No");
    printf("  Cache Line:     %u bytes\n", exs_platform_get_cache_line_size());
    
    // CPU Info
    printf("\n[CPU]\n");
    printf("  Cores:          %u\n", exs_platform_get_cpu_count());
    printf("  Features:\n");
    printf("    SSE:          %s\n", exs_platform_has_sse() ? "Yes" : "No");
    printf("    AVX:          %s\n", exs_platform_has_avx() ? "Yes" : "No");
    printf("    NEON:         %s\n", exs_platform_has_neon() ? "Yes" : "No");
    
    // Memory Info
    printf("\n[MEMORY]\n");
    uint64_t total = exs_platform_get_total_memory();
    uint64_t avail = exs_platform_get_available_memory();
    uint64_t used = total - avail;
    
    printf("  Total:          %.2f GB\n", total / (1024.0 * 1024 * 1024));
    printf("  Available:      %.2f GB\n", avail / (1024.0 * 1024 * 1024));
    printf("  Used:           %.2f GB (%.1f%%)\n", 
           used / (1024.0 * 1024 * 1024),
           (double)used / total * 100.0);
    
    // Timing Info
    printf("\n[TIMING]\n");
    uint64_t ticks = exs_platform_get_tick_count();
    uint64_t hr_ticks = exs_platform_get_high_res_timer();
    double freq = exs_platform_get_timer_frequency();
    
    printf("  System Time:    %llu ms\n", (unsigned long long)ticks);
    printf("  HR Timer:       %llu ticks\n", (unsigned long long)hr_ticks);
    printf("  Timer Freq:     %.0f Hz\n", freq);
    printf("  Resolution:     %.2f ns\n", 1e9 / freq);
    
    // Performance Test
    printf("\n[PERFORMANCE TEST]\n");
    printf("  Testing sleep accuracy...\n");
    
    const int sleep_tests = 3;
    const uint64_t sleep_durations[] = {50, 100, 200};
    
    for (int i = 0; i < sleep_tests; i++) {
        uint64_t start = exs_platform_get_high_res_timer();
        exs_platform_sleep_ms(sleep_durations[i]);
        uint64_t end = exs_platform_get_high_res_timer();
        
        double actual_ms = (end - start) / (freq / 1000.0);
        double error = actual_ms - sleep_durations[i];
        
        printf("    Sleep %llu ms: %.1f ms (error: %.1f ms)\n",
               (unsigned long long)sleep_durations[i], actual_ms, error);
    }
    
    // Memory Allocation Test
    printf("\n[MEMORY TEST]\n");
    printf("  Testing aligned allocation...\n");
    
    struct {
        size_t size;
        size_t alignment;
    } tests[] = {
        {1024, 64},
        {4096, 4096},
        {256, 32}
    };
    
    for (int i = 0; i < 3; i++) {
        void* ptr = exs_platform_aligned_alloc(tests[i].size, tests[i].alignment);
        if (ptr) {
            uintptr_t addr = (uintptr_t)ptr;
            bool aligned = (addr & (tests[i].alignment - 1)) == 0;
            
            printf("    Alloc %zu bytes @ %zu: %s\n",
                   tests[i].size, tests[i].alignment,
                   aligned ? "ALIGNED" : "MISALIGNED");
            
            // Use memory
            memset(ptr, 0xCC, tests[i].size);
            
            exs_platform_aligned_free(ptr);
            printf("      Freed successfully\n");
        } else {
            printf("    Allocation failed!\n");
        }
    }
    
    // Error Status
    printf("\n[STATUS]\n");
    const char* error = exs_platform_get_last_error();
    if (strcmp(error, "No error") != 0) {
        printf("  Last Error: %s\n", error);
    } else {
        printf("  No errors detected\n");
    }
    
    printf("\n========================================\n");
    printf("Report generated using Exs Platform Library\n");
    printf("========================================\n");
}

void demonstrate_features() {
    printf("\n\n========================================\n");
    printf("        FEATURE DEMONSTRATION\n");
    printf("========================================\n\n");
    
    printf("1. Platform Detection:\n");
    printf("   You're running on: %s (%s)\n", 
           exs_platform_get_name(), exs_platform_get_architecture());
    
    printf("\n2. CPU Information:\n");
    printf("   This system has %u CPU core(s)\n", exs_platform_get_cpu_count());
    
    printf("\n3. Memory Usage Monitoring:\n");
    uint64_t total = exs_platform_get_total_memory();
    uint64_t avail = exs_platform_get_available_memory();
    double usage = (double)(total - avail) / total * 100.0;
    printf("   Memory usage: %.1f%% (%.2f GB used of %.2f GB)\n",
           usage, (total - avail) / (1024.0*1024*1024), total / (1024.0*1024*1024));
    
    printf("\n4. High Precision Timing:\n");
    printf("   Starting timer...\n");
    uint64_t start = exs_platform_get_high_res_timer();
    exs_platform_sleep_ms(150);
    uint64_t end = exs_platform_get_high_res_timer();
    double elapsed_ms = (end - start) / (exs_platform_get_timer_frequency() / 1000.0);
    printf("   Elapsed time: %.3f ms\n", elapsed_ms);
    
    printf("\n5. Cache-Aware Allocation:\n");
    void* aligned_mem = exs_platform_aligned_alloc(1024, exs_platform_get_cache_line_size());
    if (aligned_mem) {
        printf("   Allocated 1KB aligned to cache line (%u bytes)\n", 
               exs_platform_get_cache_line_size());
        exs_platform_aligned_free(aligned_mem);
        printf("   Memory freed\n");
    }
    
    printf("\n6. Debugger Detection:\n");
    if (exs_platform_is_debugger_present()) {
        printf("   Debugger is attached (running in debug mode)\n");
    } else {
        printf("   No debugger detected (normal execution)\n");
    }
    
    printf("\n========================================\n");
}

int main() {
    printf("Exs Platform Library - Example Application\n");
    printf("Version 1.0\n\n");
    
    print_system_info();
    demonstrate_features();
    
    // Wait for user input on Windows, exit immediately on others
    #ifdef _WIN32
    printf("\nPress Enter to exit...");
    getchar();
    #endif
    
    return 0;
}
