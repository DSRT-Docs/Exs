/*
 * Copyright [2024] [DSRT-Docs]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <iostream>
#include <cassert>
#include "../../include/exs_platform.hpp"

int main() {
    std::cout << "=== Exs Platform C++ API Test ===\n\n";
    
    int passed = 0;
    int total = 0;
    
    try {
        // Test 1: Basic info
        total++;
        std::string name = exs::Platform::name();
        std::string arch = exs::Platform::architecture();
        std::string compiler = exs::Platform::compiler();
        
        if (!name.empty() && !arch.empty() && !compiler.empty()) {
            std::cout << "✓ Platform: " << name << std::endl;
            std::cout << "✓ Architecture: " << arch << std::endl;
            std::cout << "✓ Compiler: " << compiler << std::endl;
            passed++;
        } else {
            std::cout << "✗ Basic info failed\n";
        }
        
        // Test 2: CPU info
        total++;
        uint32_t cpu_count = exs::Platform::cpu_count();
        if (cpu_count > 0) {
            std::cout << "✓ CPU count: " << cpu_count << std::endl;
            passed++;
        } else {
            std::cout << "✗ CPU count failed\n";
        }
        
        // Test 3: Memory info
        total++;
        uint64_t total_mem = exs::Platform::total_memory();
        uint64_t avail_mem = exs::Platform::available_memory();
        
        if (total_mem > 0 && avail_mem <= total_mem) {
            std::cout << "✓ Total memory: " << exs::Platform::memory_string() << std::endl;
            std::cout << "✓ Memory usage: " << exs::Platform::memory_usage_percentage() << "%" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Memory info failed\n";
        }
        
        // Test 4: CPU features
        total++;
        std::cout << "✓ SSE: " << (exs::Platform::has_sse() ? "Yes" : "No") << std::endl;
        std::cout << "✓ AVX: " << (exs::Platform::has_avx() ? "Yes" : "No") << std::endl;
        std::cout << "✓ NEON: " << (exs::Platform::has_neon() ? "Yes" : "No") << std::endl;
        passed++;
        
        // Test 5: Platform type
        total++;
        exs::PlatformType type = exs::Platform::type();
        std::cout << "✓ Platform type: ";
        switch(type) {
            case exs::PlatformType::Desktop: std::cout << "Desktop"; break;
            case exs::PlatformType::Mobile: std::cout << "Mobile"; break;
            case exs::PlatformType::Server: std::cout << "Server"; break;
            default: std::cout << "Unknown"; break;
        }
        std::cout << std::endl;
        passed++;
        
        // Test 6: Processor vendor
        total++;
        exs::ProcessorVendor vendor = exs::Platform::processor_vendor();
        std::cout << "✓ Processor vendor: " << static_cast<int>(vendor) << std::endl;
        passed++;
        
        // Test 7: Cache line
        total++;
        uint32_t cache_line = exs::Platform::cache_line_size();
        if (cache_line >= 16) {
            std::cout << "✓ Cache line: " << cache_line << " bytes" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Cache line failed\n";
        }
        
        // Test 8: Endianness
        total++;
        exs::Endianness endian = exs::Platform::endianness();
        std::cout << "✓ Endianness: ";
        switch(endian) {
            case exs::Endianness::Little: std::cout << "Little"; break;
            case exs::Endianness::Big: std::cout << "Big"; break;
            default: std::cout << "Unknown"; break;
        }
        std::cout << std::endl;
        passed++;
        
        // Test 9: Timing functions
        total++;
        uint64_t timer1 = exs::Platform::high_res_timer();
        double freq = exs::Platform::timer_frequency();
        uint64_t timer2 = exs::Platform::high_res_timer();
        
        if (freq > 0 && timer2 >= timer1) {
            std::cout << "✓ Timer frequency: " << freq << " Hz" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Timer functions failed\n";
        }
        
        // Test 10: Memory alignment
        total++;
        void* ptr = exs::Platform::aligned_alloc(1024, 64);
        if (ptr) {
            bool aligned = exs::Platform::is_aligned(ptr, 64);
            if (aligned) {
                std::cout << "✓ Aligned allocation successful" << std::endl;
                passed++;
            }
            exs::Platform::aligned_free(ptr);
        } else {
            std::cout << "✗ Allocation failed\n";
        }
        
        // Test 11: Error handling
        total++;
        std::string error = exs::Platform::last_error();
        bool has_error = exs::Platform::has_error();
        exs::Platform::clear_error();
        
        std::cout << "✓ Last error: " << error << std::endl;
        std::cout << "✓ Has error: " << (has_error ? "Yes" : "No") << std::endl;
        passed++;
        
        // Test 12: Platform checks (compile-time)
        total++;
        std::cout << "✓ Is Windows: " << exs::Platform::is_windows() << std::endl;
        std::cout << "✓ Is Linux: " << exs::Platform::is_linux() << std::endl;
        std::cout << "✓ Is macOS: " << exs::Platform::is_macos() << std::endl;
        std::cout << "✓ Is Desktop: " << exs::Platform::is_desktop() << std::endl;
        std::cout << "✓ Is Mobile: " << exs::Platform::is_mobile() << std::endl;
        passed++;
        
        // Test 13: Print info (doesn't crash)
        total++;
        std::cout << "\nTesting print_info()...\n";
        exs::Platform::print_info();
        std::cout << "✓ print_info() completed\n";
        passed++;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Results: " << passed << "/" << total << " passed ===\n";
    return (passed == total) ? 0 : 1;
}
