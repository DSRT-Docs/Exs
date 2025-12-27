#pragma once
#ifndef EXS_PLATFORM_DETECTION_H
#define EXS_PLATFORM_DETECTION_H

// Operating System Detection -------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
    #define EXS_PLATFORM_WINDOWS 1
    #ifndef EXS_PLATFORM_WINDOWS
        #define EXS_PLATFORM_WINDOWS
    #endif
    
    #if defined(_WIN64)
        #define EXS_PLATFORM_WINDOWS_64 1
        #define EXS_PLATFORM_WINDOWS_32 0
    #else
        #define EXS_PLATFORM_WINDOWS_64 0
        #define EXS_PLATFORM_WINDOWS_32 1
    #endif
    
    #if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
        #define EXS_PLATFORM_WINDOWS_UWP 1
        #define EXS_PLATFORM_WINDOWS_DESKTOP 0
    #else
        #define EXS_PLATFORM_WINDOWS_UWP 0
        #define EXS_PLATFORM_WINDOWS_DESKTOP 1
    #endif
    
    #if defined(_M_ARM) || defined(_M_ARM64)
        #define EXS_PLATFORM_WINDOWS_ARM 1
    #else
        #define EXS_PLATFORM_WINDOWS_ARM 0
    #endif
    
#elif defined(__linux__) || defined(__linux)
    #define EXS_PLATFORM_LINUX 1
    #ifndef EXS_PLATFORM_LINUX
        #define EXS_PLATFORM_LINUX
    #endif
    #define EXS_PLATFORM_UNIX 1
    
    #if defined(__ANDROID__)
        #define EXS_PLATFORM_ANDROID 1
        #define EXS_PLATFORM_LINUX_DESKTOP 0
    #else
        #define EXS_PLATFORM_ANDROID 0
        #define EXS_PLATFORM_LINUX_DESKTOP 1
    #endif
    
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #define EXS_PLATFORM_APPLE 1
    #ifndef EXS_PLATFORM_APPLE
        #define EXS_PLATFORM_APPLE
    #endif
    #define EXS_PLATFORM_UNIX 1
    
    #if TARGET_OS_MAC
        #define EXS_PLATFORM_MACOS 1
        #define EXS_PLATFORM_IOS 0
        #define EXS_PLATFORM_TVOS 0
        #define EXS_PLATFORM_WATCHOS 0
    #elif TARGET_OS_IPHONE
        #define EXS_PLATFORM_MACOS 0
        #define EXS_PLATFORM_IOS 1
        #if TARGET_OS_TV
            #define EXS_PLATFORM_TVOS 1
            #define EXS_PLATFORM_WATCHOS 0
        #elif TARGET_OS_WATCH
            #define EXS_PLATFORM_TVOS 0
            #define EXS_PLATFORM_WATCHOS 1
        #else
            #define EXS_PLATFORM_TVOS 0
            #define EXS_PLATFORM_WATCHOS 0
        #endif
    #endif
    
    #if TARGET_OS_SIMULATOR
        #define EXS_PLATFORM_APPLE_SIMULATOR 1
    #else
        #define EXS_PLATFORM_APPLE_SIMULATOR 0
    #endif
    
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    #define EXS_PLATFORM_FREEBSD 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(__OpenBSD__)
    #define EXS_PLATFORM_OPENBSD 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(__NetBSD__)
    #define EXS_PLATFORM_NETBSD 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(__DragonFly__)
    #define EXS_PLATFORM_DRAGONFLYBSD 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(__sun) || defined(__sun__)
    #define EXS_PLATFORM_SOLARIS 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(_AIX)
    #define EXS_PLATFORM_AIX 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(__hpux)
    #define EXS_PLATFORM_HPUX 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(__CYGWIN__)
    #define EXS_PLATFORM_CYGWIN 1
    #define EXS_PLATFORM_UNIX 1
    
#elif defined(__HAIKU__)
    #define EXS_PLATFORM_HAIKU 1
    
