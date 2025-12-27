// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");


#pragma once
#include <cstdint>
#include <string>

namespace Exs {
namespace Platform {

class Exs_SystemInfo {
private:
    static std::string s_OSName;
    static std::string s_OSVersion;
    static std::string s_KernelVersion;
    static uint64_t s_SystemUptime;
    static uint32_t s_ProcessCount;
    static std::string s_Hostname;
    
    static bool s_Initialized;
    
    static void Exs_DetectSystemInfo();
    
public:
    static void Exs_Initialize();
    static void Exs_Update();
    
    // System information
    static std::string Exs_GetOSName();
    static std::string Exs_GetOSVersion();
    static std::string Exs_GetKernelVersion();
    static std::string Exs_GetHostname();
    
    // System metrics
    static uint64_t Exs_GetSystemUptime(); // in seconds
    static uint32_t Exs_GetProcessCount();
    static uint32_t Exs_GetProcessorCount();
    
    // System capabilities
    static uint64_t Exs_GetPageSize();
    static bool Exs_IsMultiProcessor();
    
    // Debug functions
    static void Exs_DumpSystemInfo();
};

} // namespace Platform
} // namespace Exs
