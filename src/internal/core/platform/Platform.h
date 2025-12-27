// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_PLATFORM_H
#define EXS_PLATFORM_H

#include <cstddef>

namespace Exs_ {
namespace Platform {

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define EXS_PLATFORM_WINDOWS 1
    #define EXS_PLATFORM_NAME "Windows"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #define EXS_PLATFORM_IOS 1
        #define EXS_PLATFORM_NAME "iOS"
    #else
        #define EXS_PLATFORM_MACOS 1
        #define EXS_PLATFORM_NAME "macOS"
    #endif
#elif defined(__ANDROID__)
    #define EXS_PLATFORM_ANDROID 1
    #define EXS_PLATFORM_NAME "Android"
#elif defined(__linux__)
    #define EXS_PLATFORM_LINUX 1
    #define EXS_PLATFORM_NAME "Linux"
#else
    #error "Unsupported platform"
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
    #define EXS_ARCH_X64 1
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    #define EXS_ARCH_X86 1
#elif defined(__arm__) || defined(_M_ARM)
    #define EXS_ARCH_ARM 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define EXS_ARCH_ARM64 1
#else
    #error "Unsupported architecture"
#endif

enum class ExsPlatformType {
    Windows,
    MacOS,
    iOS,
    Android,
    Linux,
    Unknown
};

enum class ExsArchitecture {
    x86,
    x64,
    ARM,
    ARM64,
    Unknown
};

class ExsPlatform {
public:
    static ExsPlatformType ExsGetPlatformType();
    static ExsArchitecture ExsGetArchitecture();
    static const char* ExsGetPlatformName();
    static const char* ExsGetArchitectureName();
    
    static size_t ExsGetPageSize();
    static size_t ExsGetCacheLineSize();
    
    static bool ExsIsLittleEndian();
    static bool ExsIsBigEndian();
    
    static void* ExsAllocatePage(size_t size);
    static void ExsFreePage(void* ptr, size_t size);
    
    static void ExsDebugBreak();
    static void ExsOutputDebugString(const char* message);
    
private:
    ExsPlatform() = delete;
};

} // namespace Platform
} // namespace Exs_

#endif // EXS_PLATFORM_H
