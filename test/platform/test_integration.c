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
#include <string.h>
#include "../../include/exs_platform.h"

void print_system_report() {
    printf("\n=== Exs Platform Integration Test ===\n");
    printf("System Report\n");
    printf("===============\n\n");
    
    // Platform Info
    printf("PLATFORM INFORMATION\n");
    printf("  Name:          %s\n", exs_platform_get_name());
    printf("  Architecture:  %s\n", exs_platform_get_architecture());
    printf("  Compiler:      %s\n", exs_platform_get_compiler());
    printf("  64-bit:        %s\n", exs_platform_is_64bit() ? "Yes" : "No");
    printf("  Debugger:      %s\n", exs_platform_is_debugger_present() ? "Yes" : "No");
    
    // CPU Info
    printf("\nCPU INFORMATION\n");
    printf("  Cores:         %u\n", exs_platform_get_cpu_count());
    printf("  Cache Line:    %u bytes\n", exs_platform_get_cache_line_size());
    printf("  Features:\n");
    printf("    SSE:         %s\n", exs_platform_has_sse() ? "Yes" : "No");
    printf("    AVX:         %s\n", exs_platform_has_avx() ? "Yes" : "No");
    printf("    NEON:        %s\n", exs_platform_has_neon() ? "Yes" : "No");
    
    // Memory Info
    printf("\nMEMORY INFORMATION\n");
    uint64_t total = exs_platform_get_total_memory();
    uint64_t available = exs_platform_get_available_memory();
    uint64_t used = total - available;
    
    printf("  Total:         %llu bytes\n", (unsigned long long)total);
    printf("  Available:     %llu bytes\n", (unsigned long long)available);
    printf("  Used:          %llu bytes\n", (unsigned long long)used);
    
    if (total > 0) {
        double usage = (double)used / total * 100.0;
        printf("  Usage:         %.1f%%\n", usage);
    }
    
    // Timing Info
    printf("\nTIMING INFORMATION\n");
    uint64_t ticks = exs_platform_get_tick_count();
    double freq = exs_platform_get_timer_frequency();
    
    printf("  System Time:   %llu ms\n", (unsigned long long)ticks);
    printf("  Timer Freq:    %.0f Hz\n", freq);
    printf("  Resolution:    %.2f ns\n", 1e9 / freq);
    
    // Test allocations
    printf("\nMEMORY ALLOCATION TESTS\n");
    size_t sizes[] = {16, 64, 256, 1024, 4096};
    size_t alignments[] = {8, 16, 32, 64, 128};
    
    for (int i = 0; i < 5; i++) {
        void* ptr = exs_platform_aligned_alloc(sizes[i], alignments[i]);
        if (ptr) {
            printf("  Allocated %4zu bytes @ %3zu alignment: OK\n", 
                   sizes[i], alignments[i]);
            exs_platform_aligned_free(ptr);
        } else {
            printf("  Allocation failed: %zu bytes @ %zu alignment\n",
                   sizes[i], alignments[i]);
        }
    }
    
    // Error status
    printf("\nSYSTEM STATUS\n");
    const char* error = exs_platform_get_last_error();
    if (strcmp(error, "No error") == 0) {
        printf("  Status:        OK (No errors)\n");
    } else {
        printf("  Status:        WARNING - %s\n", error);
    }
    
    printf("\n=== Report Complete ===\n");
}

int main() {
    printf("Exs Platform Library - Integration Test\n");
    printf("=======================================\n\n");
    
    print_system_report();
    
    // Run some functional tests
    printf("\nRunning functional tests...\n");
    
    // Test 1: Repeated calls consistency
    printf("1. Consistency test... ");
    uint32_t cpu1 = exs_platform_get_cpu_count();
    uint32_t cpu2 = exs_platform_get_cpu_count();
    
    if (cpu1 == cpu2) {
        printf("✓ Consistent\n");
    } else {
        printf("✗ Inconsistent: %u vs %u\n", cpu1, cpu2);
    }
    
    // Test 2: Memory bounds
    printf("2. Memory bounds test... ");
    uint64_t total = exs_platform_get_total_memory();
    uint64_t available = exs_platform_get_available_memory();
    
    if (available <= total) {
        printf("✓ Valid bounds\n");
    } else {
        printf("✗ Invalid bounds: available > total\n");
    }
    
    // Test 3: Sleep test
    printf("3. Sleep accuracy test... ");
    uint64_t start = exs_platform_get_tick_count();
    exs_platform_sleep_ms(100);
    uint64_t end = exs_platform_get_tick_count();
    uint64_t elapsed = end - start;
    
    if (elapsed >= 80 && elapsed <= 120) { // 20% tolerance
        printf("✓ Within tolerance (%llu ms)\n", (unsigned long long)elapsed);
    } else {
        printf("✗ Outside tolerance (%llu ms)\n", (unsigned long long)elapsed);
    }
    
    // Test 4: Error handling
    printf("4. Error handling test... ");
    exs_platform_clear_error();
    const char* error = exs_platform_get_last_error();
    
    if (strcmp(error, "No error") == 0) {
        printf("✓ Error cleared\n");
    } else {
        printf("✗ Error not cleared: %s\n", error);
    }
    
    printf("\n=== Integration Test Complete ===\n");
    return 0;
}
