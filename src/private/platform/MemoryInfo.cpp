// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#include "MemoryInfo.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__)
    #include <sys/sysinfo.h>
    #include <sys/mman.h>
    #include <unistd.h>
#elif defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <mach/mach.h>
#endif

namespace Exs {
namespace Platform {

Exs_MemoryStats Exs_MemoryInfo::s_CurrentStats = {};
bool Exs_MemoryInfo::s_Initialized = false;

void Exs_MemoryInfo::Exs_Initialize() {
    if (s_Initialized) return;
    
    Exs_DetectMemoryInfo();
    s_Initialized = true;
}

void Exs_MemoryInfo::Exs_Update() {
    Exs_DetectMemoryInfo();
}

void Exs_MemoryInfo::Exs_DetectMemoryInfo() {
    // Reset stats
    s_CurrentStats = Exs_MemoryStats{};
    
#ifdef _WIN32
    // Windows implementation
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    
    if (GlobalMemoryStatusEx(&memStatus)) {
        s_CurrentStats.Exs_TotalPhysical = memStatus.ullTotalPhys;
        s_CurrentStats.Exs_AvailablePhysical = memStatus.ullAvailPhys;
        s_CurrentStats.Exs_UsedPhysical = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
        
        s_CurrentStats.Exs_TotalVirtual = memStatus.ullTotalPageFile;
        s_CurrentStats.Exs_AvailableVirtual = memStatus.ullAvailPageFile;
        s_CurrentStats.Exs_UsedVirtual = memStatus.ullTotalPageFile - memStatus.ullAvailPageFile;
        
        s_CurrentStats.Exs_TotalPageFile = memStatus.ullTotalPageFile;
        s_CurrentStats.Exs_AvailablePageFile = memStatus.ullAvailPageFile;
        s_CurrentStats.Exs_UsedPageFile = memStatus.ullTotalPageFile - memStatus.ullAvailPageFile;
    }
    
    // Get system info for page size and granularity
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    s_CurrentStats.Exs_PageSize = sysInfo.dwPageSize;
    s_CurrentStats.Exs_AllocationGranularity = sysInfo.dwAllocationGranularity;
    
    // Get process memory info
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        s_CurrentStats.Exs_ProcessWorkingSet = pmc.WorkingSetSize;
        s_CurrentStats.Exs_ProcessPeakWorkingSet = pmc.PeakWorkingSetSize;
        s_CurrentStats.Exs_ProcessPageFileUsage = pmc.PagefileUsage;
        s_CurrentStats.Exs_ProcessPeakPageFileUsage = pmc.PeakPagefileUsage;
    }
    
#elif defined(__linux__)
    // Linux implementation
    struct sysinfo sysInfo;
    if (sysinfo(&sysInfo) == 0) {
        s_CurrentStats.Exs_TotalPhysical = sysInfo.totalram * sysInfo.mem_unit;
        s_CurrentStats.Exs_AvailablePhysical = sysInfo.freeram * sysInfo.mem_unit;
        s_CurrentStats.Exs_UsedPhysical = s_CurrentStats.Exs_TotalPhysical - 
                                          s_CurrentStats.Exs_AvailablePhysical;
        
        s_CurrentStats.Exs_TotalVirtual = sysInfo.totalswap * sysInfo.mem_unit;
        s_CurrentStats.Exs_AvailableVirtual = sysInfo.freeswap * sysInfo.mem_unit;
        s_CurrentStats.Exs_UsedVirtual = s_CurrentStats.Exs_TotalVirtual - 
                                         s_CurrentStats.Exs_AvailableVirtual;
    }
    
    s_CurrentStats.Exs_PageSize = sysconf(_SC_PAGESIZE);
    s_CurrentStats.Exs_AllocationGranularity = s_CurrentStats.Exs_PageSize;
    
    // Linux process memory (simplified)
    s_CurrentStats.Exs_ProcessWorkingSet = 0; // Would read from /proc/self/statm
    
#elif defined(__APPLE__)
    // macOS implementation
    int mib[2];
    uint64_t physical_memory;
    size_t length;
    
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(uint64_t);
    sysctl(mib, 2, &physical_memory, &length, NULL, 0);
    
