#pragma once
#ifndef EXS_CPUINFO_H
#define EXS_CPUINFO_H

#include "PlatformDetection.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Exs {
namespace Internal {
namespace Platform {

class EXS_API Exs_CPUInfo {
private:
    // CPUID data structure
    struct Exs_CPUIDData {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    };
    
    struct Exs_CacheInfo {
        uint32_t level;
        uint32_t type;
        uint32_t size;
        uint32_t lineSize;
        uint32_t associativity;
        uint32_t sets;
        bool shared;
        uint32_t sharingCores;
    };
    
    struct Exs_TopologyInfo {
        uint32_t packageId;
        uint32_t coreId;
        uint32_t threadId;
        uint32_t smtId;
        uint32_t numaNodeId;
        uint32_t apicId;
    };
    
    // Singleton instance
    static Exs_CPUInfo* Exs_instance;
    
    // CPU information
    std::string Exs_vendorString;
    std::string Exs_brandString;
    uint32_t Exs_family;
    uint32_t Exs_model;
    uint32_t Exs_stepping;
    uint32_t Exs_type;
    uint32_t Exs_extendedFamily;
    uint32_t Exs_extendedModel;
    
    // Feature flags
    bool Exs_hasFPU;
    bool Exs_hasMMX;
    bool Exs_hasSSE;
    bool Exs_hasSSE2;
    bool Exs_hasSSE3;
    bool Exs_hasSSSE3;
    bool Exs_hasSSE4_1;
    bool Exs_hasSSE4_2;
    bool Exs_hasAVX;
    bool Exs_hasAVX2;
    bool Exs_hasAVX512F;
    bool Exs_hasAVX512DQ;
    bool Exs_hasAVX512IFMA;
    bool Exs_hasAVX512PF;
    bool Exs_hasAVX512ER;
    bool Exs_hasAVX512CD;
    bool Exs_hasAVX512BW;
    bool Exs_hasAVX512VL;
    bool Exs_hasAVX512VBMI;
    bool Exs_hasFMA;
    bool Exs_hasAES;
    bool Exs_hasPCLMULQDQ;
    bool Exs_hasRDRAND;
    bool Exs_hasRDSEED;
    bool Exs_hasSHA;
    bool Exs_hasADX;
    bool Exs_hasBMI1;
    bool Exs_hasBMI2;
    bool Exs_hasLZCNT;
    bool Exs_hasPOPCNT;
    bool Exs_hasTBM;
    bool Exs_hasF16C;
    bool Exs_hasFMA4;
    bool Exs_hasXOP;
    bool Exs_has3DNOW;
    bool Exs_has3DNOWEXT;
    bool Exs_hasHTT; // Hyper-Threading Technology
    
    // Cache information
    std::vector<Exs_CacheInfo> Exs_cacheInfo;
    uint32_t Exs_cacheLineSize;
    uint32_t Exs_L1DataCacheSize;
    uint32_t Exs_L1InstructionCacheSize;
    uint32_t Exs_L2CacheSize;
    uint32_t Exs_L3CacheSize;
    uint32_t Exs_L4CacheSize;
    
    // Topology information
    uint32_t Exs_logicalCores;
    uint32_t Exs_physicalCores;
    uint32_t Exs_packages;
    std::vector<Exs_TopologyInfo> Exs_topology;
    
    // Performance information
    uint32_t Exs_baseFrequency;
    uint32_t Exs_maxFrequency;
    uint32_t Exs_busFrequency;
    
    // ARM specific
    bool Exs_hasNEON;
    bool Exs_hasCRC32;
    bool Exs_hasCRC32C;
    bool Exs_hasASIMD;
    bool Exs_hasASIMDDP;
    bool Exs_hasASIMDFHM;
    bool Exs_hasASIMDRDM;
    bool Exs_hasSVE;
    bool Exs_hasSVEBitPerm;
    bool Exs_hasSVESHA3;
    bool Exs_hasSVESM4;
    bool Exs_hasSVEAES;
    
    // Power management
    bool Exs_hasAPERF;
    bool Exs_hasMPERF;
    bool Exs_hasACPI;
    bool Exs_hasEST;
    bool Exs_hasTM;
    bool Exs_hasTM2;
    bool Exs_hasEIST;
    
    // Virtualization
    bool Exs_hasVMX;
    bool Exs_hasSVM;
    bool Exs_hasHypervisor;
    
    // Security features
    bool Exs_hasSGX;
    bool Exs_hasSMX;
    bool Exs_hasSKINIT;
    bool Exs_hasPT;
    bool Exs_hasNX;
    bool Exs_hasSMEP;
    bool Exs_hasSMAP;
    bool Exs_hasUMIP;
    bool Exs_hasPCID;
    bool Exs_hasINVPCID;
    bool Exs_hasPGE;
    bool Exs_hasFSGSBASE;
    bool Exs_hasPCONFIG;
    bool Exs_hasIBRS;
    bool Exs_hasSTIBP;
    bool Exs_hasSSBD;
    
    // Methods
    Exs_CPUInfo();
    ~Exs_CPUInfo();
    
