#ifndef EXS_PLATFORM_HPP
#define EXS_PLATFORM_HPP

#include "exs_platform.h"
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>    // ← TAMBAH INI!

namespace exs {

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
    struct Cache {
        std::atomic<uint32_t> cpu_count{0};
        std::atomic<uint64_t> total_memory{0};
        std::atomic<ProcessorVendor> vendor{ProcessorVendor::Unknown};
        std::atomic<Endianness> endian{Endianness::Unknown};
        std::atomic<bool> initialized{false};
        std::mutex init_mutex;
    };
    
    static Cache& get_cache() {
        static Cache cache;
        return cache;
    }
    
    static void initialize_cache() {
        auto& cache = get_cache();
        if (!cache.initialized.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(cache.init_mutex);
            if (!cache.initialized.load(std::memory_order_relaxed)) {
                // Initialize cache values
                cache.cpu_count.store(exs_platform_get_cpu_count(), std::memory_order_release);
                cache.total_memory.store(exs_platform_get_total_memory(), std::memory_order_release);
                cache.vendor.store(detect_processor_vendor(), std::memory_order_release);
                cache.endian.store(detect_endianness(), std::memory_order_release);
                cache.initialized.store(true, std::memory_order_release);
            }
        }
    }
    
    static ProcessorVendor detect_processor_vendor() {
        // Implementation for processor vendor detection
        return ProcessorVendor::Unknown;
    }
    
    static Endianness detect_endianness() {
        union {
            uint32_t i;
            uint8_t c[4];
        } test = {0x01020304};
        
        if (test.c[0] == 0x01 && test.c[1] == 0x02) return Endianness::Big;
        if (test.c[0] == 0x04 && test.c[1] == 0x03) return Endianness::Little;
        return Endianness::Unknown;
    }

public:
    Platform() = delete;
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    
    // Basic Info
    static std::string name() { return exs_platform_get_name(); }
    static std::string architecture() { return exs_platform_get_architecture(); }
    static std::string compiler() { return exs_platform_get_compiler(); }
    static PlatformType type();
    
    // CPU Info
    static uint32_t cpu_count() {
        initialize_cache();
        return get_cache().cpu_count.load(std::memory_order_acquire);
    }
    
    static ProcessorVendor processor_vendor() {
        initialize_cache();
        return get_cache().vendor.load(std::memory_order_acquire);
    }
    
    static std::string processor_brand();
    static uint32_t cpu_frequency_mhz();
    
    // Memory Info
    static uint64_t total_memory() {
        initialize_cache();
        return get_cache().total_memory.load(std::memory_order_acquire);
    }
    
    static uint64_t available_memory() { return exs_platform_get_available_memory(); }
    static uint32_t page_size();
    static uint32_t cache_line_size() { return exs_platform_get_cache_line_size(); }
    
    // CPU Features
    static bool has_sse() { return exs_platform_has_sse(); }
    static bool has_avx() { return exs_platform_has_avx(); }
    static bool has_neon() { return exs_platform_has_neon(); }
    static bool has_sse2();
    static bool has_sse3();
    static bool has_sse41();
    static bool has_sse42();
    static bool has_avx2();
    static bool has_avx512();
    static bool has_fma();
    static bool has_aes();
    static bool has_sha();
    
    // Cache Info
    static uint32_t l1_cache_size();
    static uint32_t l2_cache_size();
    static uint32_t l3_cache_size();
    static uint32_t tlb_size();
    
    // System Info
    static Endianness endianness() {
        initialize_cache();
        return get_cache().endian.load(std::memory_order_acquire);
    }
    
    static bool is_64bit() { return exs_platform_is_64bit(); }
    static bool is_64bit_os();
    static bool is_debugger_present() { return exs_platform_is_debugger_present(); }
    static bool is_administrator();
    static bool is_virtual_machine();
    static bool is_battery_powered();
    static bool is_network_connected();
    
    // Paths
    static std::string temp_directory();
    static std::string home_directory();
    static std::string current_directory();
    static std::string executable_path();
    static std::string module_path(void* address = nullptr);
    static std::vector<std::string> library_paths();
    
    // Timing
    static void debug_break() { exs_platform_debug_break(); }
    static void sleep_ms(uint32_t ms) { exs_platform_sleep_ms(ms); }
    static void sleep_us(uint64_t us) { exs_platform_sleep_us(us); }
    static void sleep_ns(uint64_t ns);
    
    static uint64_t high_res_timer() { return exs_platform_get_high_res_timer(); }
    static double timer_frequency() { return exs_platform_get_timer_frequency(); }
    static uint64_t system_time() { return exs_platform_get_tick_count(); }
    static uint64_t uptime();
    static uint64_t cpu_cycles();
    
    // Threading
    static bool set_thread_affinity(uint32_t core_mask);
    static uint32_t thread_affinity();
    static bool set_thread_priority(int priority);
    static int thread_priority();
    static void yield() { std::this_thread::yield(); }
    
    // Memory Management
    static bool is_aligned(const void* ptr, size_t alignment) {
        return ((uintptr_t)ptr & (alignment - 1)) == 0;
    }
    
    static void* aligned_alloc(size_t size, size_t alignment) {
        return exs_platform_aligned_alloc(size, alignment);
    }
    
