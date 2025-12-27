// src/Core/Platform/Windows/CPUInfoWindows.cpp
#include "../internal/CPUInfoBase.h"
#include <windows.h>
#include <intrin.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <winternl.h>
#include <wmicom.h>
#include <comdef.h>
#include <wbemidl.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")

namespace Exs {
namespace Internal {
namespace CPUInfo {

// CPUID function
void Exs_CPUID(int32 cpuInfo[4], int32 functionId) {
    __cpuid(cpuInfo, functionId);
}

// CPUID with extended function
void Exs_CPUIDEX(int32 cpuInfo[4], int32 functionId, int32 subfunctionId) {
    __cpuidex(cpuInfo, functionId, subfunctionId);
}

// Get xgetbv
uint64 Exs_xgetbv(uint32 index) {
    return _xgetbv(index);
}

class Exs_CPUInfoWindows : public Exs_CPUInfoBase {
private:
    mutable std::string cpuNameCache;
    mutable Exs_CPUVendor cpuVendorCache = Exs_CPUVendor::Unknown;
    mutable Exs_CPUFeatures cpuFeaturesCache;
    mutable bool featuresInitialized = false;
    mutable PDH_HQUERY cpuQuery = nullptr;
    mutable PDH_HCOUNTER totalCpuCounter = nullptr;
    mutable std::vector<PDH_HCOUNTER> coreCounters;
    
public:
    Exs_CPUInfoWindows() {
        initializePerformanceCounters();
    }
    
    virtual ~Exs_CPUInfoWindows() {
        cleanupPerformanceCounters();
    }
    
    std::string getCPUName() const override {
        if (!cpuNameCache.empty()) {
            return cpuNameCache;
        }
        
        int32 cpuInfo[4] = {0};
        char cpuName[49] = {0};
        
        // Get CPU brand string
        Exs_CPUID(cpuInfo, 0x80000002);
        memcpy(cpuName, cpuInfo, sizeof(cpuInfo));
        
        Exs_CPUID(cpuInfo, 0x80000003);
        memcpy(cpuName + 16, cpuInfo, sizeof(cpuInfo));
        
        Exs_CPUID(cpuInfo, 0x80000004);
        memcpy(cpuName + 32, cpuInfo, sizeof(cpuInfo));
        
        cpuNameCache = std::string(cpuName);
        
        // Trim trailing spaces
        cpuNameCache.erase(cpuNameCache.find_last_not_of(" \n\r\t") + 1);
        
        return cpuNameCache;
    }
    
    Exs_CPUVendor getCPUVendor() const override {
        if (cpuVendorCache != Exs_CPUVendor::Unknown) {
            return cpuVendorCache;
        }
        
        int32 cpuInfo[4] = {0};
        char vendorString[13] = {0};
        
        Exs_CPUID(cpuInfo, 0);
        
        memcpy(vendorString, &cpuInfo[1], 4);
        memcpy(vendorString + 4, &cpuInfo[3], 4);
        memcpy(vendorString + 8, &cpuInfo[2], 4);
        vendorString[12] = '\0';
        
        std::string vendor(vendorString);
        
        if (vendor == "GenuineIntel") {
            cpuVendorCache = Exs_CPUVendor::Intel;
        } else if (vendor == "AuthenticAMD") {
            cpuVendorCache = Exs_CPUVendor::AMD;
        } else if (vendor.find("ARM") != std::string::npos) {
            cpuVendorCache = Exs_CPUVendor::ARM;
        } else if (vendor.find("Apple") != std::string::npos) {
            cpuVendorCache = Exs_CPUVendor::Apple;
        } else {
            cpuVendorCache = Exs_CPUVendor::Unknown;
        }
        
        return cpuVendorCache;
    }
    
