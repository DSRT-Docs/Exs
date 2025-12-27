// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once
#include <cstdint>
#include <string>

namespace Exs {
namespace Platform {

struct Exs_CPUFeatures {
    bool Exs_SSE;
    bool Exs_SSE2;
    bool Exs_SSE3;
    bool Exs_SSSE3;
    bool Exs_SSE41;
    bool Exs_SSE42;
    bool Exs_AVX;
    bool Exs_AVX2;
    bool Exs_FMA;
    bool Exs_FMA3;
    
    bool Exs_MMX;
    bool Exs_3DNOW;
    
    bool Exs_HyperThreading;
    bool Exs_Virtualization;
};

class Exs_CPUInfo {
private:
    static std::string s_Vendor;
    static std::string s_Brand;
    static uint32_t s_CoreCount;
    static uint32_t s_ThreadCount;
    static uint64_t s_Frequency;
    static Exs_CPUFeatures s_Features;
    
    static bool s_Initialized;
    
    static void Exs_DetectCPUInfo();
    
public:
    static void Exs_Initialize();
    
    // CPU identification
    static std::string Exs_GetVendor();
    static std::string Exs_GetBrand();
    
    // CPU topology
    static uint32_t Exs_GetCoreCount();
    static uint32_t Exs_GetThreadCount();
    static uint32_t Exs_GetPhysicalCoreCount();
    static uint32_t Exs_GetLogicalCoreCount();
    
    // CPU frequency
    static uint64_t Exs_GetFrequency(); // in MHz
    static uint64_t Exs_GetMaxFrequency();
    static uint64_t Exs_GetMinFrequency();
    
    // CPU features
    static const Exs_CPUFeatures& Exs_GetFeatures();
    static bool Exs_HasFeature(const std::string& feature);
    
    // CPU cache
    static uint64_t Exs_GetCacheSizeL1();
    static uint64_t Exs_GetCacheSizeL2();
    static uint64_t Exs_GetCacheSizeL3();
    
    // Debug functions
    static void Exs_DumpCPUInfo();
};

} // namespace Platform
} // namespace Exs