#elif defined(__EMSCRIPTEN__)
    #define EXS_PLATFORM_EMSCRIPTEN 1
    #define EXS_PLATFORM_WEB 1
    
#else
    #error "Unknown or unsupported platform"
#endif

// Architecture Detection -----------------------------------------------------
#if defined(__x86_64__) || defined(_M_X64)
    #define EXS_ARCH_X64 1
    #define EXS_ARCH_64BIT 1
    #define EXS_ARCH_X86 0
    #define EXS_ARCH_32BIT 0
    
#elif defined(__i386__) || defined(_M_IX86)
    #define EXS_ARCH_X64 0
    #define EXS_ARCH_64BIT 0
    #define EXS_ARCH_X86 1
    #define EXS_ARCH_32BIT 1
    
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define EXS_ARCH_ARM64 1
    #define EXS_ARCH_64BIT 1
    #define EXS_ARCH_ARM32 0
    #define EXS_ARCH_ARM 1
    
#elif defined(__arm__) || defined(_M_ARM)
    #define EXS_ARCH_ARM64 0
    #define EXS_ARCH_64BIT 0
    #define EXS_ARCH_ARM32 1
    #define EXS_ARCH_ARM 1
    #if defined(__ARM_ARCH_7A__)
        #define EXS_ARCH_ARMv7 1
    #elif defined(__ARM_ARCH_7S__)
        #define EXS_ARCH_ARMv7S 1
    #endif
    
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
    #define EXS_ARCH_PPC64 1
    #define EXS_ARCH_64BIT 1
    #define EXS_ARCH_PPC 1
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(_ARCH_PPC)
    #define EXS_ARCH_PPC64 0
    #define EXS_ARCH_64BIT 0
    #define EXS_ARCH_PPC 1
    
#elif defined(__mips64)
    #define EXS_ARCH_MIPS64 1
    #define EXS_ARCH_64BIT 1
    #define EXS_ARCH_MIPS 1
    
#elif defined(__mips__)
    #define EXS_ARCH_MIPS64 0
    #define EXS_ARCH_64BIT 0
    #define EXS_ARCH_MIPS 1
    
#elif defined(__riscv) && (__riscv_xlen == 64)
    #define EXS_ARCH_RISCV64 1
    #define EXS_ARCH_64BIT 1
    #define EXS_ARCH_RISCV 1
    
#elif defined(__riscv) && (__riscv_xlen == 32)
    #define EXS_ARCH_RISCV64 0
    #define EXS_ARCH_64BIT 0
    #define EXS_ARCH_RISCV 1
    
#elif defined(__EMSCRIPTEN__)
    #define EXS_ARCH_WASM 1
    #define EXS_ARCH_32BIT 1
    
#else
    #error "Unknown or unsupported architecture"
#endif

// Compiler Detection ---------------------------------------------------------
#if defined(_MSC_VER)
    #define EXS_COMPILER_MSVC 1
    #define EXS_COMPILER_VERSION _MSC_VER
    #define EXS_COMPILER_NAME "Microsoft Visual C++"
    
    #if _MSC_VER >= 1930
        #define EXS_COMPILER_MSVC_2022 1
    #elif _MSC_VER >= 1920
        #define EXS_COMPILER_MSVC_2019 1
    #elif _MSC_VER >= 1910
        #define EXS_COMPILER_MSVC_2017 1
    #elif _MSC_VER >= 1900
        #define EXS_COMPILER_MSVC_2015 1
    #elif _MSC_VER >= 1800
        #define EXS_COMPILER_MSVC_2013 1
    #elif _MSC_VER >= 1700
        #define EXS_COMPILER_MSVC_2012 1
    #elif _MSC_VER >= 1600
        #define EXS_COMPILER_MSVC_2010 1
    #endif
    
#elif defined(__INTEL_COMPILER)
    #define EXS_COMPILER_INTEL 1
    #define EXS_COMPILER_VERSION __INTEL_COMPILER
    #define EXS_COMPILER_NAME "Intel C++ Compiler"
    
