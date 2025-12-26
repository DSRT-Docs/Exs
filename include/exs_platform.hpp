/*
 * Copyright [2024] [DSRT-Docs]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EXS_PLATFORM_HPP
#define EXS_PLATFORM_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace exs {

// Enums - TETAP
enum class ProcessorVendor {
    Unknown,
    Intel,
    AMD,
    ARM,
    Apple,
    Qualcomm,
    Samsung,
    Nvidia,
    IBM,
    Other
};

enum class Endianness {
    Unknown,
    Little,
    Big
};

enum class PlatformType {
    Unknown,
    Desktop,
    Mobile,
    Server,
    Embedded,
    GameConsole,
    Virtual
};

class Platform {
private:
    class Impl;  // Forward declaration (PIMPL pattern)
    static Impl& get_impl();
    
public:
    // Constructor/destructor dll dihapus karena class static-only
    Platform() = delete;
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    
    // === LICENSE FUNCTIONS ===
    static void print_license();
    static const char* license_text();
    
    // === SEMUA FUNGSI HANYA DEKLARASI (implementasi di .cpp) ===
    
    // Basic Info - DEKLARASI SAJA
    static std::string name();
    static std::string architecture();
    static std::string compiler();
    static PlatformType type();
    
    // CPU Info - DEKLARASI SAJA
    static uint32_t cpu_count();
    static ProcessorVendor processor_vendor();
    static std::string processor_brand();
    static uint32_t cpu_frequency_mhz();
    
    // Memory Info - DEKLARASI SAJA
    static uint64_t total_memory();
    static uint64_t available_memory();
    static uint32_t page_size();
    static uint32_t cache_line_size();
    
    // CPU Features - DEKLARASI SAJA
    static bool has_sse();
    static bool has_avx();
    static bool has_neon();
    static bool has_sse2();
    static bool has_sse3();
    static bool has_sse41();
    static bool has_sse42();
    static bool has_avx2();
    static bool has_avx512();
    static bool has_fma();
    static bool has_aes();
    static bool has_sha();
    
    // Cache Info - DEKLARASI SAJA
    static uint32_t l1_cache_size();
    static uint32_t l2_cache_size();
    static uint32_t l3_cache_size();
    static uint32_t tlb_size();
    
    // System Info - DEKLARASI SAJA
    static Endianness endianness();
    static bool is_64bit();
    static bool is_64bit_os();
    static bool is_debugger_present();
    static bool is_administrator();
    static bool is_virtual_machine();
    static bool is_battery_powered();
    static bool is_network_connected();
    
    // Paths - DEKLARASI SAJA
    static std::string temp_directory();
    static std::string home_directory();
    static std::string current_directory();
    static std::string executable_path();
    static std::string module_path(void* address = nullptr);
    static std::vector<std::string> library_paths();
    
    // Timing - DEKLARASI SAJA
    static void debug_break();
    static void sleep_ms(uint32_t ms);
    static void sleep_us(uint64_t us);
    static void sleep_ns(uint64_t ns);
    static uint64_t high_res_timer();
    static double timer_frequency();
    static uint64_t system_time();
    static uint64_t uptime();
    static uint64_t cpu_cycles();
    
    // Threading - DEKLARASI SAJA
    static bool set_thread_affinity(uint32_t core_mask);
    static uint32_t thread_affinity();
    static bool set_thread_priority(int priority);
    static int thread_priority();
    static void yield();
    
    // Memory Management - DEKLARASI SAJA
    static bool is_aligned(const void* ptr, size_t alignment);
    static void* aligned_alloc(size_t size, size_t alignment);
    static void aligned_free(void* ptr);
    static size_t allocation_granularity();
    
    // System Identification - DEKLARASI SAJA
    static std::string system_fingerprint();
    static std::string os_version();
    static std::string kernel_version();
    static std::string hostname();
    static std::string username();
    static uint32_t process_id();
    static uint32_t parent_process_id();
    static uint32_t session_id();
    
    // Hardware Info - DEKLARASI SAJA
    static uint32_t display_count();
    static std::string gpu_name();
    static std::string system_manufacturer();
    static std::string system_model();
    static std::string bios_version();
    
    // Monitoring - DEKLARASI SAJA
    static double cpu_load();
    static double memory_usage();
    static double disk_usage(const std::string& path = "");
    static std::vector<double> cpu_temperatures();
    
    // Utility Methods - DEKLARASI SAJA
    static std::string memory_string(bool human_readable = true);
    static double memory_usage_percentage();
    static void print_info();
    
    // Error Handling - DEKLARASI SAJA
    static std::string last_error();
    static bool has_error();
    static void clear_error();
    
    // Platform Checks (Compile-time) - TETAP INLINE
    static constexpr bool is_windows() {
#ifdef EXS_PLATFORM_WINDOWS
        return true;
#else
        return false;
#endif
    }
    
    static constexpr bool is_macos() {
#ifdef EXS_PLATFORM_MACOS
        return true;
#else
        return false;
#endif
    }
    
    static constexpr bool is_linux() {
#ifdef EXS_PLATFORM_LINUX
        return true;
#else
        return false;
#endif
    }
    
    static constexpr bool is_android() {
#ifdef EXS_PLATFORM_ANDROID
        return true;
#else
        return false;
#endif
    }
    
    static constexpr bool is_ios() {
#ifdef EXS_PLATFORM_IOS
        return true;
#else
        return false;
#endif
    }
    
    static constexpr bool is_desktop() {
#if defined(EXS_PLATFORM_WINDOWS) || defined(EXS_PLATFORM_MACOS) || defined(EXS_PLATFORM_LINUX)
        return true;
#else
        return false;
#endif
    }
    
    static constexpr bool is_mobile() {
#if defined(EXS_PLATFORM_ANDROID) || defined(EXS_PLATFORM_IOS)
        return true;
#else
        return false;
#endif
    }
    
    static constexpr bool is_unix() {
#if defined(__unix__) || defined(__APPLE__)
        return true;
#else
        return false;
#endif
    }
};

// Free functions - DEKLARASI SAJA
bool is_windows_platform();
bool is_macos_platform();
bool is_linux_platform();
bool is_android_platform();
bool is_ios_platform();
bool is_desktop_platform();
bool is_mobile_platform();

} // namespace exs

#endif // EXS_PLATFORM_HPP
