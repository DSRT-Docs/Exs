#include "CPUInfo.h"
#include "Platform.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>

#if defined(EXS_PLATFORM_WINDOWS)
    #include <windows.h>
    #include <intrin.h>
    #include <pdh.h>
    #include <pdhmsg.h>
    #pragma comment(lib, "pdh.lib")
#elif defined(EXS_PLATFORM_LINUX)
    #include <unistd.h>
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
    #include <cpuid.h>
#endif

namespace Exs {
namespace Internal {
namespace Platform {

// Static member initialization
Exs_CPUInfo* Exs_CPUInfo::Exs_instance = nullptr;

// Constructor
Exs_CPUInfo::Exs_CPUInfo() {
    Exs_DetectCPU();
}

// Destructor
Exs_CPUInfo::~Exs_CPUInfo() {
}

// Singleton access
Exs_CPUInfo& Exs_CPUInfo::Exs_GetInstance() {
    if (!Exs_instance) {
        Exs_instance = new Exs_CPUInfo();
    }
    return *Exs_instance;
}

void Exs_CPUInfo::Exs_Destroy() {
    if (Exs_instance) {
        delete Exs_instance;
        Exs_instance = nullptr;
    }
}

// CPUID execution
Exs_CPUInfo::Exs_CPUIDData Exs_CPUInfo::Exs_ExecuteCPUID(uint32_t function, uint32_t subfunction) {
    Exs_CPUIDData result = {0, 0, 0, 0};
    
#if defined(EXS_PLATFORM_WINDOWS) && (defined(EXS_ARCH_X86) || defined(EXS_ARCH_X64))
    int cpuInfo[4];
    __cpuidex(cpuInfo, function, subfunction);
    result.eax = cpuInfo[0];
    result.ebx = cpuInfo[1];
    result.ecx = cpuInfo[2];
    result.edx = cpuInfo[3];
#elif defined(EXS_PLATFORM_LINUX) && (defined(EXS_ARCH_X86) || defined(EXS_ARCH_X64))
    __cpuid_count(function, subfunction, result.eax, result.ebx, result.ecx, result.edx);
#elif defined(EXS_PLATFORM_APPLE) && defined(EXS_ARCH_ARM64)
    // ARM CPUID emulation for macOS
    // Not all functions are available
    if (function == 0) {
        // Get vendor
        result.ebx = 0x6C65746E; // "n" "t" "e" "l"
        result.edx = 0x49656E69; // "i" "n" "e" "I"
        result.ecx = 0x6C65746E; // "n" "t" "e" "l"
    }
#endif
    
    return result;
}

Exs_CPUInfo::Exs_CPUIDData Exs_CPUInfo::Exs_CPUID(uint32_t function, uint32_t subfunction) {
    return Exs_ExecuteCPUID(function, subfunction);
}

// Main detection function
void Exs_CPUInfo::Exs_DetectCPU() {
    // Get basic CPU information
    Exs_CPUIDData cpuid0 = Exs_CPUID(0);
    
    // Extract vendor string
    char vendor[13];
    memcpy(vendor, &cpuid0.ebx, 4);
    memcpy(vendor + 4, &cpuid0.edx, 4);
    memcpy(vendor + 8, &cpuid0.ecx, 4);
    vendor[12] = '\0';
    Exs_vendorString = vendor;
    
    // Get extended information
    Exs_CPUIDData cpuid1 = Exs_CPUID(1);
    
    // Decode CPU family, model, stepping
    Exs_stepping = cpuid1.eax & 0xF;
    Exs_model = (cpuid1.eax >> 4) & 0xF;
    Exs_family = (cpuid1.eax >> 8) & 0xF;
    Exs_type = (cpuid1.eax >> 12) & 0x3;
    
    // Extended family and model
    if (Exs_family == 0xF) {
        Exs_extendedFamily = (cpuid1.eax >> 20) & 0xFF;
        Exs_family += Exs_extendedFamily;
        Exs_extendedModel = (cpuid1.eax >> 16) & 0xF;
        Exs_model += (Exs_extendedModel << 4);
    }
    
    // Get brand string if available
    if (cpuid0.eax >= 0x80000004) {
        char brand[49] = {0};
        Exs_CPUIDData brand1 = Exs_CPUID(0x80000002);
        Exs_CPUIDData brand2 = Exs_CPUID(0x80000003);
        Exs_CPUIDData brand3 = Exs_CPUID(0x80000004);
        
        memcpy(brand, &brand1, sizeof(brand1));
        memcpy(brand + 16, &brand2, sizeof(brand2));
        memcpy(brand + 32, &brand3, sizeof(brand3));
        brand[48] = '\0';
        Exs_brandString = brand;
        
        // Trim trailing spaces
        size_t end = Exs_brandString.find_last_not_of(' ');
        if (end != std::string::npos) {
            Exs_brandString = Exs_brandString.substr(0, end + 1);
        }
    }
    
    // Detect features
    Exs_DetectFeatures();
    
    // Detect cache
    Exs_DetectCache();
    
    // Detect topology
    Exs_DetectTopology();
    
    // Detect frequency
    Exs_DetectFrequency();
    
    // Platform-specific detection
#if defined(EXS_PLATFORM_WINDOWS)
    Exs_DetectWindows();
#elif defined(EXS_PLATFORM_LINUX)
    Exs_DetectLinux();
#elif defined(EXS_PLATFORM_APPLE)
    Exs_DetectMac();
#endif
}

// Feature detection
void Exs_CPUInfo::Exs_DetectFeatures() {
    Exs_CPUIDData cpuid1 = Exs_CPUID(1);
    Exs_CPUIDData cpuid7 = Exs_CPUID(7);
    Exs_CPUIDData cpuid80000001 = Exs_CPUID(0x80000001);
    
    // Basic features from cpuid1
    Exs_hasFPU = (cpuid1.edx & (1 << 0)) != 0;
    Exs_hasMMX = (cpuid1.edx & (1 << 23)) != 0;
    Exs_hasSSE = (cpuid1.edx & (1 << 25)) != 0;
    Exs_hasSSE2 = (cpuid1.edx & (1 << 26)) != 0;
    Exs_hasSSE3 = (cpuid1.ecx & (1 << 0)) != 0;
    Exs_hasSSSE3 = (cpuid1.ecx & (1 << 9)) != 0;
    Exs_hasSSE4_1 = (cpuid1.ecx & (1 << 19)) != 0;
    Exs_hasSSE4_2 = (cpuid1.ecx & (1 << 20)) != 0;
    Exs_hasAVX = (cpuid1.ecx & (1 << 28)) != 0;
    Exs_hasFMA = (cpuid1.ecx & (1 << 12)) != 0;
    Exs_hasAES = (cpuid1.ecx & (1 << 25)) != 0;
    Exs_hasPCLMULQDQ = (cpuid1.ecx & (1 << 1)) != 0;
    Exs_hasRDRAND = (cpuid1.ecx & (1 << 30)) != 0;
    Exs_hasF16C = (cpuid1.ecx & (1 << 29)) != 0;
    Exs_hasPOPCNT = (cpuid1.ecx & (1 << 23)) != 0;
    
    // Hyper-Threading Technology
    Exs_hasHTT = (cpuid1.edx & (1 << 28)) != 0;
    
    // Features from cpuid7
    if (cpuid7.eax >= 1) {
        Exs_hasAVX2 = (cpuid7.ebx & (1 << 5)) != 0;
        Exs_hasAVX512F = (cpuid7.ebx & (1 << 16)) != 0;
        Exs_hasAVX512DQ = (cpuid7.ebx & (1 << 17)) != 0;
        Exs_hasAVX512IFMA = (cpuid7.ebx & (1 << 21)) != 0;
        Exs_hasAVX512PF = (cpuid7.ebx & (1 << 26)) != 0;
        Exs_hasAVX512ER = (cpuid7.ebx & (1 << 27)) != 0;
        Exs_hasAVX512CD = (cpuid7.ebx & (1 << 28)) != 0;
        Exs_hasAVX512BW = (cpuid7.ebx & (1 << 30)) != 0;
        Exs_hasAVX512VL = (cpuid7.ebx & (1 << 31)) != 0;
        Exs_hasSHA = (cpuid7.ebx & (1 << 29)) != 0;
        Exs_hasBMI1 = (cpuid7.ebx & (1 << 3)) != 0;
        Exs_hasBMI2 = (cpuid7.ebx & (1 << 8)) != 0;
        Exs_hasRDSEED = (cpuid7.ebx & (1 << 18)) != 0;
        Exs_hasADX = (cpuid7.ebx & (1 << 19)) != 0;
        Exs_hasSGX = (cpuid7.ebx & (1 << 2)) != 0;
        Exs_hasSMEP = (cpuid7.ebx & (1 << 7)) != 0;
        Exs_hasSMAP = (cpuid7.ebx & (1 << 20)) != 0;
        Exs_hasUMIP = (cpuid7.ecx & (1 << 2)) != 0;
        Exs_hasPCID = (cpuid7.ecx & (1 << 17)) != 0;
        Exs_hasINVPCID = (cpuid7.ecx & (1 << 10)) != 0;
        Exs_hasFSGSBASE = (cpuid7.ebx & (1 << 0)) != 0;
        Exs_hasPCONFIG = (cpuid7.edx & (1 << 18)) != 0;
        Exs_hasIBRS = (cpuid7.edx & (1 << 26)) != 0;
        Exs_hasSTIBP = (cpuid7.edx & (1 << 27)) != 0;
        Exs_hasSSBD = (cpuid7.edx & (1 << 31)) != 0;
        
        // AVX512 extensions from ecx
        Exs_hasAVX512VBMI = (cpuid7.ecx & (1 << 1)) != 0;
    }
    
    // Extended features from cpuid80000001
    Exs_hasNX = (cpuid80000001.edx & (1 << 20)) != 0;
    Exs_has3DNOW = (cpuid80000001.edx & (1 << 31)) != 0;
    Exs_has3DNOWEXT = (cpuid80000001.edx & (1 << 30)) != 0;
    
    // Check for virtualization
    Exs_hasVMX = (cpuid1.ecx & (1 << 5)) != 0;
    Exs_hasSVM = (cpuid80000001.ecx & (1 << 2)) != 0;
    
    // Check for hypervisor
    Exs_hasHypervisor = (cpuid1.ecx & (1 << 31)) != 0;
}

// Cache detection (simplified)
void Exs_CPUInfo::Exs_DetectCache() {
    // Default values
    Exs_cacheLineSize = 64;
    Exs_L1DataCacheSize = 32 * 1024; // 32KB typical
    Exs_L1InstructionCacheSize = 32 * 1024;
    Exs_L2CacheSize = 256 * 1024; // 256KB typical
    Exs_L3CacheSize = 8 * 1024 * 1024; // 8MB typical
    Exs_L4CacheSize = 0;
    
    // Try to get actual cache information
    Exs_CPUIDData cpuid2 = Exs_CPUID(2);
    Exs_CPUIDData cpuid4 = Exs_CPUID(4);
    
    // Parse cache information
    // This is simplified - actual cache detection is complex
}

// Topology detection (simplified)
void Exs_CPUInfo::Exs_DetectTopology() {
#if defined(EXS_PLATFORM_WINDOWS)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    Exs_logicalCores = sysInfo.dwNumberOfProcessors;
    
    // Try to get physical cores count
    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(nullptr, &bufferSize);
    
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        auto* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufferSize);
        if (buffer) {
            if (GetLogicalProcessorInformation(buffer, &bufferSize)) {
                DWORD byteOffset = 0;
                Exs_physicalCores = 0;
                Exs_packages = 0;
                
                while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= bufferSize) {
                    auto* info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)((BYTE*)buffer + byteOffset);
                    
                    if (info->Relationship == RelationProcessorCore) {
                        Exs_physicalCores++;
                        // Count bits in processor mask to get threads per core
                        DWORD mask = info->ProcessorMask;
                        DWORD threads = 0;
                        while (mask) {
                            threads += mask & 1;
                            mask >>= 1;
                        }
                    } else if (info->Relationship == RelationProcessorPackage) {
                        Exs_packages++;
                    }
                    
                    byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                }
            }
            free(buffer);
        }
    }
