// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#include "SystemInfo.h"
#include <chrono>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
    #include <sysinfoapi.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <sys/utsname.h>
    #include <unistd.h>
    #include <sys/sysinfo.h>
#endif

namespace Exs {
namespace Platform {

std::string Exs_SystemInfo::s_OSName;
std::string Exs_SystemInfo::s_OSVersion;
std::string Exs_SystemInfo::s_KernelVersion;
uint64_t Exs_SystemInfo::s_SystemUptime = 0;
uint32_t Exs_SystemInfo::s_ProcessCount = 0;
std::string Exs_SystemInfo::s_Hostname;
bool Exs_SystemInfo::s_Initialized = false;

void Exs_SystemInfo::Exs_Initialize() {
    if (s_Initialized) return;
    
    Exs_DetectSystemInfo();
    s_Initialized = true;
}

void Exs_SystemInfo::Exs_Update() {
    Exs_DetectSystemInfo();
}

void Exs_SystemInfo::Exs_DetectSystemInfo() {
    // Detect OS
#ifdef _WIN32
    s_OSName = "Windows";
    
    OSVERSIONINFOEXA osInfo;
    ZeroMemory(&osInfo, sizeof(osInfo));
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    GetVersionExA((OSVERSIONINFOA*)&osInfo);
    
    char version[64];
    snprintf(version, sizeof(version), "%d.%d.%d", 
             osInfo.dwMajorVersion, osInfo.dwMinorVersion, osInfo.dwBuildNumber);
    s_OSVersion = version;
    
    s_KernelVersion = "NT";
    
    // Hostname
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostname);
    GetComputerNameA(hostname, &size);
    s_Hostname = hostname;
    
    // Uptime
    s_SystemUptime = GetTickCount64() / 1000;
    
#elif defined(__linux__) || defined(__APPLE__)
    struct utsname sysInfo;
    if (uname(&sysInfo) == 0) {
        s_OSName = sysInfo.sysname;
        s_KernelVersion = sysInfo.release;
        s_OSVersion = sysInfo.version;
    }
    
    // Hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        s_Hostname = hostname;
    }
    
    // Uptime
#ifdef __linux__
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        s_SystemUptime = sys_info.uptime;
    }
#endif
    
#endif
    
    // Process count (simplified)
    s_ProcessCount = 1; // Placeholder
}

std::string Exs_SystemInfo::Exs_GetOSName() {
    return s_OSName;
}

std::string Exs_SystemInfo::Exs_GetOSVersion() {
    return s_OSVersion;
}

std::string Exs_SystemInfo::Exs_GetKernelVersion() {
    return s_KernelVersion;
}

std::string Exs_SystemInfo::Exs_GetHostname() {
    return s_Hostname;
}

uint64_t Exs_SystemInfo::Exs_GetSystemUptime() {
    return s_SystemUptime;
}

uint32_t Exs_SystemInfo::Exs_GetProcessCount() {
    return s_ProcessCount;
}

uint32_t Exs_SystemInfo::Exs_GetProcessorCount() {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

uint64_t Exs_SystemInfo::Exs_GetPageSize() {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
#else
    return sysconf(_SC_PAGESIZE);
#endif
}

bool Exs_SystemInfo::Exs_IsMultiProcessor() {
    return Exs_GetProcessorCount() > 1;
}

void Exs_SystemInfo::Exs_DumpSystemInfo() {
    std::cout << "=== Exs System Info ===" << std::endl;
    std::cout << "OS: " << s_OSName << std::endl;
    std::cout << "Version: " << s_OSVersion << std::endl;
    std::cout << "Kernel: " << s_KernelVersion << std::endl;
    std::cout << "Hostname: " << s_Hostname << std::endl;
    std::cout << "Uptime: " << s_SystemUptime << " seconds" << std::endl;
    std::cout << "Processors: " << Exs_GetProcessorCount() << std::endl;
    std::cout << "Page Size: " << Exs_GetPageSize() << " bytes" << std::endl;
    std::cout << "=========================" << std::endl;
}

} // namespace Platform
} // namespace Exs
