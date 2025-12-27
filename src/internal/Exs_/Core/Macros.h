// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_MACROS_H
#define EXS_MACROS_H

// Compiler feature detection
#ifndef EXS_CPP_STANDARD
    #if defined(__cplusplus)
        #if __cplusplus >= 202002L
            #define EXS_CPP_STANDARD 20
        #elif __cplusplus >= 201703L
            #define EXS_CPP_STANDARD 17
        #elif __cplusplus >= 201402L
            #define EXS_CPP_STANDARD 14
        #elif __cplusplus >= 201103L
            #define EXS_CPP_STANDARD 11
        #else
            #define EXS_CPP_STANDARD 98
        #endif
    #else
        #define EXS_CPP_STANDARD 0
    #endif
#endif

// Feature test macros
#ifndef EXS_HAS_CPP_11
    #define EXS_HAS_CPP_11 (EXS_CPP_STANDARD >= 11)
#endif

#ifndef EXS_HAS_CPP_14
    #define EXS_HAS_CPP_14 (EXS_CPP_STANDARD >= 14)
#endif

#ifndef EXS_HAS_CPP_17
    #define EXS_HAS_CPP_17 (EXS_CPP_STANDARD >= 17)
#endif

#ifndef EXS_HAS_CPP_20
    #define EXS_HAS_CPP_20 (EXS_CPP_STANDARD >= 20)
#endif

// Inline macro
#if EXS_HAS_CPP_17
    #define EXS_INLINE_VAR inline
#else
    #define EXS_INLINE_VAR
#endif

// Fallthrough macro
#ifndef EXS_FALLTHROUGH
    #if EXS_HAS_CPP_17
        #define EXS_FALLTHROUGH [[fallthrough]]
    #elif defined(__clang__)
        #define EXS_FALLTHROUGH [[clang::fallthrough]]
    #elif defined(__GNUC__) && __GNUC__ >= 7
        #define EXS_FALLTHROUGH __attribute__((fallthrough))
    #else
        #define EXS_FALLTHROUGH
    #endif
#endif

// Likely/unlikely macros (already defined in Compiler.h)
#ifndef EXS_LIKELY
    #define EXS_LIKELY(x) (x)
#endif

#ifndef EXS_UNLIKELY
    #define EXS_UNLIKELY(x) (x)
#endif

// Deprecated macro
#ifndef EXS_DEPRECATED
    #define EXS_DEPRECATED(msg) [[deprecated(msg)]]
#endif

// Nodiscard macro
#ifndef EXS_NODISCARD
    #define EXS_NODISCARD [[nodiscard]]
#endif

// No return macro
#ifndef EXS_NO_RETURN
    #define EXS_NO_RETURN [[noreturn]]
#endif

// Maybe unused macro
#ifndef EXS_MAYBE_UNUSED
    #define EXS_MAYBE_UNUSED [[maybe_unused]]
#endif

// Constinit macro
#if EXS_HAS_CPP_20
    #define EXS_CONSTINIT constinit
#else
    #define EXS_CONSTINIT
#endif

// Consteval macro
#if EXS_HAS_CPP_20
    #define EXS_CONSTEVAL consteval
#else
    #define EXS_CONSTEVAL constexpr
#endif

// Constexpr macro
#ifndef EXS_CONSTEXPR
    #if EXS_HAS_CPP_14
        #define EXS_CONSTEXPR constexpr
    #else
        #define EXS_CONSTEXPR const
    #endif
#endif

// Static assert macro
#ifndef EXS_STATIC_ASSERT
    #if EXS_HAS_CPP_17
        #define EXS_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
    #else
        #define EXS_STATIC_ASSERT(cond, msg) static_assert(cond, #msg)
    #endif
#endif

// Force inline macro
#ifndef EXS_FORCE_INLINE
    #if defined(_MSC_VER)
        #define EXS_FORCE_INLINE __forceinline
    #else
        #define EXS_FORCE_INLINE __attribute__((always_inline)) inline
    #endif
