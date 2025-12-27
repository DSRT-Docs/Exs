// src/Core/Platform/Windows/MemoryInfoWindows.cpp
#include "../internal/MemoryInfoBase.h"
#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <wbemidl.h>
#include <comdef.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "wbemuuid.lib")

namespace Exs {
namespace Internal {
namespace MemoryInfo {

class Exs_MemoryInfoWindows : public Exs_MemoryInfoBase {
private:
    mutable PDH_HQUERY memoryQuery = nullptr;
    mutable PDH_HCOUNTER availableBytesCounter = nullptr;
    mutable PDH_HCOUNTER committedBytesCounter = nullptr;
    
public:
    Exs_MemoryInfoWindows() {
        initializePerformanceCounters();
    }
    
    virtual ~Exs_MemoryInfoWindows() {
        cleanupPerformanceCounters();
    }
    
    uint64 getTotalPhysicalMemory() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullTotalPhys;
        }
        return 0;
    }
    
    uint64 getAvailablePhysicalMemory() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullAvailPhys;
        }
        return 0;
    }
    
    uint64 getUsedPhysicalMemory() const override {
        return getTotalPhysicalMemory() - getAvailablePhysicalMemory();
    }
    
    uint64 getTotalVirtualMemory() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullTotalVirtual;
        }
        return 0;
    }
    
    uint64 getAvailableVirtualMemory() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullAvailVirtual;
        }
        return 0;
    }
    
    uint64 getUsedVirtualMemory() const override {
        return getTotalVirtualMemory() - getAvailableVirtualMemory();
    }
    
    uint64 getTotalPageFile() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullTotalPageFile;
        }
        return 0;
    }
    
    uint64 getAvailablePageFile() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullAvailPageFile;
        }
        return 0;
    }
    
    uint64 getUsedPageFile() const override {
        return getTotalPageFile() - getAvailablePageFile();
    }
    
    std::vector<Exs_MemoryModuleInfo> getMemoryModules() const override {
        std::vector<Exs_MemoryModuleInfo> modules;
        
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            return modules;
        }
        
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
                                          bstr_t("SELECT * FROM Win32_PhysicalMemory"),
                                          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                                          nullptr, &pEnumerator);
                    
                    if (SUCCEEDED(hres)) {
                        IWbemClassObject* pclsObj = nullptr;
                        ULONG uReturn = 0;
                        
                        while (pEnumerator) {
                            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
                                                         &pclsObj, &uReturn);
                            if (0 == uReturn) break;
                            
                            Exs_MemoryModuleInfo module;
                            
                            // Get module properties
                            VARIANT vtProp;
                            
                            hr = pclsObj->Get(L"BankLabel", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                module.slot = _wtoi(vtProp.bstrVal);
                                VariantClear(&vtProp);
                            }
                            
                            hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                module.capacityBytes = vtProp.ullVal;
                                VariantClear(&vtProp);
                            }
                            
                            hr = pclsObj->Get(L"MemoryType", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                switch (vtProp.uintVal) {
                                    case 0: module.type = Exs_MemoryType::Unknown; break;
                                    case 20: module.type = Exs_MemoryType::DDR; break;
                                    case 21: module.type = Exs_MemoryType::DDR2; break;
                                    case 24: module.type = Exs_MemoryType::DDR3; break;
                                    case 26: module.type = Exs_MemoryType::DDR4; break;
                                    default: module.type = Exs_MemoryType::Unknown;
                                }
                                VariantClear(&vtProp);
                            }
                            
                            hr = pclsObj->Get(L"Speed", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                module.speedMHz = vtProp.uintVal;
                                VariantClear(&vtProp);
                            }
                            
                            hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                module.manufacturer = _com_util::ConvertBSTRToString(vtProp.bstrVal);
                                VariantClear(&vtProp);
                            }
                            
                            hr = pclsObj->Get(L"PartNumber", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                module.partNumber = _com_util::ConvertBSTRToString(vtProp.bstrVal);
                                VariantClear(&vtProp);
                            }
                            
                            hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                module.serialNumber = _com_util::ConvertBSTRToString(vtProp.bstrVal);
                                VariantClear(&vtProp);
                            }
                            
                            hr = pclsObj->Get(L"DataWidth", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                module.dataWidth = vtProp.uintVal;
                                VariantClear(&vtProp);
                            }
                            
                            modules.push_back(module);
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
        return modules;
    }
    
    uint32 getMemoryModuleCount() const override {
        auto modules = getMemoryModules();
        return static_cast<uint32>(modules.size());
    }
    
    Exs_MemoryType getMemoryType() const override {
        auto modules = getMemoryModules();
        if (!modules.empty()) {
            return modules[0].type;
        }
        return Exs_MemoryType::Unknown;
    }
    
    uint32 getMemorySpeed() const override {
        auto modules = getMemoryModules();
        if (!modules.empty()) {
            return modules[0].speedMHz;
        }
        return 0;
    }
    
    Exs_MemoryUsageStats getMemoryUsageStats() const override {
        Exs_MemoryUsageStats stats;
        
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            stats.totalPhysical = memStatus.ullTotalPhys;
            stats.availablePhysical = memStatus.ullAvailPhys;
            stats.usedPhysical = stats.totalPhysical - stats.availablePhysical;
            stats.totalPageFile = memStatus.ullTotalPageFile;
            stats.availablePageFile = memStatus.ullAvailPageFile;
            stats.usedPageFile = stats.totalPageFile - stats.availablePageFile;
            stats.totalVirtual = memStatus.ullTotalVirtual;
            stats.availableVirtual = memStatus.ullAvailVirtual;
            stats.usedVirtual = stats.totalVirtual - stats.availableVirtual;
            stats.usagePercentage = memStatus.dwMemoryLoad;
        }
        
        // Get additional stats via performance counters
        if (memoryQuery && availableBytesCounter && committedBytesCounter) {
            PDH_FMT_COUNTERVALUE counterValue;
            PDH_STATUS status = PdhCollectQueryData(memoryQuery);
            
            if (status == ERROR_SUCCESS) {
                status = PdhGetFormattedCounterValue(availableBytesCounter, PDH_FMT_LARGE, 
                                                   nullptr, &counterValue);
                if (status == ERROR_SUCCESS) {
                    stats.cached = counterValue.largeValue;
                }
                
                status = PdhGetFormattedCounterValue(committedBytesCounter, PDH_FMT_LARGE, 
                                                   nullptr, &counterValue);
                if (status == ERROR_SUCCESS) {
                    stats.buffered = counterValue.largeValue;
                }
            }
        }
        
        return stats;
    }
    
    uint64 getL1CacheSize() const override {
        // Get from CPUID or WMI
        DWORD bufferSize = 0;
        GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
        
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = 
                (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
            
            if (buffer && GetLogicalProcessorInformationEx(RelationCache, buffer, &bufferSize)) {
                DWORD byteOffset = 0;
                PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
                uint64 totalL1 = 0;
                
                while (byteOffset < bufferSize) {
                    if (ptr->Relationship == RelationCache && ptr->Cache.Level == 1) {
                        totalL1 += ptr->Cache.CacheSize;
                    }
                    byteOffset += ptr->Size;
                    ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((LPBYTE)buffer + byteOffset);
                }
                
                free(buffer);
                return totalL1;
            }
            
            if (buffer) free(buffer);
        }
        
        return 0;
    }
    
    uint64 getL2CacheSize() const override {
        DWORD bufferSize = 0;
        GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
        
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = 
                (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
            
            if (buffer && GetLogicalProcessorInformationEx(RelationCache, buffer, &bufferSize)) {
                DWORD byteOffset = 0;
                PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
                uint64 totalL2 = 0;
                
                while (byteOffset < bufferSize) {
                    if (ptr->Relationship == RelationCache && ptr->Cache.Level == 2) {
                        totalL2 += ptr->Cache.CacheSize;
                    }
                    byteOffset += ptr->Size;
                    ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((LPBYTE)buffer + byteOffset);
                }
                
                free(buffer);
                return totalL2;
            }
            
            if (buffer) free(buffer);
        }
        
        return 0;
    }
    
    uint64 getL3CacheSize() const override {
        DWORD bufferSize = 0;
        GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
        
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = 
                (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
            
            if (buffer && GetLogicalProcessorInformationEx(RelationCache, buffer, &bufferSize)) {
                DWORD byteOffset = 0;
                PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
                uint64 totalL3 = 0;
                
                while (byteOffset < bufferSize) {
                    if (ptr->Relationship == RelationCache && ptr->Cache.Level == 3) {
                        totalL3 += ptr->Cache.CacheSize;
                    }
                    byteOffset += ptr->Size;
                    ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((LPBYTE)buffer + byteOffset);
                }
                
                free(buffer);
                return totalL3;
            }
            
            if (buffer) free(buffer);
        }
        
        return 0;
    }
    
    Exs_MemoryErrorInfo getMemoryErrorInfo() const override {
        Exs_MemoryErrorInfo info = {};
        
        // Try to get memory errors from WMI
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (SUCCEEDED(hres)) {
            IWbemLocator* pLoc = nullptr;
            hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
                                   IID_IWbemLocator, (LPVOID*)&pLoc);
            
            if (SUCCEEDED(hres)) {
                IWbemServices* pSvc = nullptr;
                hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), nullptr, nullptr, 
                                          0, 0, 0, 0, &pSvc);
                
                if (SUCCEEDED(hres)) {
                    IEnumWbemClassObject* pEnumerator = nullptr;
                    hres = pSvc->ExecQuery(bstr_t("WQL"), 
                                          bstr_t("SELECT * FROM MSMCAEvent_MemoryError"),
                                          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                                          nullptr, &pEnumerator);
                    
                    if (SUCCEEDED(hres)) {
                        IWbemClassObject* pclsObj = nullptr;
                        ULONG uReturn = 0;
                        uint64 errorCount = 0;
                        
                        while (pEnumerator) {
                            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
                                                         &pclsObj, &uReturn);
                            if (0 == uReturn) break;
                            
                            errorCount++;
                            
                            // Get error type
                            VARIANT vtProp;
                            hr = pclsObj->Get(L"ErrorSeverity", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                if (vtProp.uintVal == 3) { // Correctable
                                    info.correctableErrors++;
                                } else {
                                    info.uncorrectableErrors++;
                                }
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
            CoUninitialize();
        }
        
        return info;
    }
    
    bool hasMemoryErrors() const override {
        auto info = getMemoryErrorInfo();
        return (info.correctableErrors > 0) || (info.uncorrectableErrors > 0);
    }
    
    uint64 getProcessMemoryUsage() const override {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
        return 0;
    }
    
    uint64 getProcessPeakMemoryUsage() const override {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.PeakWorkingSetSize;
        }
        return 0;
    }
    
    uint64 getProcessPrivateBytes() const override {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.PrivateUsage;
        }
        return 0;
    }
    
    uint64 getProcessWorkingSet() const override {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
        return 0;
    }
    
    std::vector<std::pair<uint64, uint64>> getMemoryRegions() const override {
        std::vector<std::pair<uint64, uint64>> regions;
        
        // Enumerate process memory regions
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        MEMORY_BASIC_INFORMATION mbi;
        uint64 address = 0;
        
        while (VirtualQueryEx(GetCurrentProcess(), (LPCVOID)address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT || mbi.State == MEM_RESERVE) {
                regions.push_back({reinterpret_cast<uint64>(mbi.BaseAddress), 
                                  reinterpret_cast<uint64>(mbi.BaseAddress) + mbi.RegionSize});
            }
            
            address += mbi.RegionSize;
            if (address >= reinterpret_cast<uint64>(sysInfo.lpMaximumApplicationAddress)) {
                break;
            }
        }
        
        return regions;
    }
    
    double getMemoryBandwidth() const override {
        // This requires specific performance counters or hardware monitoring
        // For now, return 0
        return 0.0;
    }
    
    uint64 getMemoryLatency() const override {
        // This requires specific benchmarks
        return 0;
    }
    
    uint32 getNumaNodeCount() const override {
        ULONG highestNodeNumber;
        if (GetNumaHighestNodeNumber(&highestNodeNumber)) {
            return highestNodeNumber + 1;
        }
        return 1;
    }
    
    uint64 getNumaNodeMemory(uint32 node) const override {
        ULONGLONG availableBytes;
        if (GetNumaAvailableMemoryNodeEx(node, &availableBytes)) {
            return availableBytes;
        }
        return 0;
    }
    
    bool isMemoryPressureHigh() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.dwMemoryLoad > 90; // Over 90% usage
        }
        return false;
    }
    
    double getMemoryPressurePercentage() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return static_cast<double>(memStatus.dwMemoryLoad);
        }
        return 0.0;
    }
    
    double getMemoryFragmentation() const override {
        // Calculate fragmentation based on free memory blocks
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (!GlobalMemoryStatusEx(&memStatus)) {
            return 0.0;
        }
        
        uint64 totalFree = memStatus.ullAvailPhys;
        if (totalFree == 0) return 0.0;
        
        // Count free memory blocks
        uint32 freeBlocks = 0;
        MEMORY_BASIC_INFORMATION mbi;
        uint64 address = 0;
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        while (VirtualQueryEx(GetCurrentProcess(), (LPCVOID)address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_FREE) {
                freeBlocks++;
            }
            
            address += mbi.RegionSize;
            if (address >= reinterpret_cast<uint64>(sysInfo.lpMaximumApplicationAddress)) {
                break;
            }
        }
        
        if (freeBlocks == 0) return 0.0;
        
        // Simple fragmentation metric
        double avgBlockSize = static_cast<double>(totalFree) / freeBlocks;
        double maxPossibleBlock = totalFree; // If all free memory was contiguous
        
        return (maxPossibleBlock - avgBlockSize) / maxPossibleBlock * 100.0;
    }
    
    uint64 getSwapSize() const override {
        return getTotalPageFile();
    }
    
    uint64 getSwapUsed() const override {
        return getUsedPageFile();
    }
    
    double getSwapUsagePercentage() const override {
        uint64 total = getSwapSize();
        uint64 used = getSwapUsed();
        
        if (total == 0) return 0.0;
        return static_cast<double>(used) / total * 100.0;
    }
    
    uint64 getCommitLimit() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullTotalPageFile;
        }
        return 0;
    }
    
    uint64 getCommittedMemory() const override {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        
        if (GlobalMemoryStatusEx(&memStatus)) {
            return memStatus.ullTotalPageFile - memStatus.ullAvailPageFile;
        }
        return 0;
    }
    
