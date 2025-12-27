// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once
#include <cstdint>

namespace Exs {
namespace Platform {

class Exs_PlatformDetection {
public:
    // Operating System detection
    static bool Exs_IsWindows();
    static bool Exs_IsLinux();
    static bool Exs_IsMacOS();
    static bool Exs_IsUnix();
    
    // Architecture detection
    static bool Exs_Is64Bit();
    static bool Exs_Is32Bit();
    static bool Exs_IsX86();
    static bool Exs_IsX64();
    static bool Exs_IsARM();
    static bool Exs_IsARM64();
    
    // Endianness detection
    static bool Exs_IsLittleEndian();
    static bool Exs_IsBigEndian();
    
    // Compiler detection
    static bool Exs_IsMSVC();
    static bool Exs_IsGCC();
    static bool Exs_IsClang();
    static bool Exs_IsMinGW();
    
    // Platform features
    static bool Exs_HasSSE();
    static bool Exs_HasSSE2();
    static bool Exs_HasAVX();
    
    // String information
    static const char* Exs_GetCompilerName();
    static const char* Exs_GetCompilerVersion();
    static const char* Exs_GetArchitectureName();
    static const char* Exs_GetEndiannessName();
};

} // namespace Platform
} // namespace Exs