#endif

// No inline macro
#ifndef EXS_NO_INLINE
    #if defined(_MSC_VER)
        #define EXS_NO_INLINE __declspec(noinline)
    #else
        #define EXS_NO_INLINE __attribute__((noinline))
    #endif
#endif

// Restrict macro
#ifndef EXS_RESTRICT
    #if defined(_MSC_VER)
        #define EXS_RESTRICT __restrict
    #else
        #define EXS_RESTRICT __restrict__
    #endif
#endif

// Align macro
#ifndef EXS_ALIGNAS
    #define EXS_ALIGNAS(align) alignas(align)
#endif

#ifndef EXS_ALIGNOF
    #define EXS_ALIGNOF(type) alignof(type)
#endif

// Pack macro
#ifndef EXS_PACK
    #if defined(_MSC_VER)
        #define EXS_PACK(push, n) __pragma(pack(push, n))
        #define EXS_PACK_POP() __pragma(pack(pop))
    #else
        #define EXS_PACK(push, n) _Pragma("pack(push, n)")
        #define EXS_PACK_POP() _Pragma("pack(pop)")
    #endif
#endif

// Section macro
#ifndef EXS_SECTION
    #if defined(_MSC_VER)
        #define EXS_SECTION(name) __declspec(allocate(name))
    #else
        #define EXS_SECTION(name) __attribute__((section(name)))
    #endif
#endif

// Weak symbol macro
#ifndef EXS_WEAK
    #if defined(_MSC_VER)
        #define EXS_WEAK
    #else
        #define EXS_WEAK __attribute__((weak))
    #endif
#endif

// Constructor/destructor priority
#ifndef EXS_CONSTRUCTOR
    #if defined(_MSC_VER)
        #define EXS_CONSTRUCTOR(priority)
    #else
        #define EXS_CONSTRUCTOR(priority) __attribute__((constructor(priority)))
    #endif
#endif

#ifndef EXS_DESTRUCTOR
    #if defined(_MSC_VER)
        #define EXS_DESTRUCTOR(priority)
    #else
        #define EXS_DESTRUCTOR(priority) __attribute__((destructor(priority)))
    #endif
#endif

// Format string checking
#ifndef EXS_FORMAT
    #if defined(__clang__) || defined(__GNUC__)
        #define EXS_FORMAT(type, fmtIndex, argIndex) \
            __attribute__((format(type, fmtIndex, argIndex)))
    #else
        #define EXS_FORMAT(type, fmtIndex, argIndex)
    #endif
#endif

// Pure/const functions
#ifndef EXS_PURE
    #if defined(__clang__) || defined(__GNUC__)
        #define EXS_PURE __attribute__((pure))
    #else
        #define EXS_PURE
    #endif
#endif

#ifndef EXS_CONST
    #if defined(__clang__) || defined(__GNUC__)
        #define EXS_CONST __attribute__((const))
    #else
        #define EXS_CONST
    #endif
#endif

// Hot/cold functions
#ifndef EXS_HOT
    #if defined(__clang__) || defined(__GNUC__)
        #define EXS_HOT __attribute__((hot))
    #else
        #define EXS_HOT
    #endif
#endif

#ifndef EXS_COLD
    #if defined(__clang__) || defined(__GNUC__)
        #define EXS_COLD __attribute__((cold))
    #else
        #define EXS_COLD
    #endif
#endif

// Unused parameter macro
#ifndef EXS_UNUSED
    #define EXS_UNUSED(x) (void)(x)
#endif

// Stringify macro
#define EXS_STRINGIFY(x) #x
#define EXS_STRINGIFY_EXPAND(x) EXS_STRINGIFY(x)

// Concat macro
#define EXS_CONCAT_IMPL(x, y) x##y
#define EXS_CONCAT(x, y) EXS_CONCAT_IMPL(x, y)

