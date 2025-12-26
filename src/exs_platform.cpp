#include "exs_platform.hpp"
#include "exs_platform.h"
#include "platform_impl.hpp"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <thread>

namespace exs {

class Platform::Impl {
private:
    std::string cached_name;
    uint32_t cached_cpu_count;
    uint64_t cached_total_memory;
    
public:
    Impl() {
        cached_name = exs_platform_get_name();
        cached_cpu_count = exs_platform_get_cpu_count();
        cached_total_memory = exs_platform_get_total_memory();
        
        internal::PlatformImpl::update_cache(cached_cpu_count, cached_total_memory);
    }
    
    const std::string& get_name() const { return cached_name; }
    uint32_t get_cpu_count() const { return cached_cpu_count; }
    uint64_t get_total_memory() const { return cached_total_memory; }
};

Platform::Impl& Platform::get_impl() {
    static Impl instance;
    return instance;
}

std::string Platform::name() {
    return get_impl().get_name();
}

std::string Platform::architecture() {
    return exs_platform_get_architecture();
}

std::string Platform::compiler() {
    return exs_platform_get_compiler();
}

PlatformType Platform::type() {
#ifdef EXS_PLATFORM_WINDOWS
    return PlatformType::Desktop;
#elif defined(EXS_PLATFORM_MACOS) || defined(EXS_PLATFORM_LINUX)
    return PlatformType::Desktop;
#elif defined(EXS_PLATFORM_ANDROID) || defined(EXS_PLATFORM_IOS)
    return PlatformType::Mobile;
#elif defined(EXS_PLATFORM_UNIX)
    return PlatformType::Server;
#else
    return PlatformType::Unknown;
#endif
}

uint32_t Platform::cpu_count() {
    return internal::PlatformImpl::get_cpu_count_cached();
}

ProcessorVendor Platform::processor_vendor() {
    const char* arch = exs_platform_get_architecture();
    
    if (std::strstr(arch, "ARM") || std::strstr(arch, "aarch64")) {
#ifdef __APPLE__
        return ProcessorVendor::Apple;
#else
        return ProcessorVendor::ARM;
#endif
    }
    
    if (std::strstr(arch, "x86") || std::strstr(arch, "x64")) {
        return ProcessorVendor::Intel;
    }
    
    return ProcessorVendor::Unknown;
}

std::string Platform::processor_brand() {
    return "";
}

uint32_t Platform::cpu_frequency_mhz() {
    return 0;
}

uint64_t Platform::total_memory() {
    return internal::PlatformImpl::get_total_memory_cached();
}

uint64_t Platform::available_memory() {
    return exs_platform_get_available_memory();
}

uint32_t Platform::page_size() {
    return 4096;
}

uint32_t Platform::cache_line_size() {
    return exs_platform_get_cache_line_size();
}

bool Platform::has_sse() {
    return exs_platform_has_sse();
}

bool Platform::has_avx() {
    return exs_platform_has_avx();
}

bool Platform::has_neon() {
    return exs_platform_has_neon();
}

bool Platform::has_sse2() {
    return false;
}

bool Platform::has_sse3() {
    return false;
}

bool Platform::has_sse41() {
    return false;
}

bool Platform::has_sse42() {
    return false;
}

bool Platform::has_avx2() {
    return false;
}

bool Platform::has_avx512() {
    return false;
}

bool Platform::has_fma() {
    return false;
}

bool Platform::has_aes() {
    return false;
}

bool Platform::has_sha() {
    return false;
}

uint32_t Platform::l1_cache_size() {
    return 0;
}

uint32_t Platform::l2_cache_size() {
    return 0;
}

uint32_t Platform::l3_cache_size() {
    return 0;
}

uint32_t Platform::tlb_size() {
    return 0;
}

Endianness Platform::endianness() {
    union {
        uint32_t i;
        uint8_t c[4];
    } test = {0x01020304};
    
    if (test.c[0] == 0x01 && test.c[1] == 0x02) return Endianness::Big;
    if (test.c[0] == 0x04 && test.c[1] == 0x03) return Endianness::Little;
    return Endianness::Unknown;
}

bool Platform::is_64bit() {
    return exs_platform_is_64bit();
}

bool Platform::is_64bit_os() {
    return exs_platform_is_64bit();
}

bool Platform::is_debugger_present() {
    return exs_platform_is_debugger_present();
}

bool Platform::is_administrator() {
    return false;
}

bool Platform::is_virtual_machine() {
    return false;
}

bool Platform::is_battery_powered() {
    return false;
}

bool Platform::is_network_connected() {
    return false;
}

std::string Platform::temp_directory() {
    return "";
}

std::string Platform::home_directory() {
    return "";
}

std::string Platform::current_directory() {
    return "";
}

std::string Platform::executable_path() {
    return "";
}

std::string Platform::module_path(void* address) {
    return "";
}

std::vector<std::string> Platform::library_paths() {
    return std::vector<std::string>();
}

void Platform::debug_break() {
    exs_platform_debug_break();
}

void Platform::sleep_ms(uint32_t ms) {
    exs_platform_sleep_ms(ms);
}

void Platform::sleep_us(uint64_t us) {
    exs_platform_sleep_us(us);
}

void Platform::sleep_ns(uint64_t ns) {
    exs_platform_sleep_us(ns / 1000);
}

uint64_t Platform::high_res_timer() {
    return exs_platform_get_high_res_timer();
}

double Platform::timer_frequency() {
    return exs_platform_get_timer_frequency();
}

uint64_t Platform::system_time() {
    return exs_platform_get_tick_count();
}

uint64_t Platform::uptime() {
    return exs_platform_get_tick_count();
}

uint64_t Platform::cpu_cycles() {
    return 0;
}

bool Platform::set_thread_affinity(uint32_t core_mask) {
    return false;
}

uint32_t Platform::thread_affinity() {
    return 0;
}

bool Platform::set_thread_priority(int priority) {
    return false;
}

int Platform::thread_priority() {
    return 0;
}

void Platform::yield() {
    std::this_thread::yield();
}

bool Platform::is_aligned(const void* ptr, size_t alignment) {
    return ((uintptr_t)ptr & (alignment - 1)) == 0;
}

void* Platform::aligned_alloc(size_t size, size_t alignment) {
    return exs_platform_aligned_alloc(size, alignment);
}

void Platform::aligned_free(void* ptr) {
    exs_platform_aligned_free(ptr);
}

size_t Platform::allocation_granularity() {
    return 4096;
}

std::string Platform::system_fingerprint() {
    return "";
}

std::string Platform::os_version() {
    return "";
}

std::string Platform::kernel_version() {
    return "";
}

std::string Platform::hostname() {
    return "";
}

std::string Platform::username() {
    return "";
}

uint32_t Platform::process_id() {
    return 0;
}

uint32_t Platform::parent_process_id() {
    return 0;
}

uint32_t Platform::session_id() {
    return 0;
}

uint32_t Platform::display_count() {
    return 0;
}

std::string Platform::gpu_name() {
    return "";
}

std::string Platform::system_manufacturer() {
    return "";
}

std::string Platform::system_model() {
    return "";
}

std::string Platform::bios_version() {
    return "";
}

double Platform::cpu_load() {
    return 0.0;
}

double Platform::memory_usage() {
    uint64_t total = total_memory();
    uint64_t available = available_memory();
    if (total == 0) return 0.0;
    return 100.0 * (1.0 - static_cast<double>(available) / static_cast<double>(total));
}

double Platform::disk_usage(const std::string& path) {
    return 0.0;
}

std::vector<double> Platform::cpu_temperatures() {
    return std::vector<double>();
}

std::string Platform::memory_string(bool human_readable) {
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

double Platform::memory_usage_percentage() {
    return memory_usage();
}

void Platform::print_info() {
    std::cout << "\n=== System Information ===" << std::endl;
    std::cout << "Platform: " << name() << std::endl;
    std::cout << "Architecture: " << architecture() << std::endl;
    std::cout << "Compiler: " << compiler() << std::endl;
    
    std::cout << "\nCPU Information:" << std::endl;
    std::cout << "  Cores: " << cpu_count() << std::endl;
    
    std::cout << "\nMemory Information:" << std::endl;
    std::cout << "  Total: " << memory_string() << std::endl;
    std::cout << "  Usage: " << std::fixed << std::setprecision(1) 
              << memory_usage_percentage() << "%" << std::endl;
    
    std::cout << "\nCPU Features:" << std::endl;
    std::cout << "  SSE: " << (has_sse() ? "Yes" : "No") << std::endl;
    std::cout << "  AVX: " << (has_avx() ? "Yes" : "No") << std::endl;
    std::cout << "  NEON: " << (has_neon() ? "Yes" : "No") << std::endl;
    
    std::cout << "\nSystem Status:" << std::endl;
    std::cout << "  64-bit: " << (is_64bit() ? "Yes" : "No") << std::endl;
    std::cout << "  Debugger: " << (is_debugger_present() ? "Yes" : "No") << std::endl;
    
    std::cout << "==========================" << std::endl;
}

std::string Platform::last_error() {
    return exs_platform_get_last_error();
}

bool Platform::has_error() {
    return std::strcmp(exs_platform_get_last_error(), "No error") != 0;
}

void Platform::clear_error() {
    exs_platform_clear_error();
}

bool is_windows_platform() {
    return Platform::is_windows();
}

bool is_macos_platform() {
    return Platform::is_macos();
}

bool is_linux_platform() {
    return Platform::is_linux();
}

bool is_android_platform() {
    return Platform::is_android();
}

bool is_ios_platform() {
    return Platform::is_ios();
}

bool is_desktop_platform() {
    return Platform::is_desktop();
}

bool is_mobile_platform() {
    return Platform::is_mobile();
}

} // namespace exs