#elif defined(__clang__)
    #define EXS_COMPILER_CLANG 1
    #define EXS_COMPILER_VERSION ((__clang_major__ * 10000) + (__clang_minor__ * 100) + __clang_patchlevel__)
    #define EXS_COMPILER_NAME "Clang"
    
    #if defined(__apple_build_version__)
        #define EXS_COMPILER_APPLE_CLANG 1
        #define EXS_COMPILER_APPLE_CLANG_VERSION __apple_build_version__
    #else
        #define EXS_COMPILER_APPLE_CLANG 0
    #endif
    
#elif defined(__GNUC__) || defined(__GNUG__)
    #define EXS_COMPILER_GCC 1
    #define EXS_COMPILER_VERSION ((__GNUC__ * 10000) + (__GNUC_MINOR__ * 100) + __GNUC_PATCHLEVEL__)
    #define EXS_COMPILER_NAME "GNU GCC"
    
#elif defined(__MINGW32__)
    #define EXS_COMPILER_MINGW32 1
    #define EXS_COMPILER_VERSION ((__MINGW32_MAJOR_VERSION * 100) + __MINGW32_MINOR_VERSION)
    #define EXS_COMPILER_NAME "MinGW32"
    
#elif defined(__MINGW64__)
    #define EXS_COMPILER_MINGW64 1
    #define EXS_COMPILER_VERSION ((__MINGW64_VERSION_MAJOR * 100) + __MINGW64_VERSION_MINOR)
    #define EXS_COMPILER_NAME "MinGW64"
    
#else
    #warning "Unknown compiler"
    #define EXS_COMPILER_UNKNOWN 1
    #define EXS_COMPILER_VERSION 0
    #define EXS_COMPILER_NAME "Unknown"
#endif

// C++ Standard Library Detection ---------------------------------------------
#if defined(_LIBCPP_VERSION)
    #define EXS_STDLIB_LIBCPP 1
    #define EXS_STDLIB_VERSION _LIBCPP_VERSION
    #define EXS_STDLIB_NAME "libc++"
    
#elif defined(__GLIBCXX__)
    #define EXS_STDLIB_LIBSTDCPP 1
    #define EXS_STDLIB_VERSION __GLIBCXX__
    #define EXS_STDLIB_NAME "GNU libstdc++"
    
#elif defined(_CPPLIB_VER)
    #define EXS_STDLIB_MSVC_STL 1
    #define EXS_STDLIB_VERSION _CPPLIB_VER
    #define EXS_STDLIB_NAME "Microsoft STL"
    
#else
    #define EXS_STDLIB_UNKNOWN 1
    #define EXS_STDLIB_VERSION 0
    #define EXS_STDLIB_NAME "Unknown"
#endif

// C++ Standard Version Detection ---------------------------------------------
#if defined(_MSVC_LANG)
    #define EXS_CPP_STANDARD _MSVC_LANG
#else
    #define EXS_CPP_STANDARD __cplusplus
#endif

#if EXS_CPP_STANDARD >= 202002L
    #define EXS_CPP20 1
    #define EXS_CPP17 1
    #define EXS_CPP14 1
    #define EXS_CPP11 1
#elif EXS_CPP_STANDARD >= 201703L
    #define EXS_CPP20 0
    #define EXS_CPP17 1
    #define EXS_CPP14 1
    #define EXS_CPP11 1
#elif EXS_CPP_STANDARD >= 201402L
    #define EXS_CPP20 0
    #define EXS_CPP17 0
    #define EXS_CPP14 1
    #define EXS_CPP11 1
#elif EXS_CPP_STANDARD >= 201103L
    #define EXS_CPP20 0
    #define EXS_CPP17 0
    #define EXS_CPP14 0
    #define EXS_CPP11 1