    std::string getCPUVendorString() const override {
        int32 cpuInfo[4] = {0};
        char vendorString[13] = {0};
        
        Exs_CPUID(cpuInfo, 0);
        
        memcpy(vendorString, &cpuInfo[1], 4);
        memcpy(vendorString + 4, &cpuInfo[3], 4);
        memcpy(vendorString + 8, &cpuInfo[2], 4);
        vendorString[12] = '\0';
        
        return std::string(vendorString);
    }
    
    std::string getCPUFamily() const override {
        int32 cpuInfo[4] = {0};
        Exs_CPUID(cpuInfo, 1);
        
        uint32 family = ((cpuInfo[0] >> 8) & 0xF) | ((cpuInfo[0] >> 16) & 0xFF0);
        uint32 model = ((cpuInfo[0] >> 4) & 0xF) | ((cpuInfo[0] >> 12) & 0xF0);
        
        std::stringstream ss;
        ss << "Family " << family << " Model " << std::hex << model;
        return ss.str();
    }
    
    std::string getCPUModel() const override {
        int32 cpuInfo[4] = {0};
        Exs_CPUID(cpuInfo, 1);
        
        uint32 model = ((cpuInfo[0] >> 4) & 0xF) | ((cpuInfo[0] >> 12) & 0xF0);
        uint32 stepping = cpuInfo[0] & 0xF;
        
        std::stringstream ss;
        ss << "Model " << std::hex << model << " Stepping " << stepping;
        return ss.str();
    }
    
    std::string getCPUStepping() const override {
        int32 cpuInfo[4] = {0};
        Exs_CPUID(cpuInfo, 1);
        
        uint32 stepping = cpuInfo[0] & 0xF;
        return std::to_string(stepping);
    }
    
    uint32 getPhysicalCoreCount() const override {
        DWORD byteOffset = 0;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = nullptr;
        DWORD bufferSize = 0;
        
        GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return 0;
        }
        
        buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
        if (!buffer) {
            return 0;
        }
        
        uint32 coreCount = 0;
        if (GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &bufferSize)) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
            
            while (byteOffset < bufferSize) {
                if (ptr->Relationship == RelationProcessorCore) {
                    coreCount++;
                }
                
                byteOffset += ptr->Size;
                ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((LPBYTE)buffer + byteOffset);
            }
        }
        
        free(buffer);
        return coreCount;
    }
    
    uint32 getLogicalCoreCount() const override {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwNumberOfProcessors;
    }
    
    uint32 getSocketCount() const override {
        DWORD byteOffset = 0;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = nullptr;
        DWORD bufferSize = 0;
        
        GetLogicalProcessorInformationEx(RelationProcessorPackage, nullptr, &bufferSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return 0;
        }
        
        buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
        if (!buffer) {
            return 0;
        }
        
        uint32 socketCount = 0;
        if (GetLogicalProcessorInformationEx(RelationProcessorPackage, buffer, &bufferSize)) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
            
            while (byteOffset < bufferSize) {
                if (ptr->Relationship == RelationProcessorPackage) {
                    socketCount++;
                }
                
                byteOffset += ptr->Size;
                ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((LPBYTE)buffer + byteOffset);
            }
        }
        