// Unique identifier macro
#define EXS_UNIQUE_NAME(prefix) EXS_CONCAT(prefix, __LINE__)

// Count arguments macro
#define EXS_ARG_COUNT(...) \
    EXS_ARG_COUNT_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define EXS_ARG_COUNT_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N

// Overload macro
#define EXS_OVERLOAD(macro, ...) \
    EXS_CONCAT(macro, EXS_ARG_COUNT(__VA_ARGS__))(__VA_ARGS__)

// Type checking macro
#define EXS_TYPE_CHECK(type, expr) \
    static_assert(std::is_same<decltype(expr), type>::value, \
                  "Type mismatch: expected " #type)

// Size checking macro
#define EXS_SIZE_CHECK(type, size) \
    static_assert(sizeof(type) == (size), \
                  "Size mismatch for " #type ": expected " #size)

// Align checking macro
#define EXS_ALIGN_CHECK(type, align) \
    static_assert(alignof(type) == (align), \
                  "Alignment mismatch for " #type ": expected " #align)

// Offsetof macro with type safety
#define EXS_OFFSETOF(type, member) \
    (reinterpret_cast<size_t>(&reinterpret_cast<char&>( \
        reinterpret_cast<type*>(0)->member)))

// Containerof macro
#define EXS_CONTAINEROF(ptr, type, member) \
    reinterpret_cast<type*>( \
        reinterpret_cast<char*>(ptr) - offsetof(type, member))

// Min/max macros (type-safe versions)
#define EXS_MIN(a, b) ((a) < (b) ? (a) : (b))
#define EXS_MAX(a, b) ((a) > (b) ? (a) : (b))

// Clamp macro
#define EXS_CLAMP(val, min, max) EXS_MIN(EXS_MAX(val, min), max)

// Abs macro
#define EXS_ABS(x) ((x) < 0 ? -(x) : (x))

// Sign macro
#define EXS_SIGN(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

// Round macros
#define EXS_ROUND(x) ((x) >= 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define EXS_FLOOR(x) ((int)(x))
#define EXS_CEIL(x) (((int)(x)) + (((x) > (int)(x)) ? 1 : 0))

// Power of two macros
#define EXS_IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))
#define EXS_NEXT_POWER_OF_TWO(x) \
    (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))

// Bit manipulation macros
#define EXS_BIT(n) (1U << (n))
#define EXS_BIT_SET(x, n) ((x) |= EXS_BIT(n))
#define EXS_BIT_CLEAR(x, n) ((x) &= ~EXS_BIT(n))
#define EXS_BIT_TOGGLE(x, n) ((x) ^= EXS_BIT(n))
#define EXS_BIT_CHECK(x, n) (((x) & EXS_BIT(n)) != 0)

// Byte swapping macros
#define EXS_SWAP16(x) \
    ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))

#define EXS_SWAP32(x) \
    ((((x) & 0xFF000000) >> 24) | \
     (((x) & 0x00FF0000) >> 8)  | \
     (((x) & 0x0000FF00) << 8)  | \
     (((x) & 0x000000FF) << 24))

#define EXS_SWAP64(x) \
    ((((x) & 0xFF00000000000000ULL) >> 56) | \
     (((x) & 0x00FF000000000000ULL) >> 40) | \
     (((x) & 0x0000FF0000000000ULL) >> 24) | \
     (((x) & 0x000000FF00000000ULL) >> 8)  | \
     (((x) & 0x00000000FF000000ULL) << 8)  | \
     (((x) & 0x0000000000FF0000ULL) << 24) | \
     (((x) & 0x000000000000FF00ULL) << 40) | \
     (((x) & 0x00000000000000FFULL) << 56))