#elif defined(EXS_PLATFORM_LINUX)
    Exs_logicalCores = sysconf(_SC_NPROCESSORS_ONLN);
    
    // Read topology from /sys
    std::ifstream cpuPresent("/sys/devices/system/cpu/present");
    if (cpuPresent.is_open()) {
        std::string line;
        std::getline(cpuPresent, line);
        // Parse range like "0-7"
        size_t dashPos = line.find('-');
        if (dashPos != std::string::npos) {
            int start = std::stoi(line.substr(0, dashPos));
            int end = std::stoi(line.substr(dashPos + 1));
            Exs_logicalCores = end - start + 1;
        }
    }
    
    // Try to get physical cores count
    // This is simplified - actual topology detection requires parsing /sys/devices/system/cpu/cpuX/topology
    Exs_physicalCores = Exs_logicalCores / (Exs_hasHTT ? 2 : 1);
    Exs_packages = 1;
#endif
}

// Frequency detection
void Exs_CPUInfo::Exs_DetectFrequency() {
#if defined(EXS_PLATFORM_WINDOWS)
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD mhz = 0;
        DWORD size = sizeof(mhz);
        if (RegQueryValueEx(hKey, "~MHz", nullptr, nullptr, 
                           (LPBYTE)&mhz, &size) == ERROR_SUCCESS) {
            Exs_baseFrequency = mhz;
            Exs_maxFrequency = mhz;
        }
        RegCloseKey(hKey);
    }
    
    // Try to get from CPUID
    Exs_CPUIDData cpuid16 = Exs_CPUID(0x16);
    if (cpuid16.eax != 0) {
        Exs_baseFrequency = cpuid16.eax;
        Exs_maxFrequency = cpuid16.ebx;
        Exs_busFrequency = cpuid16.ecx;
    }