        free(buffer);
        return socketCount;
    }
    
    uint32 getNumaNodeCount() const override {
        ULONG highestNodeNumber;
        if (GetNumaHighestNodeNumber(&highestNodeNumber)) {
            return highestNodeNumber + 1;
        }
        return 1;
    }
    
    uint32 getBaseFrequencyMHz() const override {
        HKEY hKey;
        DWORD freqMHz = 0;
        DWORD dataSize = sizeof(freqMHz);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"~MHz", nullptr, nullptr, 
                           (LPBYTE)&freqMHz, &dataSize);
            RegCloseKey(hKey);
        }
        
        return freqMHz;
    }
    
    uint32 getMaxTurboFrequencyMHz() const override {
        // Try to get from WMI
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            return 0;
        }
        
        uint32 maxFreq = 0;
        
        IWbemLocator* pLoc = nullptr;
        hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
                               IID_IWbemLocator, (LPVOID*)&pLoc);
        
        if (SUCCEEDED(hres)) {
            IWbemServices* pSvc = nullptr;
            hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 
                                      0, 0, 0, 0, &pSvc);
            
            if (SUCCEEDED(hres)) {
                hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, 
                                        nullptr, RPC_C_AUTHN_LEVEL_CALL, 
                                        RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
                
                if (SUCCEEDED(hres)) {
                    IEnumWbemClassObject* pEnumerator = nullptr;
                    hres = pSvc->ExecQuery(bstr_t("WQL"), 
                                          bstr_t("SELECT MaxClockSpeed FROM Win32_Processor"),
                                          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                                          nullptr, &pEnumerator);
                    
                    if (SUCCEEDED(hres)) {
                        IWbemClassObject* pclsObj = nullptr;
                        ULONG uReturn = 0;
                        
                        while (pEnumerator) {
                            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
                                                         &pclsObj, &uReturn);
                            if (0 == uReturn) break;
                            
                            VARIANT vtProp;
                            hr = pclsObj->Get(L"MaxClockSpeed", 0, &vtProp, 0, 0);
                            
                            if (SUCCEEDED(hr)) {
                                maxFreq = vtProp.uintVal;
                                VariantClear(&vtProp);
                            }
                            
                            pclsObj->Release();
                        }
                        
                        pEnumerator->Release();
                    }
                }
                pSvc->Release();
            }
            pLoc->Release();
        }
        
        CoUninitialize();
        return maxFreq;
    }
    
    uint32 getCurrentFrequencyMHz() const override {
        LARGE_INTEGER freq;
        if (QueryPerformanceFrequency(&freq)) {
            // This is not accurate for CPU frequency, but gives an indication
            return static_cast<uint32>(freq.QuadPart / 1000000);
        }
        return 0;
    }
    
    std::vector<Exs_CPUCacheInfo> getCacheInfo() const override {
        std::vector<Exs_CPUCacheInfo> cacheInfo;
        
        DWORD bufferSize = 0;
        GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return cacheInfo;
        }
        
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = 
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
        if (!buffer) {
            return cacheInfo;
        }
        
        if (GetLogicalProcessorInformationEx(RelationCache, buffer, &bufferSize)) {
            DWORD byteOffset = 0;
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
            
            while (byteOffset < bufferSize) {
                if (ptr->Relationship == RelationCache) {
                    Exs_CPUCacheInfo info;
                    info.level = ptr->Cache.Level;
                    info.sizeKB = ptr->Cache.CacheSize;
                    
                    switch (ptr->Cache.Type) {
                        case CacheUnified: info.type = "Unified"; break;
                        case CacheInstruction: info.type = "Instruction"; break;
                        case CacheData: info.type = "Data"; break;
                        case CacheTrace: info.type = "Trace"; break;
                        default: info.type = "Unknown";
                    }
                    
                    cacheInfo.push_back(info);
                }
                
                byteOffset += ptr->Size;
                ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((LPBYTE)buffer + byteOffset);
            }
        }
        
        free(buffer);
        return cacheInfo;
    }
    
    uint32 getCacheSize(uint32 level, const std::string& type) const override {
        auto cacheInfo = getCacheInfo();
        for (const auto& cache : cacheInfo) {
            if (cache.level == level && cache.type == type) {
                return cache.sizeKB;
            }
        }
        return 0;
    }
    
    std::vector<Exs_CPUCoreInfo> getCoreInfo() const override {
        std::vector<Exs_CPUCoreInfo> coreInfo;
        
        DWORD bufferSize = 0;
        GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return coreInfo;
        }
        
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = 
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
        if (!buffer) {
            return coreInfo;
        }
        
        if (GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &bufferSize)) {
            DWORD byteOffset = 0;
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
            uint32 coreIndex = 0;
            
            while (byteOffset < bufferSize) {
                if (ptr->Relationship == RelationProcessorCore) {
                    Exs_CPUCoreInfo info;
                    info.coreId = coreIndex;
                    info.physicalId = coreIndex;
                    info.socketId = 0; // Would need additional info
                    info.numaNodeId = 0; // Would need additional info
                    info.maxFrequencyMHz = getMaxTurboFrequencyMHz();
                    info.currentFrequencyMHz = getCurrentFrequencyMHz();
                    info.temperatureCelsius = getCPUTemperature();
                    info.utilizationPercentage = getCoreUsage(coreIndex);
                    info.isHyperThread = (ptr->Processor.GroupCount > 1);
                    
                    coreInfo.push_back(info);
                    coreIndex++;
                }
                
                byteOffset += ptr->Size;
                ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((LPBYTE)buffer + byteOffset);
            }
        }
        
        free(buffer);
        return coreInfo;
    }
    
    Exs_CPUCoreInfo getCoreInfoById(uint32 coreId) const override {
        auto cores = getCoreInfo();
        if (coreId < cores.size()) {
            return cores[coreId];
        }
        return Exs_CPUCoreInfo();
    }
    
    Exs_CPUFeatures getCPUFeatures() const override {
        if (featuresInitialized) {
            return cpuFeaturesCache;
        }
        
        int32 cpuInfo[4] = {0};
        
        // Get standard features
        Exs_CPUID(cpuInfo, 1);
        
        cpuFeaturesCache.mmx    = (cpuInfo[3] & (1 << 23)) != 0;
        cpuFeaturesCache.sse    = (cpuInfo[3] & (1 << 25)) != 0;
        cpuFeaturesCache.sse2   = (cpuInfo[3] & (1 << 26)) != 0;
        cpuFeaturesCache.sse3   = (cpuInfo[2] & (1 << 0)) != 0;
        cpuFeaturesCache.ssse3  = (cpuInfo[2] & (1 << 9)) != 0;
        cpuFeaturesCache.sse4_1 = (cpuInfo[2] & (1 << 19)) != 0;
        cpuFeaturesCache.sse4_2 = (cpuInfo[2] & (1 << 20)) != 0;
        cpuFeaturesCache.aes    = (cpuInfo[2] & (1 << 25)) != 0;
        
        // Check AVX
        bool osxsave = (cpuInfo[2] & (1 << 27)) != 0;
        cpuFeaturesCache.avx = (cpuInfo[2] & (1 << 28)) != 0 && osxsave;
        
        // Get extended features
        Exs_CPUID(cpuInfo, 7);
        
        cpuFeaturesCache.avx2 = (cpuInfo[1] & (1 << 5)) != 0;
        cpuFeaturesCache.avx512 = (cpuInfo[1] & (1 << 16)) != 0;
        cpuFeaturesCache.fma = (cpuInfo[1] & (1 << 12)) != 0;
        
        // Check virtualization
        cpuFeaturesCache.vmx = (cpuInfo[1] & (1 << 5)) != 0; // Actually for AVX2
        cpuFeaturesCache.svm = false; // AMD specific
        
        // Get additional info
        Exs_CPUID(cpuInfo, 0x80000001);
        cpuFeaturesCache.hypervisor = (cpuInfo[2] & (1 << 31)) != 0;
        
        featuresInitialized = true;
        return cpuFeaturesCache;
    }
    
    bool supportsFeature(const std::string& feature) const override {
        auto features = getCPUFeatures();
        
        if (feature == "SSE") return features.sse;
        if (feature == "SSE2") return features.sse2;
        if (feature == "SSE3") return features.sse3;
        if (feature == "SSSE3") return features.ssse3;
        if (feature == "SSE4.1") return features.sse4_1;
        if (feature == "SSE4.2") return features.sse4_2;
        if (feature == "AVX") return features.avx;
        if (feature == "AVX2") return features.avx2;
        if (feature == "AVX512") return features.avx512;
        if (feature == "FMA") return features.fma;
        if (feature == "AES") return features.aes;
        
        return false;
    }
    
    double getTotalCPUUsage() const override {
        if (!cpuQuery || !totalCpuCounter) {
            return 0.0;
        }
        
        PDH_FMT_COUNTERVALUE counterValue;
        PDH_STATUS status = PdhCollectQueryData(cpuQuery);
        if (status != ERROR_SUCCESS) {
            return 0.0;
        }
        
        status = PdhGetFormattedCounterValue(totalCpuCounter, PDH_FMT_DOUBLE, 
                                           nullptr, &counterValue);
        if (status == ERROR_SUCCESS) {
            return counterValue.doubleValue;
        }
        
        return 0.0;
    }
    
    double getCoreUsage(uint32 coreId) const override {
        if (coreId >= coreCounters.size() || !coreCounters[coreId]) {
            return 0.0;
        }
        
        PDH_FMT_COUNTERVALUE counterValue;
        PDH_STATUS status = PdhCollectQueryData(cpuQuery);
        if (status != ERROR_SUCCESS) {
            return 0.0;
        }
        
        status = PdhGetFormattedCounterValue(coreCounters[coreId], PDH_FMT_DOUBLE, 
                                           nullptr, &counterValue);
        if (status == ERROR_SUCCESS) {
            return counterValue.doubleValue;
        }
        
        return 0.0;
    }
    
    std::vector<double> getAllCoreUsage() const override {
        std::vector<double> usages;
        
        for (uint32 i = 0; i < coreCounters.size(); i++) {
            usages.push_back(getCoreUsage(i));
        }
        
        return usages;
    }
    
    int32 getCPUTemperature() const override {
        // Try to get temperature via WMI
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            return 0;
        }
        
        int32 temperature = 0;
        
        IWbemLocator* pLoc = nullptr;
        hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
                               IID_IWbemLocator, (LPVOID*)&pLoc);
        
        if (SUCCEEDED(hres)) {
            IWbemServices* pSvc = nullptr;
            hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), nullptr, nullptr, 
                                      0, 0, 0, 0, &pSvc);
            
            if (SUCCEEDED(hres)) {
                hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, 
                                        nullptr, RPC_C_AUTHN_LEVEL_CALL, 
                                        RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
                
                if (SUCCEEDED(hres)) {
                    IEnumWbemClassObject* pEnumerator = nullptr;
                    hres = pSvc->ExecQuery(bstr_t("WQL"), 
                                          bstr_t("SELECT CurrentTemperature FROM MSAcpi_ThermalZoneTemperature"),
                                          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                                          nullptr, &pEnumerator);
                    
                    if (SUCCEEDED(hres)) {
                        IWbemClassObject* pclsObj = nullptr;
                        ULONG uReturn = 0;
                        
                        while (pEnumerator) {
                            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
                                                         &pclsObj, &uReturn);
                            if (0 == uReturn) break;
                            
                            VARIANT vtProp;
                            hr = pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
                            
                            if (SUCCEEDED(hr)) {
                                // Temperature is in tenths of degrees Kelvin
                                temperature = (vtProp.intVal - 2732) / 10;
                                VariantClear(&vtProp);
                            }
                            
                            pclsObj->Release();
                        }
                        
                        pEnumerator->Release();
                    }
                }
                pSvc->Release();
            }
            pLoc->Release();
        }
        
        CoUninitialize();
        return temperature;
    }
    
    std::vector<int32> getCoreTemperatures() const override {
        // This would require hardware-specific implementations
        // For now, return the same temperature for all cores
        int32 temp = getCPUTemperature();
        uint32 coreCount = getLogicalCoreCount();
        
        std::vector<int32> temps(coreCount, temp);
        return temps;
    }
    
    double getCPUPowerUsage() const override {
        // Try to get via RAPL (Running Average Power Limit) if available
        // This requires specific CPU support
        return 0.0;
    }
    
    double getCPUPowerLimit() const override {
        return 0.0;
    }
    
    uint64 getInstructionsPerCycle() const override {
        // Would require performance counters
        return 0;
    }
    
    uint64 getTotalInstructions() const override {
        // Would require performance counters
        return 0;
    }
    
    uint64 getCacheMisses() const override {
        // Would require performance counters
        return 0;
    }
    
    uint64 getBranchMisses() const override {
        // Would require performance counters
        return 0;
    }
    
    uint64 getCycles() const override {
        // Would require performance counters
        return 0;
    }
    
    std::string getTopologyString() const override {
        std::stringstream ss;
        
        ss << "Physical Cores: " << getPhysicalCoreCount() << "\n";
        ss << "Logical Cores: " << getLogicalCoreCount() << "\n";
        ss << "Sockets: " << getSocketCount() << "\n";
        ss << "NUMA Nodes: " << getNumaNodeCount() << "\n";
        
        auto cacheInfo = getCacheInfo();
        for (const auto& cache : cacheInfo) {
            ss << "L" << cache.level << " " << cache.type 
               << " Cache: " << cache.sizeKB << " KB\n";
        }
        
        return ss.str();
    }
    
    bool supportsVirtualization() const override {
        int32 cpuInfo[4] = {0};
        Exs_CPUID(cpuInfo, 1);
        
        // Check VMX bit
        bool vmxSupported = (cpuInfo[2] & (1 << 5)) != 0;
        
        // Check for hypervisor presence
        Exs_CPUID(cpuInfo, 0x1);
        bool hypervisorBit = (cpuInfo[2] & (1 << 31)) != 0;
        
        return vmxSupported && !hypervisorBit;
    }
    
    bool supports64Bit() const override {
        int32 cpuInfo[4] = {0};
        Exs_CPUID(cpuInfo, 0x80000001);
        
        // Check LM bit (Long Mode)
        return (cpuInfo[3] & (1 << 29)) != 0;
    }
    
    bool supportsHyperThreading() const override {
        int32 cpuInfo[4] = {0};
        Exs_CPUID(cpuInfo, 1);
        
        // Check HTT bit
        return (cpuInfo[3] & (1 << 28)) != 0;
    }
    
