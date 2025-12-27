// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#include "PlatformDetection.h"
#include <iostream>

namespace Exs {
namespace Platform {

bool Exs_PlatformDetection::Exs_IsWindows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsLinux() {
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsMacOS() {
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsUnix() {
#if defined(__unix__) || defined(__unix)
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_Is64Bit() {
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_Is32Bit() {
    return !Exs_Is64Bit();
}

bool Exs_PlatformDetection::Exs_IsX86() {
#if defined(__i386__) || defined(_M_IX86)
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsX64() {
#if defined(__x86_64__) || defined(_M_X64)
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsARM() {
#if defined(__arm__) || defined(_M_ARM)
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsARM64() {
#if defined(__aarch64__) || defined(_M_ARM64)
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsLittleEndian() {
    constexpr uint32_t test = 0x01020304;
    return *reinterpret_cast<const uint8_t*>(&test) == 0x04;
}

bool Exs_PlatformDetection::Exs_IsBigEndian() {
    return !Exs_IsLittleEndian();
}

bool Exs_PlatformDetection::Exs_IsMSVC() {
#ifdef _MSC_VER
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsGCC() {
#ifdef __GNUC__
    #ifndef __clang__
        return true;
    #endif
#endif
    return false;
}

bool Exs_PlatformDetection::Exs_IsClang() {
#ifdef __clang__
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_IsMinGW() {
#ifdef __MINGW32__
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_HasSSE() {
    // Simplified detection
#ifdef __SSE__
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_HasSSE2() {
#ifdef __SSE2__
    return true;
#else
    return false;
#endif
}

bool Exs_PlatformDetection::Exs_HasAVX() {
#ifdef __AVX__
    return true;
#else
    return false;
#endif
}

const char* Exs_PlatformDetection::Exs_GetCompilerName() {
    if (Exs_IsMSVC()) return "MSVC";
    if (Exs_IsClang()) return "Clang";
    if (Exs_IsGCC()) return "GCC";
    if (Exs_IsMinGW()) return "MinGW";
    return "Unknown";
}

const char* Exs_PlatformDetection::Exs_GetCompilerVersion() {
#ifdef _MSC_VER
    #if _MSC_VER == 1900
        return "2015";
    #elif _MSC_VER == 1910
        return "2017";
    #elif _MSC_VER >= 1920 && _MSC_VER < 1930
        return "2019";
    #elif _MSC_VER >= 1930
        return "2022";
    #else
        return "Unknown MSVC";
    #endif
#elif defined(__clang_major__)
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d", 
             __clang_major__, __clang_minor__, __clang_patchlevel__);
    return version;
#elif defined(__GNUC__)
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d", 
             __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    return version;
#else
    return "Unknown";
#endif
}

const char* Exs_PlatformDetection::Exs_GetArchitectureName() {
    if (Exs_IsX64()) return "x64";
    if (Exs_IsX86()) return "x86";
    if (Exs_IsARM64()) return "ARM64";
    if (Exs_IsARM()) return "ARM";
    return "Unknown";
}

const char* Exs_PlatformDetection::Exs_GetEndiannessName() {
    return Exs_IsLittleEndian() ? "Little Endian" : "Big Endian";
}

} // namespace Platform
} // namespace Exs
