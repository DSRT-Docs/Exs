#ifndef EXS_PLATFORM_H
#define EXS_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> 

#ifdef __cplusplus
extern "C" {
#endif

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define EXS_PLATFORM_WINDOWS 1
    #define EXS_PLATFORM_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #define EXS_PLATFORM_IOS 1
        #define EXS_PLATFORM_NAME "iOS"
    #elif TARGET_OS_MAC
        #define EXS_PLATFORM_MACOS 1
        #define EXS_PLATFORM_NAME "macOS"
    #else
        #define EXS_PLATFORM_APPLE_UNKNOWN 1
        #define EXS_PLATFORM_NAME "AppleUnknown"
    #endif
#elif defined(__ANDROID__)
    #define EXS_PLATFORM_ANDROID 1
    #define EXS_PLATFORM_NAME "Android"
#elif defined(__linux__)
    #define EXS_PLATFORM_LINUX 1
    #define EXS_PLATFORM_NAME "Linux"
#elif defined(__unix__)
    #define EXS_PLATFORM_UNIX 1
    #define EXS_PLATFORM_NAME "Unix"
#else
    #define EXS_PLATFORM_UNKNOWN 1
    #define EXS_PLATFORM_NAME "Unknown"
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
    #define EXS_ARCH_X64 1
    #define EXS_ARCH_NAME "x64"
#elif defined(__i386__) || defined(_M_IX86)
    #define EXS_ARCH_X86 1
    #define EXS_ARCH_NAME "x86"
#elif defined(__aarch64__) || defined(__ARM_ARCH_8A__) || defined(_M_ARM64)
    #define EXS_ARCH_ARM64 1
    #define EXS_ARCH_NAME "ARM64"
#elif defined(__arm__) || defined(__ARM_ARCH_7A__) || defined(_M_ARM)
    #define EXS_ARCH_ARM 1
    #define EXS_ARCH_NAME "ARM"
#elif defined(__powerpc64__)
    #define EXS_ARCH_PPC64 1
    #define EXS_ARCH_NAME "PPC64"
#elif defined(__powerpc__)
    #define EXS_ARCH_PPC 1
    #define EXS_ARCH_NAME "PPC"
#elif defined(__mips64)
    #define EXS_ARCH_MIPS64 1
    #define EXS_ARCH_NAME "MIPS64"
#elif defined(__mips__)
    #define EXS_ARCH_MIPS 1
    #define EXS_ARCH_NAME "MIPS"
#elif defined(__riscv)
    #if __riscv_xlen == 64
        #define EXS_ARCH_RISCV64 1
        #define EXS_ARCH_NAME "RISC-V64"
    #else
        #define EXS_ARCH_RISCV32 1
        #define EXS_ARCH_NAME "RISC-V32"
    #endif
#else
    #define EXS_ARCH_UNKNOWN 1
    #define EXS_ARCH_NAME "Unknown"
#endif

// Compiler detection
#if defined(_MSC_VER)
    #define EXS_COMPILER_MSVC 1
    #define EXS_COMPILER_VERSION _MSC_VER
    #if _MSC_VER >= 1930
        #define EXS_COMPILER_NAME "MSVC 2022+"
    #elif _MSC_VER >= 1920
        #define EXS_COMPILER_NAME "MSVC 2019"
    #elif _MSC_VER >= 1910
        #define EXS_COMPILER_NAME "MSVC 2017"
    #else
        #define EXS_COMPILER_NAME "MSVC"
    #endif
#elif defined(__clang__)
    #define EXS_COMPILER_CLANG 1
    #define EXS_COMPILER_VERSION __clang_major__
    #define EXS_COMPILER_NAME "Clang"
#elif defined(__GNUC__)
    #define EXS_COMPILER_GCC 1
    #define EXS_COMPILER_VERSION __GNUC__
    #define EXS_COMPILER_NAME "GCC"
#elif defined(__INTEL_COMPILER)
    #define EXS_COMPILER_ICC 1
    #define EXS_COMPILER_VERSION __INTEL_COMPILER
    #define EXS_COMPILER_NAME "Intel C++"
#else
    #define EXS_COMPILER_UNKNOWN 1
    #define EXS_COMPILER_NAME "Unknown"
#endif

// C API Functions
const char* exs_platform_get_name(void);
const char* exs_platform_get_architecture(void);
const char* exs_platform_get_compiler(void);
uint32_t exs_platform_get_cpu_count(void);
uint64_t exs_platform_get_total_memory(void);
uint64_t exs_platform_get_available_memory(void);
bool exs_platform_has_sse(void);
bool exs_platform_has_avx(void);
bool exs_platform_has_neon(void);
void exs_platform_debug_break(void);
void exs_platform_sleep_ms(uint32_t milliseconds);
void exs_platform_sleep_us(uint64_t microseconds);

// Error handling
const char* exs_platform_get_last_error(void);
void exs_platform_clear_error(void);

// Utility
bool exs_platform_is_64bit(void);
bool exs_platform_is_debugger_present(void);
uint64_t exs_platform_get_tick_count(void);

// Advanced features
uint32_t exs_platform_get_cache_line_size(void);
uint64_t exs_platform_get_high_res_timer(void);
double exs_platform_get_timer_frequency(void);
void* exs_platform_aligned_alloc(size_t size, size_t alignment);
void exs_platform_aligned_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // EXS_PLATFORM_H