private:
    void initializePerformanceCounters() {
        PDH_STATUS status = PdhOpenQuery(nullptr, 0, &cpuQuery);
        if (status != ERROR_SUCCESS) {
            cpuQuery = nullptr;
            return;
        }
        
        // Add total CPU counter
        status = PdhAddCounterW(cpuQuery, 
                               L"\\Processor(_Total)\\% Processor Time", 
                               0, &totalCpuCounter);
        
        // Add per-core counters
        uint32 coreCount = getLogicalCoreCount();
        coreCounters.resize(coreCount, nullptr);
        
        for (uint32 i = 0; i < coreCount; i++) {
            std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
            status = PdhAddCounterW(cpuQuery, counterPath.c_str(), 0, &coreCounters[i]);
        }
        
        // Collect initial data
        PdhCollectQueryData(cpuQuery);
    }
    
    void cleanupPerformanceCounters() {
        if (cpuQuery) {
            if (totalCpuCounter) {
                PdhRemoveCounter(totalCpuCounter);
            }
            
            for (auto& counter : coreCounters) {
                if (counter) {
                    PdhRemoveCounter(counter);
                }
            }
            
            PdhCloseQuery(cpuQuery);
            cpuQuery = nullptr;
        }
    }
};

// Factory function implementation
Exs_CPUInfoBase* Exs_CreateCPUInfoInstance() {
    return new Exs_CPUInfoWindows();
}

} // namespace CPUInfo
} // namespace Internal
} // namespace Exs