private:
    void initializePerformanceCounters() {
        PDH_STATUS status = PdhOpenQuery(nullptr, 0, &memoryQuery);
        if (status != ERROR_SUCCESS) {
            memoryQuery = nullptr;
            return;
        }
        
        // Add available bytes counter
        status = PdhAddCounterW(memoryQuery, 
                               L"\\Memory\\Available Bytes", 
                               0, &availableBytesCounter);
        
        // Add committed bytes counter
        status = PdhAddCounterW(memoryQuery, 
                               L"\\Memory\\Committed Bytes", 
                               0, &committedBytesCounter);
        
        // Collect initial data
        PdhCollectQueryData(memoryQuery);
    }
    
    void cleanupPerformanceCounters() {
        if (memoryQuery) {
            if (availableBytesCounter) {
                PdhRemoveCounter(availableBytesCounter);
            }
            if (committedBytesCounter) {
                PdhRemoveCounter(committedBytesCounter);
            }
            
            PdhCloseQuery(memoryQuery);
            memoryQuery = nullptr;
        }
    }
};

// Factory function implementation
Exs_MemoryInfoBase* Exs_CreateMemoryInfoInstance() {
    return new Exs_MemoryInfoWindows();
}

} // namespace MemoryInfo
} // namespace Internal
} // namespace Exs