// Debug break macro
#ifndef EXS_DEBUG_BREAK
    #if defined(_MSC_VER)
        #define EXS_DEBUG_BREAK __debugbreak()
    #elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
        #define EXS_DEBUG_BREAK __asm__ volatile("int $0x03")
    #elif defined(__GNUC__) && defined(__arm__)
        #define EXS_DEBUG_BREAK __asm__ volatile(".inst 0xe7f001f0")
    #elif defined(__GNUC__) && defined(__aarch64__)
        #define EXS_DEBUG_BREAK __asm__ volatile(".inst 0xd4200000")
    #else
        #include <signal.h>
        #define EXS_DEBUG_BREAK raise(SIGTRAP)
    #endif
#endif

// Compile-time assert (deprecated, use static_assert)
#ifndef EXS_COMPILE_TIME_ASSERT
    #define EXS_COMPILE_TIME_ASSERT(cond, msg) \
        typedef char EXS_CONCAT(compile_time_assert_, __LINE__)[(cond) ? 1 : -1]
#endif

// Suppress warnings macro
#ifndef EXS_SUPPRESS_WARNING
    #if defined(_MSC_VER)
        #define EXS_SUPPRESS_WARNING(warningNumber) \
            __pragma(warning(push)) \
            __pragma(warning(disable: warningNumber))
        #define EXS_SUPPRESS_WARNING_POP() __pragma(warning(pop))
    #elif defined(__clang__) || defined(__GNUC__)
        #define EXS_SUPPRESS_WARNING(warning) \
            _Pragma("GCC diagnostic push") \
            _Pragma("GCC diagnostic ignored \"#warning\"")
        #define EXS_SUPPRESS_WARNING_POP() _Pragma("GCC diagnostic pop")
    #else
        #define EXS_SUPPRESS_WARNING(warning)
        #define EXS_SUPPRESS_WARNING_POP()
    #endif
#endif

// Likely/unlikely for conditions
#define EXS_IF_LIKELY(cond) if (EXS_LIKELY(cond))
#define EXS_IF_UNLIKELY(cond) if (EXS_UNLIKELY(cond))

// Loop unrolling hint
#ifndef EXS_LOOP_UNROLL
    #if defined(__clang__) || defined(__GNUC__)
        #define EXS_LOOP_UNROLL(n) _Pragma("GCC unroll " #n)
    #else
        #define EXS_LOOP_UNROLL(n)
    #endif
#endif

// Optimization barrier
#ifndef EXS_OPTIMIZATION_BARRIER
    #if defined(__GNUC__)
        #define EXS_OPTIMIZATION_BARRIER() __asm__ volatile("" ::: "memory")
    #else
        #define EXS_OPTIMIZATION_BARRIER()
    #endif
#endif

// Prefetch macro
#ifndef EXS_PREFETCH
    #if defined(__clang__) || defined(__GNUC__)
        #define EXS_PREFETCH(addr, rw, locality) \
            __builtin_prefetch(addr, rw, locality)
    #else
        #define EXS_PREFETCH(addr, rw, locality)
    #endif
#endif

// Cache line size (common values)
#ifndef EXS_CACHE_LINE_SIZE
    #if defined(__i386__) || defined(__x86_64__)
        #define EXS_CACHE_LINE_SIZE 64
    #elif defined(__arm__) || defined(__aarch64__)
        #define EXS_CACHE_LINE_SIZE 64
    #else
        #define EXS_CACHE_LINE_SIZE 64
    #endif
#endif

// Cache line alignment
#define EXS_CACHE_ALIGN EXS_ALIGNAS(EXS_CACHE_LINE_SIZE)

// Page size (common values)
#ifndef EXS_PAGE_SIZE
    #if defined(__i386__) || defined(__x86_64__)
        #define EXS_PAGE_SIZE 4096
    #elif defined(__arm__) || defined(__aarch64__)
        #define EXS_PAGE_SIZE 4096
    #else
        #define EXS_PAGE_SIZE 4096
    #endif
#endif

// Page alignment
#define EXS_PAGE_ALIGN EXS_ALIGNAS(EXS_PAGE_SIZE)

#endif // EXS_MACROS_H