#endif
}

// Platform-specific detection
#if defined(EXS_PLATFORM_WINDOWS)
void Exs_CPUInfo::Exs_DetectWindows() {
    // Additional Windows-specific detection
}
#elif defined(EXS_PLATFORM_LINUX)
void Exs_CPUInfo::Exs_DetectLinux() {
    // Additional Linux-specific detection
}
#elif defined(EXS_PLATFORM_APPLE)
void Exs_CPUInfo::Exs_DetectMac() {
    // Additional macOS-specific detection
    // ARM-specific features for Apple Silicon
    Exs_hasNEON = true;
    Exs_hasCRC32 = true;
    Exs_hasCRC32C = true;
    Exs_hasASIMD = true;
}
#endif

// Utility functions
bool Exs_CPUInfo::Exs_IsIntel() const {
    return Exs_vendorString == "GenuineIntel";
}

bool Exs_CPUInfo::Exs_IsAMD() const {
    return Exs_vendorString == "AuthenticAMD";
}

bool Exs_CPUInfo::Exs_IsARM() const {
    return Exs_vendorString.find("ARM") != std::string::npos;
}

bool Exs_CPUInfo::Exs_Is64Bit() const {
#if defined(EXS_ARCH_64BIT)
    return true;
#else
    return false;
#endif
}