#else
    #define EXS_CPP20 0
    #define EXS_CPP17 0
    #define EXS_CPP14 0
    #define EXS_CPP11 0
#endif

// SIMD Instruction Set Detection ---------------------------------------------
// x86/x64 SIMD
#if defined(__SSE__)
    #define EXS_SIMD_SSE 1
#else
    #define EXS_SIMD_SSE 0
#endif

#if defined(__SSE2__)
    #define EXS_SIMD_SSE2 1
#else
    #define EXS_SIMD_SSE2 0
#endif

#if defined(__SSE3__)
    #define EXS_SIMD_SSE3 1
#else
    #define EXS_SIMD_SSE3 0
#endif

#if defined(__SSSE3__)
    #define EXS_SIMD_SSSE3 1
#else
    #define EXS_SIMD_SSSE3 0
#endif

#if defined(__SSE4_1__)
    #define EXS_SIMD_SSE4_1 1
#else
    #define EXS_SIMD_SSE4_1 0
#endif

#if defined(__SSE4_2__)
    #define EXS_SIMD_SSE4_2 1
#else
    #define EXS_SIMD_SSE4_2 0
#endif

#if defined(__AVX__)
    #define EXS_SIMD_AVX 1
#else
    #define EXS_SIMD_AVX 0
#endif

#if defined(__AVX2__)
    #define EXS_SIMD_AVX2 1
#else
    #define EXS_SIMD_AVX2 0
#endif

#if defined(__AVX512F__)
    #define EXS_SIMD_AVX512F 1
#else
    #define EXS_SIMD_AVX512F 0
#endif

#if defined(__AVX512BW__)
    #define EXS_SIMD_AVX512BW 1
#else
    #define EXS_SIMD_AVX512BW 0
#endif

#if defined(__AVX512VL__)
    #define EXS_SIMD_AVX512VL 1
#else
    #define EXS_SIMD_AVX512VL 0
#endif

#if defined(__AVX512DQ__)
    #define EXS_SIMD_AVX512DQ 1
#else
    #define EXS_SIMD_AVX512DQ 0
#endif

// ARM SIMD
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    #define EXS_SIMD_NEON 1
#else
    #define EXS_SIMD_NEON 0
#endif

#if defined(__ARM_FEATURE_SVE)
    #define EXS_SIMD_SVE 1
#else
    #define EXS_SIMD_SVE 0
#endif

// PowerPC SIMD
#if defined(__ALTIVEC__) || defined(__VEC__)
    #define EXS_SIMD_ALTIVEC 1
#else
    #define EXS_SIMD_ALTIVEC 0
#endif

#if defined(__VSX__)
    #define EXS_SIMD_VSX 1
#else
    #define EXS_SIMD_VSX 0
#endif

// WebAssembly SIMD
#if defined(__wasm_simd128__)
    #define EXS_SIMD_WASM_SIMD128 1
#else
    #define EXS_SIMD_WASM_SIMD128 0
#endif

// Endianness Detection -------------------------------------------------------
#if defined(__BYTE_ORDER__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define EXS_LITTLE_ENDIAN 1
        #define EXS_BIG_ENDIAN 0
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define EXS_LITTLE_ENDIAN 0
        #define EXS_BIG_ENDIAN 1
    #elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
        #define EXS_LITTLE_ENDIAN 0
        #define EXS_BIG_ENDIAN 0
        #define EXS_PDP_ENDIAN 1
    #else
        #error "Unknown byte order"
    #endif
#else
    // Architecture-based fallback
    #if EXS_ARCH_X86 || EXS_ARCH_X64 || EXS_ARCH_ARM || EXS_ARCH_ARM64 || EXS_ARCH_WASM
        #define EXS_LITTLE_ENDIAN 1
        #define EXS_BIG_ENDIAN 0
        #define EXS_PDP_ENDIAN 0
    #elif EXS_ARCH_PPC || EXS_ARCH_MIPS || EXS_ARCH_SPARC
        #define EXS_LITTLE_ENDIAN 0
        #define EXS_BIG_ENDIAN 1
        #define EXS_PDP_ENDIAN 0
    #else
        #error "Cannot determine endianness, please define __BYTE_ORDER__"
    #endif