    static void aligned_free(void* ptr) { exs_platform_aligned_free(ptr); }
    static size_t allocation_granularity();
    
    // System Identification
    static std::string system_fingerprint();
    static std::string os_version();
    static std::string kernel_version();
    static std::string hostname();
    static std::string username();
    static uint32_t process_id();
    static uint32_t parent_process_id();
    static uint32_t session_id();
    
    // Hardware Info
    static uint32_t display_count();
    static std::string gpu_name();
    static std::string system_manufacturer();
    static std::string system_model();
    static std::string bios_version();
    
    // Monitoring
    static double cpu_load();
    static double memory_usage();
    static double disk_usage(const std::string& path = "");
    static std::vector<double> cpu_temperatures();
    
    // Utility Methods
    static std::string memory_string(bool human_readable = true) {
        uint64_t total = total_memory();
        
        if (!human_readable) {
            return std::to_string(total) + " bytes";
        }
        
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit = 0;
        double size = static_cast<double>(total);
        
        while (size >= 1024.0 && unit < 4) {
            size /= 1024.0;
            unit++;
        }
        
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << size << " " << units[unit];
        return ss.str();
    }
    
    static double memory_usage_percentage() {
        uint64_t total = total_memory();
        uint64_t available = available_memory();
        
        if (total == 0) return 0.0;
        if (available > total) return 0.0;
        
        return 100.0 * (1.0 - static_cast<double>(available) / static_cast<double>(total));
    }
    
    static void print_info() {
        std::cout << "\n=== System Information ===" << std::endl;
        std::cout << "Platform: " << name() << std::endl;
        std::cout << "Architecture: " << architecture() << std::endl;
        std::cout << "Compiler: " << compiler() << std::endl;
        std::cout << "OS Type: ";
        switch (type()) {
            case PlatformType::Desktop: std::cout << "Desktop"; break;
            case PlatformType::Mobile: std::cout << "Mobile"; break;
            case PlatformType::Server: std::cout << "Server"; break;
            case PlatformType::Embedded: std::cout << "Embedded"; break;
            default: std::cout << "Unknown"; break;
        }
        std::cout << std::endl;
        
        std::cout << "\nCPU Information:" << std::endl;
        std::cout << "  Cores: " << cpu_count() << std::endl;
        std::cout << "  Vendor: ";
        switch (processor_vendor()) {
            case ProcessorVendor::Intel: std::cout << "Intel"; break;
            case ProcessorVendor::AMD: std::cout << "AMD"; break;
            case ProcessorVendor::ARM: std::cout << "ARM"; break;
            case ProcessorVendor::Apple: std::cout << "Apple"; break;
            default: std::cout << "Unknown"; break;
        }
        std::cout << std::endl;
        
        std::cout << "\nMemory Information:" << std::endl;
        std::cout << "  Total: " << memory_string() << std::endl;
        std::cout << "  Usage: " << std::fixed << std::setprecision(1) 
                  << memory_usage_percentage() << "%" << std::endl;
        std::cout << "  Cache Line: " << cache_line_size() << " bytes" << std::endl;
        
        std::cout << "\nCPU Features:" << std::endl;
        std::cout << "  SSE: " << (has_sse() ? "Yes" : "No") << std::endl;
        std::cout << "  AVX: " << (has_avx() ? "Yes" : "No") << std::endl;
        std::cout << "  NEON: " << (has_neon() ? "Yes" : "No") << std::endl;
        
        std::cout << "\nSystem Status:" << std::endl;
        std::cout << "  64-bit: " << (is_64bit() ? "Yes" : "No") << std::endl;
        std::cout << "  Debugger: " << (is_debugger_present() ? "Yes" : "No") << std::endl;
        std::cout << "  Admin/Root: " << (is_administrator() ? "Yes" : "No") << std::endl;
        
        std::cout << "\nTiming:" << std::endl;
        std::cout << "  Timer Frequency: " << timer_frequency() << " Hz" << std::endl;
        std::cout << "  System Time: " << system_time() << " ms" << std::endl;
        
        std::cout << "\nPaths:" << std::endl;
        std::cout << "  Temp: " << temp_directory() << std::endl;
        std::cout << "  Home: " << home_directory() << std::endl;
        std::cout << "  Current: " << current_directory() << std::endl;
        
        std::cout << "\nSystem Fingerprint: " << system_fingerprint() << std::endl;
        std::cout << "==========================" << std::endl;
    }
    
    // Error Handling
    static std::string last_error() { return exs_platform_get_last_error(); }
    static bool has_error() { 
    return std::strcmp(exs_platform_get_last_error(), "No error") != 0; 
}
    static void clear_error() { exs_platform_clear_error(); }
    
    // Platform Checks (Compile-time)
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

// Free functions for C-style compatibility
inline bool is_windows_platform() { return Platform::is_windows(); }
inline bool is_macos_platform() { return Platform::is_macos(); }
inline bool is_linux_platform() { return Platform::is_linux(); }
inline bool is_android_platform() { return Platform::is_android(); }
inline bool is_ios_platform() { return Platform::is_ios(); }
inline bool is_desktop_platform() { return Platform::is_desktop(); }
inline bool is_mobile_platform() { return Platform::is_mobile(); }

} // namespace exs

#endif // EXS_PLATFORM_HPP
