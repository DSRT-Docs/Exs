#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/exs_platform.h"

void test_basic_info() {
    printf("=== Testing Basic Platform Info ===\n");
    
    const char* name = exs_platform_get_name();
    const char* arch = exs_platform_get_architecture();
    const char* compiler = exs_platform_get_compiler();
    
    printf("Platform: %s\n", name);
    printf("Architecture: %s\n", arch);
    printf("Compiler: %s\n", compiler);
    
    // Basic assertions
    assert(name != NULL && strlen(name) > 0);
    assert(arch != NULL && strlen(arch) > 0);
    assert(compiler != NULL);
    
    printf("✓ Basic info test passed\n\n");
}

void test_cpu_info() {
    printf("=== Testing CPU Info ===\n");
    
    uint32_t cpu_count = exs_platform_get_cpu_count();
    uint32_t cache_line = exs_platform_get_cache_line_size();
    
    printf("CPU Count: %u\n", cpu_count);
    printf("Cache Line Size: %u bytes\n", cache_line);
    
    // Validasi nilai minimal
    assert(cpu_count > 0 && cpu_count <= 1024); // Batasan realistis
    assert(cache_line >= 16 && cache_line <= 512); // 16-512 byte umum
    
    printf("✓ CPU info test passed\n\n");
}

void test_memory_info() {
    printf("=== Testing Memory Info ===\n");
    
    uint64_t total_mem = exs_platform_get_total_memory();
    uint64_t avail_mem = exs_platform_get_available_memory();
    
    printf("Total Memory: %llu bytes (%.2f GB)\n", 
           (unsigned long long)total_mem, 
           total_mem / (1024.0 * 1024 * 1024));
    
    if (avail_mem > 0) {
        printf("Available Memory: %llu bytes (%.2f GB)\n", 
               (unsigned long long)avail_mem,
               avail_mem / (1024.0 * 1024 * 1024));
    }
    
    // Validasi memori
    assert(total_mem > 0);
    assert(avail_mem <= total_mem);
    
    printf("✓ Memory info test passed\n\n");
}

void test_cpu_features() {
    printf("=== Testing CPU Features ===\n");
    
    bool has_sse = exs_platform_has_sse();
    bool has_avx = exs_platform_has_avx();
    bool has_neon = exs_platform_has_neon();
    bool is_64bit = exs_platform_is_64bit();
    
    printf("SSE: %s\n", has_sse ? "Yes" : "No");
    printf("AVX: %s\n", has_avx ? "Yes" : "No");
    printf("NEON: %s\n", has_neon ? "Yes" : "No");
    printf("64-bit: %s\n", is_64bit ? "Yes" : "No");
    
    // Tidak ada assertion karena features tergantung hardware
    printf("✓ CPU features test passed\n\n");
}

void test_timing_functions() {
    printf("=== Testing Timing Functions ===\n");
    
    uint64_t tick1 = exs_platform_get_tick_count();
    exs_platform_sleep_ms(100);
    uint64_t tick2 = exs_platform_get_tick_count();
    
    uint64_t elapsed = tick2 - tick1;
    printf("Sleep 100ms, elapsed: %llu ms\n", (unsigned long long)elapsed);
    
    // Validasi sleep minimal 50ms (memungkinkan variasi)
    assert(elapsed >= 50 && elapsed <= 200); // Tolerance
    
    // Test high resolution timer
    uint64_t hr_start = exs_platform_get_high_res_timer();
    double freq = exs_platform_get_timer_frequency();
    for (int i = 0; i < 1000; i++) { /* busy wait */ }
    uint64_t hr_end = exs_platform_get_high_res_timer();
    
    printf("High-res timer frequency: %.0f Hz\n", freq);
    printf("Timer delta: %llu ticks\n", (unsigned long long)(hr_end - hr_start));
    
    assert(freq > 0);
    assert(hr_end >= hr_start);
    
    printf("✓ Timing functions test passed\n\n");
}

void test_memory_allocation() {
    printf("=== Testing Aligned Allocation ===\n");
    
    size_t sizes[] = {64, 128, 256, 1024, 4096};
    size_t alignments[] = {16, 32, 64, 128, 4096};
    
    for (int i = 0; i < 5; i++) {
        void* ptr = exs_platform_aligned_alloc(sizes[i], alignments[i]);
        assert(ptr != NULL);
        
        // Check alignment
        uintptr_t addr = (uintptr_t)ptr;
        assert((addr & (alignments[i] - 1)) == 0);
        
        // Write some data
        memset(ptr, 0xAA, sizes[i]);
        
        printf("  Allocated %zu bytes aligned to %zu: %p\n", 
               sizes[i], alignments[i], ptr);
        
        exs_platform_aligned_free(ptr);
    }
    
    printf("✓ Memory allocation test passed\n\n");
}

void test_error_handling() {
    printf("=== Testing Error Handling ===\n");
    
    const char* error = exs_platform_get_last_error();
    printf("Initial error: %s\n", error);
    
    // Clear error
    exs_platform_clear_error();
    error = exs_platform_get_last_error();
    printf("After clear: %s\n", error);
    
    assert(strcmp(error, "No error") == 0);
    printf("✓ Error handling test passed\n\n");
}

void test_debugger_detection() {
    printf("=== Testing Debugger Detection ===\n");
    
    bool debugger_present = exs_platform_is_debugger_present();
    printf("Debugger present: %s\n", debugger_present ? "Yes" : "No");
    
    printf("✓ Debugger detection test passed\n\n");
}

int main() {
    printf("================================\n");
    printf("Exs Platform Library Test Suite\n");
    printf("================================\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Jalankan semua test
    #define RUN_TEST(test_func) \
        do { \
            total_tests++; \
            try { \
                test_func(); \
                passed_tests++; \
            } catch (...) { \
                printf("✗ Test failed: " #test_func "\n"); \
            } \
        } while(0)
    
    RUN_TEST(test_basic_info);
    RUN_TEST(test_cpu_info);
    RUN_TEST(test_memory_info);
    RUN_TEST(test_cpu_features);
    RUN_TEST(test_timing_functions);
    RUN_TEST(test_memory_allocation);
    RUN_TEST(test_error_handling);
    RUN_TEST(test_debugger_detection);
    
    printf("================================\n");
    printf("Test Summary:\n");
    printf("  Passed: %d/%d\n", passed_tests, total_tests);
    printf("  Status: %s\n", 
           (passed_tests == total_tests) ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    printf("================================\n");
    
    return (passed_tests == total_tests) ? 0 : 1;
}