#endif

// Build Configuration Detection ----------------------------------------------
#if defined(NDEBUG) || defined(_NDEBUG)
    #define EXS_BUILD_RELEASE 1
    #define EXS_BUILD_DEBUG 0
#else
    #define EXS_BUILD_RELEASE 0
    #define EXS_BUILD_DEBUG 1
#endif

#if defined(EXS_SHARED_LIBRARY)
    #if EXS_PLATFORM_WINDOWS
        #ifdef EXS_BUILD_DLL
            #define EXS_API __declspec(dllexport)
        #else
            #define EXS_API __declspec(dllimport)
        #endif
    #else
        #define EXS_API __attribute__((visibility("default")))
    #endif
#else
    #define EXS_API
#endif

// Inline and Optimization Macros ---------------------------------------------
#if EXS_COMPILER_MSVC
    #define EXS_FORCE_INLINE __forceinline
    #define EXS_NOINLINE __declspec(noinline)
    #define EXS_ALWAYS_INLINE __forceinline
    #define EXS_NEVER_INLINE __declspec(noinline)
    #define EXS_RESTRICT __restrict
#else
    #define EXS_FORCE_INLINE inline __attribute__((always_inline))
    #define EXS_NOINLINE __attribute__((noinline))
    #define EXS_ALWAYS_INLINE inline __attribute__((always_inline))
    #define EXS_NEVER_INLINE __attribute__((noinline))
    #define EXS_RESTRICT __restrict
#endif

// Alignment Macros -----------------------------------------------------------
#if EXS_COMPILER_MSVC
    #define EXS_ALIGN(n) __declspec(align(n))
    #define EXS_ALIGN_TYPE(n, type) __declspec(align(n)) type
    #define EXS_ALIGN_OF(type) __alignof(type)
#else
    #define EXS_ALIGN(n) __attribute__((aligned(n)))
    #define EXS_ALIGN_TYPE(n, type) type __attribute__((aligned(n)))
    #define EXS_ALIGN_OF(type) __alignof__(type)
#endif

// Packing Macros -------------------------------------------------------------
#if EXS_COMPILER_MSVC
    #define EXS_PACK_BEGIN __pragma(pack(push, 1))
    #define EXS_PACK_END __pragma(pack(pop))
    #define EXS_PACK(n) __pragma(pack(push, n))
#else
    #define EXS_PACK_BEGIN _Pragma("pack(push, 1)")
    #define EXS_PACK_END _Pragma("pack(pop)")
    #define EXS_PACK(n) _Pragma("pack(push, n)")
#endif

// Branch Prediction Macros ---------------------------------------------------
#if EXS_COMPILER_CLANG || EXS_COMPILER_GCC
    #define EXS_LIKELY(x) __builtin_expect(!!(x), 1)
    #define EXS_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define EXS_EXPECT(x, v) __builtin_expect((x), (v))
#else
    #define EXS_LIKELY(x) (x)
    #define EXS_UNLIKELY(x) (x)
    #define EXS_EXPECT(x, v) (x)
#endif

// Unused Parameter Macro -----------------------------------------------------
#define EXS_UNUSED(x) (void)(x)

// Static Assert --------------------------------------------------------------
#if EXS_CPP17
    #define EXS_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
    #define EXS_STATIC_ASSERT(cond, msg) static_assert(cond, #msg)
#endif

// Compile-time Type Information ----------------------------------------------
namespace Exs {
namespace Internal {
namespace Platform {

struct Exs_PlatformInfo {
    // Platform
    static constexpr bool Windows = EXS_PLATFORM_WINDOWS;
    static constexpr bool Windows64 = EXS_PLATFORM_WINDOWS_64;
    static constexpr bool Windows32 = EXS_PLATFORM_WINDOWS_32;
    static constexpr bool WindowsUWP = EXS_PLATFORM_WINDOWS_UWP;
    static constexpr bool WindowsDesktop = EXS_PLATFORM_WINDOWS_DESKTOP;
    static constexpr bool WindowsARM = EXS_PLATFORM_WINDOWS_ARM;
    