    s_CurrentStats.Exs_TotalPhysical = physical_memory;
    s_CurrentStats.Exs_PageSize = getpagesize();
    s_CurrentStats.Exs_AllocationGranularity = s_CurrentStats.Exs_PageSize;
#endif
}

const Exs_MemoryStats& Exs_MemoryInfo::Exs_GetStats() {
    return s_CurrentStats;
}

uint64_t Exs_MemoryInfo::Exs_GetTotalPhysicalMemory() {
    return s_CurrentStats.Exs_TotalPhysical;
}

uint64_t Exs_MemoryInfo::Exs_GetAvailablePhysicalMemory() {
    return s_CurrentStats.Exs_AvailablePhysical;
}

uint64_t Exs_MemoryInfo::Exs_GetUsedPhysicalMemory() {
    return s_CurrentStats.Exs_UsedPhysical;
}

uint64_t Exs_MemoryInfo::Exs_GetPhysicalMemoryUsagePercent() {
    if (s_CurrentStats.Exs_TotalPhysical == 0) return 0;
    return (s_CurrentStats.Exs_UsedPhysical * 100) / s_CurrentStats.Exs_TotalPhysical;
}

uint64_t Exs_MemoryInfo::Exs_GetTotalVirtualMemory() {
    return s_CurrentStats.Exs_TotalVirtual;
}

uint64_t Exs_MemoryInfo::Exs_GetAvailableVirtualMemory() {
    return s_CurrentStats.Exs_AvailableVirtual;
}

uint64_t Exs_MemoryInfo::Exs_GetUsedVirtualMemory() {
    return s_CurrentStats.Exs_UsedVirtual;
}

uint64_t Exs_MemoryInfo::Exs_GetVirtualMemoryUsagePercent() {
    if (s_CurrentStats.Exs_TotalVirtual == 0) return 0;
    return (s_CurrentStats.Exs_UsedVirtual * 100) / s_CurrentStats.Exs_TotalVirtual;
}

uint64_t Exs_MemoryInfo::Exs_GetTotalPageFile() {
    return s_CurrentStats.Exs_TotalPageFile;
}

uint64_t Exs_MemoryInfo::Exs_GetAvailablePageFile() {
    return s_CurrentStats.Exs_AvailablePageFile;
}

uint64_t Exs_MemoryInfo::Exs_GetUsedPageFile() {
    return s_CurrentStats.Exs_UsedPageFile;
}

uint64_t Exs_MemoryInfo::Exs_GetPageFileUsagePercent() {
    if (s_CurrentStats.Exs_TotalPageFile == 0) return 0;
    return (s_CurrentStats.Exs_UsedPageFile * 100) / s_CurrentStats.Exs_TotalPageFile;
}

uint64_t Exs_MemoryInfo::Exs_GetProcessMemoryUsage() {
    return s_CurrentStats.Exs_ProcessWorkingSet;
}

uint64_t Exs_MemoryInfo::Exs_GetProcessPeakMemoryUsage() {
    return s_CurrentStats.Exs_ProcessPeakWorkingSet;
}

uint64_t Exs_MemoryInfo::Exs_GetProcessPrivateBytes() {
    return s_CurrentStats.Exs_ProcessPageFileUsage;
}

uint64_t Exs_MemoryInfo::Exs_GetProcessSharedBytes() {
    // Simplified calculation
    return s_CurrentStats.Exs_ProcessWorkingSet - s_CurrentStats.Exs_ProcessPageFileUsage;
}

uint64_t Exs_MemoryInfo::Exs_GetPageSize() {
    return s_CurrentStats.Exs_PageSize;
}

uint64_t Exs_MemoryInfo::Exs_GetAllocationGranularity() {
    return s_CurrentStats.Exs_AllocationGranularity;
}

uint64_t Exs_MemoryInfo::Exs_GetCacheLineSize() {
#ifdef _WIN32
    DWORD buffer_size = 0;
    GetLogicalProcessorInformation(0, &buffer_size);
    return 64; // Common cache line size
#else
    return 64; // Default
#endif
}

void* Exs_MemoryInfo::Exs_AllocateSystemMemory(size_t size, size_t alignment) {
    if (alignment == 0) {
        alignment = s_CurrentStats.Exs_AllocationGranularity;
    }
    
#ifdef _WIN32
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif defined(__linux__) || defined(__APPLE__)
    return aligned_alloc(alignment, ((size + alignment - 1) / alignment) * alignment);
#endif
}

void Exs_MemoryInfo::Exs_FreeSystemMemory(void* ptr, size_t size) {
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(__linux__) || defined(__APPLE__)
    free(ptr);
#endif
}

void Exs_MemoryInfo::Exs_DumpMemoryInfo() {
    std::cout << "=== Exs Memory Info ===" << std::endl;
    std::cout << "Physical Memory:" << std::endl;
    std::cout << "  Total: " << (s_CurrentStats.Exs_TotalPhysical / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Available: " << (s_CurrentStats.Exs_AvailablePhysical / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Used: " << (s_CurrentStats.Exs_UsedPhysical / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Usage: " << Exs_GetPhysicalMemoryUsagePercent() << "%" << std::endl;
    
    std::cout << "Virtual Memory:" << std::endl;
    std::cout << "  Total: " << (s_CurrentStats.Exs_TotalVirtual / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Available: " << (s_CurrentStats.Exs_AvailableVirtual / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Usage: " << Exs_GetVirtualMemoryUsagePercent() << "%" << std::endl;
    
    std::cout << "Page Size: " << s_CurrentStats.Exs_PageSize << " bytes" << std::endl;
    std::cout << "Allocation Granularity: " << s_CurrentStats.Exs_AllocationGranularity << " bytes" << std::endl;
    std::cout << "=========================" << std::endl;
}

void Exs_MemoryInfo::Exs_DumpProcessMemory() {
    std::cout << "=== Exs Process Memory ===" << std::endl;
    std::cout << "Working Set: " << (s_CurrentStats.Exs_ProcessWorkingSet / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Peak Working Set: " << (s_CurrentStats.Exs_ProcessPeakWorkingSet / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Page File Usage: " << (s_CurrentStats.Exs_ProcessPageFileUsage / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Peak Page File: " << (s_CurrentStats.Exs_ProcessPeakPageFileUsage / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "==========================" << std::endl;
}

} // namespace Platform
} // namespace Exs
