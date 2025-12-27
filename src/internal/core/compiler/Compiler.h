// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_COMPILER_H
#define EXS_COMPILER_H

// Compiler detection
#if defined(__clang__)
    #define EXS_COMPILER_CLANG 1
    #define EXS_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
    #define EXS_COMPILER_GCC 1
    #define EXS_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
    #define EXS_COMPILER_MSVC 1
    #define EXS_COMPILER_VERSION _MSC_VER
#else
    #define EXS_COMPILER_UNKNOWN 1
#endif

// Compiler feature macros
#ifndef EXS_FORCE_INLINE
    #if EXS_COMPILER_MSVC
        #define EXS_FORCE_INLINE __forceinline
    #else
        #define EXS_FORCE_INLINE __attribute__((always_inline)) inline
    #endif
#endif

#ifndef EXS_NO_INLINE
    #if EXS_COMPILER_MSVC
        #define EXS_NO_INLINE __declspec(noinline)
    #else
        #define EXS_NO_INLINE __attribute__((noinline))
    #endif
#endif

#ifndef EXS_NO_RETURN
    #if EXS_COMPILER_MSVC
        #define EXS_NO_RETURN __declspec(noreturn)
    #else
        #define EXS_NO_RETURN __attribute__((noreturn))
    #endif
#endif

#ifndef EXS_RESTRICT
    #if EXS_COMPILER_MSVC
        #define EXS_RESTRICT __restrict
    #else
        #define EXS_RESTRICT __restrict__
    #endif
#endif

#ifndef EXS_ALIGNAS
    #define EXS_ALIGNAS(alignment) alignas(alignment)
#endif

#ifndef EXS_ALIGNOF
    #define EXS_ALIGNOF(type) alignof(type)
#endif

// Branch prediction
#ifndef EXS_LIKELY
    #if EXS_COMPILER_CLANG || EXS_COMPILER_GCC
        #define EXS_LIKELY(x) __builtin_expect(!!(x), 1)
        #define EXS_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #else
        #define EXS_LIKELY(x) (x)
        #define EXS_UNLIKELY(x) (x)
    #endif
#endif

// Deprecation
#ifndef EXS_DEPRECATED
    #if EXS_COMPILER_MSVC
        #define EXS_DEPRECATED(msg) __declspec(deprecated(msg))
    #else
        #define EXS_DEPRECATED(msg) __attribute__((deprecated(msg)))
    #endif
#endif

// Nodiscard
#ifndef EXS_NODISCARD
    #if defined(__cplusplus) && __cplusplus >= 201703L
        #define EXS_NODISCARD [[nodiscard]]
    #elif EXS_COMPILER_MSVC && _MSC_VER >= 1911
        #define EXS_NODISCARD [[nodiscard]]
    #else
        #define EXS_NODISCARD
    #endif
#endif

// No unique address
#ifndef EXS_NO_UNIQUE_ADDRESS
    #if defined(__cplusplus) && __cplusplus >= 202002L
        #define EXS_NO_UNIQUE_ADDRESS [[no_unique_address]]
    #elif EXS_COMPILER_MSVC && _MSC_VER >= 1928
        #define EXS_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
    #else
        #define EXS_NO_UNIQUE_ADDRESS
    #endif
#endif

namespace Exs_ {
namespace Compiler {

class ExsCompiler {
public:
    static const char* ExsGetCompilerName();
    static const char* ExsGetCompilerVersionString();
    static int ExsGetCompilerVersion();
    
    static bool ExsHasFeature(const char* feature);
    
    static bool ExsSupportsSSE();
    static bool ExsSupportsSSE2();
    static bool ExsSupportsSSE3();
    static bool ExsSupportsSSE4();
    static bool ExsSupportsAVX();
    static bool ExsSupportsAVX2();
    
    static bool ExsSupportsNEON();
    static bool ExsSupportsFMA();
    
    static size_t ExsGetMaxAlignment();
    static size_t ExsGetPreferredAlignment();
    
    static void ExsSetFloatingPointMode();
    static void ExsEnableFastMath();
    static void ExsDisableFastMath();
    
    static void ExsPrefetch(const void* ptr);
    
private:
    ExsCompiler() = delete;
};

} // namespace Compiler
} // namespace Exs_

#endif // EXS_COMPILER_H
