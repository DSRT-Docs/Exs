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

int main() {
    printf("=== Exs Platform C API Test ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Platform name
    total++;
    const char* name = exs_platform_get_name();
    if (name && strlen(name) > 0) {
        printf("✓ Platform name: %s\n", name);
        passed++;
    } else {
        printf("✗ Platform name failed\n");
    }
    
    // Test 2: Architecture
    total++;
    const char* arch = exs_platform_get_architecture();
    if (arch && strlen(arch) > 0) {
        printf("✓ Architecture: %s\n", arch);
        passed++;
    } else {
        printf("✗ Architecture failed\n");
    }
    
    // Test 3: CPU count
    total++;
    uint32_t cpu_count = exs_platform_get_cpu_count();
    if (cpu_count > 0 && cpu_count < 1024) {
        printf("✓ CPU count: %u\n", cpu_count);
        passed++;
    } else {
        printf("✗ CPU count failed: %u\n", cpu_count);
    }
    
    // Test 4: Total memory
    total++;
    uint64_t total_mem = exs_platform_get_total_memory();
    if (total_mem > 0) {
        printf("✓ Total memory: %llu bytes\n", (unsigned long long)total_mem);
        passed++;
    } else {
        printf("✗ Total memory failed\n");
    }
    
    // Test 5: Cache line size
    total++;
    uint32_t cache_line = exs_platform_get_cache_line_size();
    if (cache_line >= 16 && cache_line <= 512) {
        printf("✓ Cache line: %u bytes\n", cache_line);
        passed++;
    } else {
        printf("✗ Cache line failed: %u\n", cache_line);
    }
    
    // Test 6: 64-bit detection
    total++;
    bool is_64bit = exs_platform_is_64bit();
    printf("✓ 64-bit: %s\n", is_64bit ? "Yes" : "No");
    passed++;
    
    // Test 7: CPU features
    total++;
    printf("✓ SSE: %s\n", exs_platform_has_sse() ? "Yes" : "No");
    printf("✓ AVX: %s\n", exs_platform_has_avx() ? "Yes" : "No");
    printf("✓ NEON: %s\n", exs_platform_has_neon() ? "Yes" : "No");
    passed++;
    
    // Test 8: Sleep function
    total++;
    printf("Testing sleep 100ms...\n");
    uint64_t start = exs_platform_get_tick_count();
    exs_platform_sleep_ms(100);
    uint64_t end = exs_platform_get_tick_count();
    uint64_t elapsed = end - start;
    
    if (elapsed >= 50 && elapsed <= 200) { // Tolerance
        printf("✓ Sleep test: %llu ms\n", (unsigned long long)elapsed);
        passed++;
    } else {
        printf("✗ Sleep test failed: %llu ms\n", (unsigned long long)elapsed);
    }
    
    // Test 9: Memory allocation
    total++;
    void* mem = exs_platform_aligned_alloc(256, 64);
    if (mem) {
        printf("✓ Aligned allocation: %p\n", mem);
        exs_platform_aligned_free(mem);
        printf("✓ Memory freed\n");
        passed++;
    } else {
        printf("✗ Allocation failed\n");
    }
    
    // Test 10: Error handling
    total++;
    const char* error = exs_platform_get_last_error();
    printf("✓ Last error: %s\n", error);
    exs_platform_clear_error();
    error = exs_platform_get_last_error();
    if (strcmp(error, "No error") == 0) {
        printf("✓ Error cleared\n");
        passed++;
    } else {
        printf("✗ Error clear failed\n");
    }
    
    printf("\n=== Results: %d/%d passed ===\n", passed, total);
    return (passed == total) ? 0 : 1;
}
