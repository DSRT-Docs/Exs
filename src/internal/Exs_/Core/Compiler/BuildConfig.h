// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_BUILD_CONFIG_H
#define EXS_BUILD_CONFIG_H

#include "../Platform/Platform.h"

// Build mode configuration
#if !defined(EXS_DEBUG) && !defined(EXS_RELEASE) && !defined(EXS_SHIPPING)
    #ifdef _DEBUG
        #define EXS_DEBUG 1
    #else
        #define EXS_RELEASE 1
    #endif
#endif

// Feature flags
#ifndef EXS_FEATURE_ASSERTIONS
    #if EXS_DEBUG
        #define EXS_FEATURE_ASSERTIONS 1
    #else
        #define EXS_FEATURE_ASSERTIONS 0
    #endif
#endif

#ifndef EXS_FEATURE_LOGGING
    #define EXS_FEATURE_LOGGING 1
#endif

#ifndef EXS_FEATURE_PROFILING
    #if EXS_DEBUG || EXS_RELEASE
        #define EXS_FEATURE_PROFILING 1
    #else
        #define EXS_FEATURE_PROFILING 0
    #endif
#endif

#ifndef EXS_FEATURE_MEMORY_TRACKING
    #if EXS_DEBUG
        #define EXS_FEATURE_MEMORY_TRACKING 1
    #else
        #define EXS_FEATURE_MEMORY_TRACKING 0
    #endif
#endif

#ifndef EXS_FEATURE_THREAD_SAFETY
    #define EXS_FEATURE_THREAD_SAFETY 1
#endif

// Version information
#define EXS_VERSION_MAJOR 0
#define EXS_VERSION_MINOR 1
#define EXS_VERSION_PATCH 0
#define EXS_VERSION_BUILD 0

#define EXS_VERSION_STRING "0.1.0"

// API export macros
#ifndef EXS_API_EXPORT
    #if EXS_PLATFORM_WINDOWS
        #ifdef EXS_BUILDING_DLL
            #define EXS_API_EXPORT __declspec(dllexport)
        #else
            #define EXS_API_EXPORT __declspec(dllimport)
        #endif
    #else
        #define EXS_API_EXPORT __attribute__((visibility("default")))
    #endif
#endif

#ifndef EXS_API_IMPORT
    #if EXS_PLATFORM_WINDOWS
        #define EXS_API_IMPORT __declspec(dllimport)
    #else
        #define EXS_API_IMPORT
    #endif
#endif

#ifndef EXS_API_HIDDEN
    #if EXS_PLATFORM_WINDOWS
        #define EXS_API_HIDDEN
    #else
        #define EXS_API_HIDDEN __attribute__((visibility("hidden")))
    #endif
#endif

namespace Exs_ {
namespace Build {

class ExsBuildConfig {
public:
    struct ExsVersion {
        int major;
        int minor;
        int patch;
        int build;
        const char* string;
    };
    
    static ExsVersion ExsGetVersion();
    
    static bool ExsIsDebug();
    static bool ExsIsRelease();
    static bool ExsIsShipping();
    
    static bool ExsHasAssertions();
    static bool ExsHasLogging();
    static bool ExsHasProfiling();
    static bool ExsHasMemoryTracking();
    static bool ExsHasThreadSafety();
    
    static bool ExsIsFeatureEnabled(const char* feature);
    static void ExsEnableFeature(const char* feature);
    static void ExsDisableFeature(const char* feature);
    
    static const char* ExsGetBuildDate();
    static const char* ExsGetBuildTime();
    static const char* ExsGetBuildMachine();
    static const char* ExsGetBuildUser();
    
    static uint64_t ExsGetBuildId();
    
private:
    ExsBuildConfig() = delete;
};

} // namespace Build
} // namespace Exs_

#endif // EXS_BUILD_CONFIG_H