    void Exs_DetectCPU();
    void Exs_DetectFeatures();
    void Exs_DetectCache();
    void Exs_DetectTopology();
    void Exs_DetectFrequency();
    
#if defined(EXS_PLATFORM_WINDOWS)
    void Exs_DetectWindows();
#elif defined(EXS_PLATFORM_LINUX)
    void Exs_DetectLinux();
#elif defined(EXS_PLATFORM_APPLE)
    void Exs_DetectMac();
#endif
    
    Exs_CPUIDData Exs_CPUID(uint32_t function, uint32_t subfunction = 0);
    
public:
    // Singleton access
    static Exs_CPUInfo& Exs_GetInstance();
    static void Exs_Destroy();
    
    // Getters
    const std::string& Exs_GetVendorString() const { return Exs_vendorString; }
    const std::string& Exs_GetBrandString() const { return Exs_brandString; }
    
    uint32_t Exs_GetFamily() const { return Exs_family; }
    uint32_t Exs_GetModel() const { return Exs_model; }
    uint32_t Exs_GetStepping() const { return Exs_stepping; }
    uint32_t Exs_GetType() const { return Exs_type; }
    
    uint32_t Exs_GetLogicalCores() const { return Exs_logicalCores; }
    uint32_t Exs_GetPhysicalCores() const { return Exs_physicalCores; }
    uint32_t Exs_GetPackages() const { return Exs_packages; }
    
    uint32_t Exs_GetCacheLineSize() const { return Exs_cacheLineSize; }
    uint32_t Exs_GetL1DataCacheSize() const { return Exs_L1DataCacheSize; }
    uint32_t Exs_GetL1InstructionCacheSize() const { return Exs_L1InstructionCacheSize; }
    uint32_t Exs_GetL2CacheSize() const { return Exs_L2CacheSize; }
    uint32_t Exs_GetL3CacheSize() const { return Exs_L3CacheSize; }
    uint32_t Exs_GetL4CacheSize() const { return Exs_L4CacheSize; }
    
    uint32_t Exs_GetBaseFrequency() const { return Exs_baseFrequency; }
    uint32_t Exs_GetMaxFrequency() const { return Exs_maxFrequency; }
    uint32_t Exs_GetBusFrequency() const { return Exs_busFrequency; }
    
    // Feature checks
    bool Exs_HasFPU() const { return Exs_hasFPU; }
    bool Exs_HasMMX() const { return Exs_hasMMX; }
    bool Exs_HasSSE() const { return Exs_hasSSE; }
    bool Exs_HasSSE2() const { return Exs_hasSSE2; }
    bool Exs_HasSSE3() const { return Exs_hasSSE3; }
    bool Exs_HasSSSE3() const { return Exs_hasSSSE3; }
    bool Exs_HasSSE4_1() const { return Exs_hasSSE4_1; }
    bool Exs_HasSSE4_2() const { return Exs_hasSSE4_2; }
    bool Exs_HasAVX() const { return Exs_hasAVX; }
    bool Exs_HasAVX2() const { return Exs_hasAVX2; }
    bool Exs_HasFMA() const { return Exs_hasFMA; }
    bool Exs_HasAES() const { return Exs_hasAES; }
    bool Exs_HasRDRAND() const { return Exs_hasRDRAND; }
    bool Exs_HasRDSEED() const { return Exs_hasRDSEED; }
    bool Exs_HasSHA() const { return Exs_hasSHA; }
    bool Exs_HasBMI1() const { return Exs_hasBMI1; }
    bool Exs_HasBMI2() const { return Exs_hasBMI2; }
    bool Exs_HasPOPCNT() const { return Exs_hasPOPCNT; }
    bool Exs_HasHTT() const { return Exs_hasHTT; }
    
    bool Exs_HasNEON() const { return Exs_hasNEON; }
    bool Exs_HasCRC32() const { return Exs_hasCRC32; }
    
    bool Exs_HasVMX() const { return Exs_hasVMX; }
    bool Exs_HasSVM() const { return Exs_hasSVM; }
    bool Exs_HasHypervisor() const { return Exs_hasHypervisor; }
    
    bool Exs_HasSGX() const { return Exs_hasSGX; }
    bool Exs_HasNX() const { return Exs_hasNX; }
    bool Exs_HasSMEP() const { return Exs_hasSMEP; }
    bool Exs_HasSMAP() const { return Exs_hasSMAP; }
    
    // Utility functions
    bool Exs_IsIntel() const;
    bool Exs_IsAMD() const;
    bool Exs_IsARM() const;
    bool Exs_Is64Bit() const;
    bool Exs_IsHyperThreaded() const;
    
    const std::vector<Exs_CacheInfo>& Exs_GetCacheInfo() const { return Exs_cacheInfo; }
    const std::vector<Exs_TopologyInfo>& Exs_GetTopology() const { return Exs_topology; }
    
    void Exs_PrintInfo() const;
    std::string Exs_ToString() const;
    
    // Performance
    uint32_t Exs_GetCurrentFrequency() const;
    float Exs_GetCurrentLoad(uint32_t core = 0) const;
    float Exs_GetTemperature() const;
    float Exs_GetPowerUsage() const;
    
    // CPUID wrapper
    static Exs_CPUIDData Exs_ExecuteCPUID(uint32_t function, uint32_t subfunction = 0);
};

} // namespace Platform
} // namespace Internal
} // namespace Exs

#endif // EXS_CPUINFO_H
