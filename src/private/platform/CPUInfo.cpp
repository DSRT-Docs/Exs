#include "CPUInfo.h"
#include <iostream>

#ifdef _WIN32
    #include <intrin.h>
    #include <windows.h>
#else
    #include <cpuid.h>
#endif

namespace Exs {
namespace Platform {

std::string Exs_CPUInfo::s_Vendor;
std::string Exs_CPUInfo::s_Brand;
uint32_t Exs_CPUInfo::s_CoreCount = 0;
uint32_t Exs_CPUInfo::s_ThreadCount = 0;
uint64_t Exs_CPUInfo::s_Frequency = 0;
Exs_CPUFeatures Exs_CPUInfo::s_Features;
bool Exs_CPUInfo::s_Initialized = false;

void Exs_CPUInfo::Exs_Initialize() {
    if (s_Initialized) return;
    
    Exs_DetectCPUInfo();
    s_Initialized = true;
}

void Exs_CPUInfo::Exs_DetectCPUInfo() {
    s_Vendor = "Unknown";
    s_Brand = "Unknown CPU";
    s_CoreCount = 1;
    s_ThreadCount = 1;
    s_Frequency = 0;
    
    // Reset features
    s_Features = Exs_CPUFeatures{};
    
#ifdef _WIN32
    // Get CPU info using Windows API
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    s_CoreCount = sysInfo.dwNumberOfProcessors;
    s_ThreadCount = s_CoreCount;
    
    // Try to get CPU brand string
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0);
    
    if (cpuInfo[0] >= 1) {
        // Get vendor string
        char vendor[13];
        memset(vendor, 0, sizeof(vendor));
        *reinterpret_cast<int*>(vendor) = cpuInfo[1];
        *reinterpret_cast<int*>(vendor + 4) = cpuInfo[3];
        *reinterpret_cast<int*>(vendor + 8) = cpuInfo[2];
        s_Vendor = vendor;
        
        // Get brand string
        if (cpuInfo[0] >= 0x80000004) {
            char brand[49];
            memset(brand, 0, sizeof(brand));
            
            __cpuid(cpuInfo, 0x80000002);
            memcpy(brand, cpuInfo, sizeof(cpuInfo));
            
            __cpuid(cpuInfo, 0x80000003);
            memcpy(brand + 16, cpuInfo, sizeof(cpuInfo));
            
            __cpuid(cpuInfo, 0x80000004);
            memcpy(brand + 32, cpuInfo, sizeof(cpuInfo));
            
            s_Brand = brand;
        }
        
        // Get features
        __cpuid(cpuInfo, 1);
        
        s_Features.Exs_SSE = (cpuInfo[3] & (1 << 25)) != 0;
        s_Features.Exs_SSE2 = (cpuInfo[3] & (1 << 26)) != 0;
        s_Features.Exs_SSE3 = (cpuInfo[2] & (1 << 0)) != 0;
        s_Features.Exs_SSSE3 = (cpuInfo[2] & (1 << 9)) != 0;
        s_Features.Exs_SSE41 = (cpuInfo[2] & (1 << 19)) != 0;
        s_Features.Exs_SSE42 = (cpuInfo[2] & (1 << 20)) != 0;
        s_Features.Exs_AVX = (cpuInfo[2] & (1 << 28)) != 0;
        s_Features.Exs_FMA3 = (cpuInfo[2] & (1 << 12)) != 0;
        
        s_Features.Exs_MMX = (cpuInfo[3] & (1 << 23)) != 0;
    }
#endif
    
    // For non-Windows, use simplified detection
#ifndef _WIN32
    s_Vendor = "Generic";
    s_Brand = "Unknown CPU";
    
    // Try to get core count from system
#ifdef __linux__
    s_CoreCount = sysconf(_SC_NPROCESSORS_ONLN);
    s_ThreadCount = s_CoreCount;
#endif
#endif
}

std::string Exs_CPUInfo::Exs_GetVendor() {
    return s_Vendor;
}

std::string Exs_CPUInfo::Exs_GetBrand() {
    return s_Brand;
}

uint32_t Exs_CPUInfo::Exs_GetCoreCount() {
    return s_CoreCount;
}

uint32_t Exs_CPUInfo::Exs_GetThreadCount() {
    return s_ThreadCount;
}

uint32_t Exs_CPUInfo::Exs_GetPhysicalCoreCount() {
    return s_CoreCount;
}

uint32_t Exs_CPUInfo::Exs_GetLogicalCoreCount() {
    return s_ThreadCount;
}

uint64_t Exs_CPUInfo::Exs_GetFrequency() {
    return s_Frequency;
}

uint64_t Exs_CPUInfo::Exs_GetMaxFrequency() {
    return s_Frequency;
}

uint64_t Exs_CPUInfo::Exs_GetMinFrequency() {
    return s_Frequency;
}

const Exs_CPUFeatures& Exs_CPUInfo::Exs_GetFeatures() {
    return s_Features;
}

bool Exs_CPUInfo::Exs_HasFeature(const std::string& feature) {
    if (feature == "SSE") return s_Features.Exs_SSE;
    if (feature == "SSE2") return s_Features.Exs_SSE2;
    if (feature == "SSE3") return s_Features.Exs_SSE3;
    if (feature == "AVX") return s_Features.Exs_AVX;
    if (feature == "AVX2") return s_Features.Exs_AVX2;
    if (feature == "FMA") return s_Features.Exs_FMA;
    if (feature == "MMX") return s_Features.Exs_MMX;
    return false;
}

uint64_t Exs_CPUInfo::Exs_GetCacheSizeL1() {
    return 0; // Not implemented
}

uint64_t Exs_CPUInfo::Exs_GetCacheSizeL2() {
    return 0; // Not implemented
}

uint64_t Exs_CPUInfo::Exs_GetCacheSizeL3() {
    return 0; // Not implemented
}

void Exs_CPUInfo::Exs_DumpCPUInfo() {
    std::cout << "=== Exs CPU Info ===" << std::endl;
    std::cout << "Vendor: " << s_Vendor << std::endl;
    std::cout << "Brand: " << s_Brand << std::endl;
    std::cout << "Cores: " << s_CoreCount << std::endl;
    std::cout << "Threads: " << s_ThreadCount << std::endl;
    std::cout << "Frequency: " << s_Frequency << " MHz" << std::endl;
    std::cout << "Features: ";
    if (s_Features.Exs_SSE) std::cout << "SSE ";
    if (s_Features.Exs_SSE2) std::cout << "SSE2 ";
    if (s_Features.Exs_SSE3) std::cout << "SSE3 ";
    if (s_Features.Exs_AVX) std::cout << "AVX ";
    if (s_Features.Exs_AVX2) std::cout << "AVX2 ";
    if (s_Features.Exs_MMX) std::cout << "MMX ";
    std::cout << std::endl;
    std::cout << "=====================" << std::endl;
}

} // namespace Platform
} // namespace Exs