bool Exs_CPUInfo::Exs_IsHyperThreaded() const {
    return Exs_hasHTT && (Exs_logicalCores > Exs_physicalCores);
}

// Information printing
void Exs_CPUInfo::Exs_PrintInfo() const {
    printf("CPU Information:\n");
    printf("  Vendor: %s\n", Exs_vendorString.c_str());
    printf("  Brand: %s\n", Exs_brandString.c_str());
    printf("  Family: %u, Model: %u, Stepping: %u\n", 
           Exs_family, Exs_model, Exs_stepping);
    printf("  Cores: %u logical, %u physical, %u packages\n",
           Exs_logicalCores, Exs_physicalCores, Exs_packages);
    printf("  Cache: L1 Data: %uKB, L1 Inst: %uKB, L2: %uKB, L3: %uKB\n",
           Exs_L1DataCacheSize / 1024, Exs_L1InstructionCacheSize / 1024,
           Exs_L2CacheSize / 1024, Exs_L3CacheSize / 1024);
    printf("  Frequency: Base: %u MHz, Max: %u MHz, Bus: %u MHz\n",
           Exs_baseFrequency, Exs_maxFrequency, Exs_busFrequency);
    
    printf("  Features: ");
    if (Exs_hasSSE) printf("SSE ");
    if (Exs_hasSSE2) printf("SSE2 ");
    if (Exs_hasSSE3) printf("SSE3 ");
    if (Exs_hasSSSE3) printf("SSSE3 ");
    if (Exs_hasSSE4_1) printf("SSE4.1 ");
    if (Exs_hasSSE4_2) printf("SSE4.2 ");
    if (Exs_hasAVX) printf("AVX ");
    if (Exs_hasAVX2) printf("AVX2 ");
    if (Exs_hasAVX512F) printf("AVX512F ");
    if (Exs_hasFMA) printf("FMA ");
    if (Exs_hasAES) printf("AES ");
    if (Exs_hasRDRAND) printf("RDRAND ");
    if (Exs_hasRDSEED) printf("RDSEED ");
    printf("\n");
}

