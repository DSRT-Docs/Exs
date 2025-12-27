// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_ASSERT_H
#define EXS_ASSERT_H

#include "../Compiler/BuildConfig.h"
#include "Debug.h"

namespace Exs_ {
namespace Debug {

class ExsAssert {
public:
    enum class ExsAssertLevel {
        Debug,
        Release,
        Always
    };
    
    struct ExsAssertContext {
        const char* expression;
        const char* message;
        const char* file;
        int line;
        ExsAssertLevel level;
    };
    
    using ExsAssertHandler = bool (*)(const ExsAssertContext& context);
    
    static void ExsSetAssertHandler(ExsAssertHandler handler);
    static ExsAssertHandler ExsGetAssertHandler();
    static void ExsRestoreDefaultAssertHandler();
    
    static bool ExsDefaultAssertHandler(const ExsAssertContext& context);
    
    static void ExsHandleAssert(
        const char* expression,
        const char* message,
        const char* file,
        int line,
        ExsAssertLevel level = ExsAssertLevel::Debug
    );
    
    static void ExsEnableAsserts(ExsAssertLevel level);
    static void ExsDisableAsserts();
    static bool ExsAreAssertsEnabled(ExsAssertLevel level);
    
    static const char* ExsAssertLevelToString(ExsAssertLevel level);
    
private:
    ExsAssert() = delete;
    
    static ExsAssertHandler currentHandler;
    static ExsAssertLevel assertLevel;
};

// Assert macros
#if EXS_FEATURE_ASSERTIONS

    #define EXS_ASSERT(expr) \
        do { \
            if (EXS_UNLIKELY(!(expr))) { \
                Exs_::Debug::ExsAssert::ExsHandleAssert( \
                    #expr, \
                    nullptr, \
                    __FILE__, \
                    __LINE__, \
                    Exs_::Debug::ExsAssert::ExsAssertLevel::Debug \
                ); \
            } \
        } while(0)
    
    #define EXS_ASSERT_MSG(expr, msg) \
        do { \
            if (EXS_UNLIKELY(!(expr))) { \
                Exs_::Debug::ExsAssert::ExsHandleAssert( \
                    #expr, \
                    msg, \
                    __FILE__, \
                    __LINE__, \
                    Exs_::Debug::ExsAssert::ExsAssertLevel::Debug \
                ); \
            } \
        } while(0)
    
    #define EXS_ASSERT_FMT(expr, fmt, ...) \
        do { \
            if (EXS_UNLIKELY(!(expr))) { \
                char buffer[256]; \
                snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
                Exs_::Debug::ExsAssert::ExsHandleAssert( \
                    #expr, \
                    buffer, \
                    __FILE__, \
                    __LINE__, \
                    Exs_::Debug::ExsAssert::ExsAssertLevel::Debug \
                ); \
            } \
        } while(0)
    
    #define EXS_CHECK(expr) EXS_ASSERT(expr)
    #define EXS_CHECK_MSG(expr, msg) EXS_ASSERT_MSG(expr, msg)
    #define EXS_CHECK_FMT(expr, fmt, ...) EXS_ASSERT_FMT(expr, fmt, ##__VA_ARGS__)
    
    #define EXS_VERIFY(expr) \
        do { \
            if (EXS_UNLIKELY(!(expr))) { \
                Exs_::Debug::ExsAssert::ExsHandleAssert( \
                    #expr, \
                    nullptr, \
                    __FILE__, \
                    __LINE__, \
                    Exs_::Debug::ExsAssert::ExsAssertLevel::Release \
                ); \
            } \
        } while(0)
    
    #define EXS_VERIFY_MSG(expr, msg) \
        do { \
            if (EXS_UNLIKELY(!(expr))) { \
                Exs_::Debug::ExsAssert::ExsHandleAssert( \
                    #expr, \
                    msg, \
                    __FILE__, \
                    __LINE__, \
                    Exs_::Debug::ExsAssert::ExsAssertLevel::Release \
                ); \
            } \
        } while(0)
    
    #define EXS_VERIFY_FMT(expr, fmt, ...) \
        do { \
            if (EXS_UNLIKELY(!(expr))) { \
                char buffer[256]; \
                snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__); \
                Exs_::Debug::ExsAssert::ExsHandleAssert( \
                    #expr, \
                    buffer, \
                    __FILE__, \
                    __LINE__, \
                    Exs_::Debug::ExsAssert::ExsAssertLevel::Release \
                ); \
            } \
        } while(0)

#else

    #define EXS_ASSERT(expr) ((void)0)
    #define EXS_ASSERT_MSG(expr, msg) ((void)0)
    #define EXS_ASSERT_FMT(expr, fmt, ...) ((void)0)
    
    #define EXS_CHECK(expr) (expr)
    #define EXS_CHECK_MSG(expr, msg) (expr)
    #define EXS_CHECK_FMT(expr, fmt, ...) (expr)
    
    #define EXS_VERIFY(expr) (expr)
    #define EXS_VERIFY_MSG(expr, msg) (expr)
    #define EXS_VERIFY_FMT(expr, fmt, ...) (expr)

#endif

// Always active asserts
#define EXS_ASSERT_ALWAYS(expr) \
    do { \
        if (EXS_UNLIKELY(!(expr))) { \
            Exs_::Debug::ExsAssert::ExsHandleAssert( \
                #expr, \
                nullptr, \
                __FILE__, \
                __LINE__, \
                Exs_::Debug::ExsAssert::ExsAssertLevel::Always \
            ); \
        } \
    } while(0)

#define EXS_ASSERT_ALWAYS_MSG(expr, msg) \
    do { \
        if (EXS_UNLIKELY(!(expr))) { \
            Exs_::Debug::ExsAssert::ExsHandleAssert( \
                #expr, \
                msg, \
                __FILE__, \
                __LINE__, \
                Exs_::Debug::ExsAssert::ExsAssertLevel::Always \
            ); \
        } \
    } while(0)

// Static asserts
#define EXS_STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#define EXS_STATIC_ASSERT_EQ(a, b, msg) static_assert((a) == (b), msg)
#define EXS_STATIC_ASSERT_NE(a, b, msg) static_assert((a) != (b), msg)
#define EXS_STATIC_ASSERT_LT(a, b, msg) static_assert((a) < (b), msg)
#define EXS_STATIC_ASSERT_LE(a, b, msg) static_assert((a) <= (b), msg)
#define EXS_STATIC_ASSERT_GT(a, b, msg) static_assert((a) > (b), msg)
#define EXS_STATIC_ASSERT_GE(a, b, msg) static_assert((a) >= (b), msg)

} // namespace Debug
} // namespace Exs_

#endif // EXS_ASSERT_H
