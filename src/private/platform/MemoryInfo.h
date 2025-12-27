// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once
#include <cstdint>

namespace Exs {
namespace Platform {

struct Exs_MemoryStats {
    uint64_t Exs_TotalPhysical;      // Total physical memory in bytes
    uint64_t Exs_AvailablePhysical;  // Available physical memory in bytes
    uint64_t Exs_UsedPhysical;       // Used physical memory in bytes
    
    uint64_t Exs_TotalVirtual;       // Total virtual memory in bytes
    uint64_t Exs_AvailableVirtual;   // Available virtual memory in bytes
    uint64_t Exs_UsedVirtual;        // Used virtual memory in bytes
    
    uint64_t Exs_TotalPageFile;      // Total page file in bytes
    uint64_t Exs_AvailablePageFile;  // Available page file in bytes
    uint64_t Exs_UsedPageFile;       // Used page file in bytes
    
    uint64_t Exs_PageSize;           // System page size in bytes
    uint64_t Exs_AllocationGranularity; // Allocation granularity
    
    // Process memory
    uint64_t Exs_ProcessWorkingSet;     // Current working set
    uint64_t Exs_ProcessPeakWorkingSet; // Peak working set
    uint64_t Exs_ProcessPageFileUsage;  // Page file usage
    uint64_t Exs_ProcessPeakPageFileUsage; // Peak page file usage
};

class Exs_MemoryInfo {
private:
    static Exs_MemoryStats s_CurrentStats;
    static bool s_Initialized;
    
    static void Exs_DetectMemoryInfo();
    
public:
    static void Exs_Initialize();
    static void Exs_Update();
    
    // System memory information
    static const Exs_MemoryStats& Exs_GetStats();
    static uint64_t Exs_GetTotalPhysicalMemory();
    static uint64_t Exs_GetAvailablePhysicalMemory();
    static uint64_t Exs_GetUsedPhysicalMemory();
    static uint64_t Exs_GetPhysicalMemoryUsagePercent();
    
    // Virtual memory information
    static uint64_t Exs_GetTotalVirtualMemory();
    static uint64_t Exs_GetAvailableVirtualMemory();
    static uint64_t Exs_GetUsedVirtualMemory();
    static uint64_t Exs_GetVirtualMemoryUsagePercent();
    
    // Page file information
    static uint64_t Exs_GetTotalPageFile();
    static uint64_t Exs_GetAvailablePageFile();
    static uint64_t Exs_GetUsedPageFile();
    static uint64_t Exs_GetPageFileUsagePercent();
    
    // Process memory information
    static uint64_t Exs_GetProcessMemoryUsage();
    static uint64_t Exs_GetProcessPeakMemoryUsage();
    static uint64_t Exs_GetProcessPrivateBytes();
    static uint64_t Exs_GetProcessSharedBytes();
    
    // Memory system properties
    static uint64_t Exs_GetPageSize();
    static uint64_t Exs_GetAllocationGranularity();
    static uint64_t Exs_GetCacheLineSize();
    
    // Memory utilities
    static void* Exs_AllocateSystemMemory(size_t size, size_t alignment = 0);
    static void Exs_FreeSystemMemory(void* ptr, size_t size);
    
    // Debug functions
    static void Exs_DumpMemoryInfo();
    static void Exs_DumpProcessMemory();
};

} // namespace Platform
} // namespace Exs