std::string Exs_CPUInfo::Exs_ToString() const {
    std::stringstream ss;
    ss << "CPU: " << Exs_brandString << "\n";
    ss << "Vendor: " << Exs_vendorString << "\n";
    ss << "Architecture: " << (Exs_Is64Bit() ? "64-bit" : "32-bit") << "\n";
    ss << "Cores: " << Exs_logicalCores << " logical, " 
       << Exs_physicalCores << " physical\n";
    ss << "Cache: L1D=" << (Exs_L1DataCacheSize / 1024) << "KB, "
       << "L1I=" << (Exs_L1InstructionCacheSize / 1024) << "KB, "
       << "L2=" << (Exs_L2CacheSize / 1024) << "KB, "
       << "L3=" << (Exs_L3CacheSize / 1024) << "KB\n";
    ss << "Frequency: " << Exs_baseFrequency << " MHz base, "
       << Exs_maxFrequency << " MHz max\n";
    
    return ss.str();
}

// Performance monitoring
uint32_t Exs_CPUInfo::Exs_GetCurrentFrequency() const {
#if defined(EXS_PLATFORM_WINDOWS)
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return static_cast<uint32_t>(frequency.QuadPart / 1000000);
#else
    return Exs_baseFrequency;
#endif
}

float Exs_CPUInfo::Exs_GetCurrentLoad(uint32_t core) const {
    // Simplified implementation
    // In reality, this requires reading performance counters
    return 0.0f;
}

float Exs_CPUInfo::Exs_GetTemperature() const {
    // Platform-specific temperature reading
    return 0.0f;
}

float Exs_CPUInfo::Exs_GetPowerUsage() const {
    // Platform-specific power reading
    return 0.0f;
}

} // namespace Platform
} // namespace Internal
} // namespace Exs