    static constexpr bool Linux = EXS_PLATFORM_LINUX;
    static constexpr bool LinuxDesktop = EXS_PLATFORM_LINUX_DESKPORT;
    static constexpr bool Android = EXS_PLATFORM_ANDROID;
    
    static constexpr bool Apple = EXS_PLATFORM_APPLE;
    static constexpr bool MacOS = EXS_PLATFORM_MACOS;
    static constexpr bool iOS = EXS_PLATFORM_IOS;
    static constexpr bool TvOS = EXS_PLATFORM_TVOS;
    static constexpr bool WatchOS = EXS_PLATFORM_WATCHOS;
    static constexpr bool AppleSimulator = EXS_PLATFORM_APPLE_SIMULATOR;
    
    static constexpr bool Unix = EXS_PLATFORM_UNIX;
    static constexpr bool FreeBSD = EXS_PLATFORM_FREEBSD;
    static constexpr bool OpenBSD = EXS_PLATFORM_OPENBSD;
    static constexpr bool NetBSD = EXS_PLATFORM_NETBSD;
    static constexpr bool DragonFlyBSD = EXS_PLATFORM_DRAGONFLYBSD;
    static constexpr bool Solaris = EXS_PLATFORM_SOLARIS;
    static constexpr bool AIX = EXS_PLATFORM_AIX;
    static constexpr bool HPUX = EXS_PLATFORM_HPUX;
    static constexpr bool Cygwin = EXS_PLATFORM_CYGWIN;
    static constexpr bool Haiku = EXS_PLATFORM_HAIKU;
    static constexpr bool Emscripten = EXS_PLATFORM_EMSCRIPTEN;
    static constexpr bool Web = EXS_PLATFORM_WEB;
    
    // Architecture
    static constexpr bool X64 = EXS_ARCH_X64;
    static constexpr bool X86 = EXS_ARCH_X86;
    static constexpr bool ARM64 = EXS_ARCH_ARM64;
    static constexpr bool ARM32 = EXS_ARCH_ARM32;
    static constexpr bool ARM = EXS_ARCH_ARM;
    static constexpr bool ARMv7 = EXS_ARCH_ARMv7;
    static constexpr bool ARMv7S = EXS_ARCH_ARMv7S;
    static constexpr bool PPC64 = EXS_ARCH_PPC64;
    static constexpr bool PPC = EXS_ARCH_PPC;
    static constexpr bool MIPS64 = EXS_ARCH_MIPS64;
    static constexpr bool MIPS = EXS_ARCH_MIPS;
    static constexpr bool RISC_V64 = EXS_ARCH_RISCV64;
    static constexpr bool RISC_V = EXS_ARCH_RISCV;
    static constexpr bool WASM = EXS_ARCH_WASM;
    
    static constexpr bool Is64Bit = EXS_ARCH_64BIT;
    static constexpr bool Is32Bit = EXS_ARCH_32BIT;
    
    // Compiler
    static constexpr bool MSVC = EXS_COMPILER_MSVC;
    static constexpr bool MSVC2022 = EXS_COMPILER_MSVC_2022;
    static constexpr bool MSVC2019 = EXS_COMPILER_MSVC_2019;
    static constexpr bool MSVC2017 = EXS_COMPILER_MSVC_2017;
    static constexpr bool MSVC2015 = EXS_COMPILER_MSVC_2015;
    static constexpr bool MSVC2013 = EXS_COMPILER_MSVC_2013;
    static constexpr bool MSVC2012 = EXS_COMPILER_MSVC_2012;
    static constexpr bool MSVC2010 = EXS_COMPILER_MSVC_2010;
    
