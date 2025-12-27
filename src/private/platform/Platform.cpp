// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#include "Platform.h"
#include <chrono>
#include <iostream>

namespace Exs {
namespace Platform {

bool Exs_Platform::s_Initialized = false;
uint64_t Exs_Platform::s_StartupTime = 0;

void Exs_Platform::Exs_Initialize() {
    if (s_Initialized) {
        return;
    }
    
    // Record startup time
    auto now = std::chrono::steady_clock::now();
    s_StartupTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
    
    s_Initialized = true;
    
    // Internal initialization
    std::cout << "[Exs_Platform] Initialized" << std::endl;
}

void Exs_Platform::Exs_Shutdown() {
    if (!s_Initialized) {
        return;
    }
    
    s_Initialized = false;
    std::cout << "[Exs_Platform] Shutdown" << std::endl;
}

const char* Exs_Platform::Exs_GetPlatformName() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__unix__)
    return "Unix";
#else
    return "Unknown";
#endif
}

const char* Exs_Platform::Exs_GetVersionString() {
    return "Exs_Platform v1.0.0 (Internal)";
}

uint64_t Exs_Platform::Exs_GetInternalTimestamp() {
    auto now = std::chrono::steady_clock::now();
    uint64_t current = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
    return current - s_StartupTime;
}

bool Exs_Platform::Exs_IsInitialized() {
    return s_Initialized;
}

void Exs_Platform::Exs_DumpPlatformInfo() {
    std::cout << "=== Exs Platform Info ===" << std::endl;
    std::cout << "Platform: " << Exs_GetPlatformName() << std::endl;
    std::cout << "Version: " << Exs_GetVersionString() << std::endl;
    std::cout << "Initialized: " << (s_Initialized ? "Yes" : "No") << std::endl;
    std::cout << "Uptime: " << Exs_GetInternalTimestamp() << " ns" << std::endl;
    std::cout << "=========================" << std::endl;
}

} // namespace Platform
} // namespace Exs