    static constexpr bool Intel = EXS_COMPILER_INTEL;
    static constexpr bool Clang = EXS_COMPILER_CLANG;
    static constexpr bool AppleClang = EXS_COMPILER_APPLE_CLANG;
    static constexpr bool GCC = EXS_COMPILER_GCC;
    static constexpr bool MinGW32 = EXS_COMPILER_MINGW32;
    static constexpr bool MinGW64 = EXS_COMPILER_MINGW64;
    static constexpr bool UnknownCompiler = EXS_COMPILER_UNKNOWN;
    
    // Standard Library
    static constexpr bool LibCpp = EXS_STDLIB_LIBCPP;
    static constexpr bool LibStdCpp = EXS_STDLIB_LIBSTDCPP;
    static constexpr bool MSVC_STL = EXS_STDLIB_MSVC_STL;
    static constexpr bool UnknownStdLib = EXS_STDLIB_UNKNOWN;
    
    // C++ Standard
    static constexpr bool Cpp20 = EXS_CPP20;
    static constexpr bool Cpp17 = EXS_CPP17;
    static constexpr bool Cpp14 = EXS_CPP14;
    static constexpr bool Cpp11 = EXS_CPP11;
    
    // SIMD
    static constexpr bool SSE = EXS_SIMD_SSE;
    static constexpr bool SSE2 = EXS_SIMD_SSE2;
    static constexpr bool SSE3 = EXS_SIMD_SSE3;
    static constexpr bool SSSE3 = EXS_SIMD_SSSE3;
    static constexpr bool SSE4_1 = EXS_SIMD_SSE4_1;
    static constexpr bool SSE4_2 = EXS_SIMD_SSE4_2;
    static constexpr bool AVX = EXS_SIMD_AVX;
    static constexpr bool AVX2 = EXS_SIMD_AVX2;
    static constexpr bool AVX512F = EXS_SIMD_AVX512F;
    static constexpr bool AVX512BW = EXS_SIMD_AVX512BW;
    static constexpr bool AVX512VL = EXS_SIMD_AVX512VL;
    static constexpr bool AVX512DQ = EXS_SIMD_AVX512DQ;
    
    static constexpr bool NEON = EXS_SIMD_NEON;
    static constexpr bool SVE = EXS_SIMD_SVE;
    
    static constexpr bool AltiVec = EXS_SIMD_ALTIVEC;
    static constexpr bool VSX = EXS_SIMD_VSX;
    
    static constexpr bool WasmSimd128 = EXS_SIMD_WASM_SIMD128;
    
    // Endianness
    static constexpr bool LittleEndian = EXS_LITTLE_ENDIAN;
    static constexpr bool BigEndian = EXS_BIG_ENDIAN;
    static constexpr bool PDPEndian = EXS_PDP_ENDIAN;
    
    // Build Configuration
    static constexpr bool Debug = EXS_BUILD_DEBUG;
    static constexpr bool Release = EXS_BUILD_RELEASE;
    
    // Utility Functions
    static const char* Exs_GetPlatformName();
    static const char* Exs_GetArchitectureName();
    static const char* Exs_GetCompilerName();
    static const char* Exs_GetCompilerVersionString();
    static const char* Exs_GetStandardLibraryName();
    static const char* Exs_GetBuildConfigurationName();
    static const char* Exs_GetEndiannessName();
    static const char* Exs_GetCppStandardName();
    
    static bool Exs_IsSimdAvailable();
    static bool Exs_IsBigEndianSystem();
    static bool Exs_IsLittleEndianSystem();
    static bool Exs_IsDebugBuild();
    static bool Exs_IsReleaseBuild();
    
    static void Exs_PrintPlatformInfo();
    static const char* Exs_GetFullPlatformDescription();
};

} // namespace Platform
} // namespace Internal
} // namespace Exs

#endif // EXS_PLATFORM_DETECTION_H
