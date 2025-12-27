// src/Core/Platform/internal/MemoryInfoBase.h
#ifndef EXS_INTERNAL_MEMORY_INFO_BASE_H
#define EXS_INTERNAL_MEMORY_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>

namespace Exs {
namespace Internal {
namespace MemoryInfo {

// Memory type enumeration
enum class Exs_MemoryType {
    Unknown = 0,
    DDR = 1,
    DDR2 = 2,
    DDR3 = 3,
    DDR4 = 4,
    DDR5 = 5,
    LPDDR = 6,
    LPDDR2 = 7,
    LPDDR3 = 8,
    LPDDR4 = 9,
    LPDDR5 = 10,
    GDDR = 11,
    HBM = 12,
    HBM2 = 13
};

// Memory module information
struct Exs_MemoryModuleInfo {
    uint32 slot;
    uint64 capacityBytes;
    Exs_MemoryType type;
    uint32 speedMHz;
    std::string manufacturer;
    std::string partNumber;
    std::string serialNumber;
    uint32 dataWidth;
    uint32 rankCount;
    bool isECC;
    bool isBuffered;
};

// Memory usage statistics
struct Exs_MemoryUsageStats {
    uint64 totalPhysical;
    uint64 availablePhysical;
    uint64 usedPhysical;
    uint64 totalPageFile;
    uint64 availablePageFile;
    uint64 usedPageFile;
    uint64 totalVirtual;
    uint64 availableVirtual;
    uint64 usedVirtual;
    uint64 cached;
    uint64 buffered;
    uint64 shared;
    double usagePercentage;
};

// Memory error information
struct Exs_MemoryErrorInfo {
    uint64 correctableErrors;
    uint64 uncorrectableErrors;
    uint64 lastErrorAddress;
    std::string lastErrorType;
    std::chrono::system_clock::time_point lastErrorTime;
};

// Base memory info class
class Exs_MemoryInfoBase {
public:
    virtual ~Exs_MemoryInfoBase() = default;
    
    // Physical memory information
    virtual uint64 getTotalPhysicalMemory() const = 0;
    virtual uint64 getAvailablePhysicalMemory() const = 0;
    virtual uint64 getUsedPhysicalMemory() const = 0;
    
    // Virtual memory information
    virtual uint64 getTotalVirtualMemory() const = 0;
    virtual uint64 getAvailableVirtualMemory() const = 0;
    virtual uint64 getUsedVirtualMemory() const = 0;
    
    // Page file information
    virtual uint64 getTotalPageFile() const = 0;
    virtual uint64 getAvailablePageFile() const = 0;
    virtual uint64 getUsedPageFile() const = 0;
    
    // Memory modules
    virtual std::vector<Exs_MemoryModuleInfo> getMemoryModules() const = 0;
    virtual uint32 getMemoryModuleCount() const = 0;
    
    // Memory type and speed
    virtual Exs_MemoryType getMemoryType() const = 0;
    virtual uint32 getMemorySpeed() const = 0;
    
    // Memory usage statistics
    virtual Exs_MemoryUsageStats getMemoryUsageStats() const = 0;
    
    // Cache memory
    virtual uint64 getL1CacheSize() const = 0;
    virtual uint64 getL2CacheSize() const = 0;
    virtual uint64 getL3CacheSize() const = 0;
    
    // Memory errors
    virtual Exs_MemoryErrorInfo getMemoryErrorInfo() const = 0;
    virtual bool hasMemoryErrors() const = 0;
    
    // Memory allocation information
    virtual uint64 getProcessMemoryUsage() const = 0;
    virtual uint64 getProcessPeakMemoryUsage() const = 0;
    virtual uint64 getProcessPrivateBytes() const = 0;
    virtual uint64 getProcessWorkingSet() const = 0;
    
    // Memory regions
    virtual std::vector<std::pair<uint64, uint64>> getMemoryRegions() const = 0;
    
    // Memory performance
    virtual double getMemoryBandwidth() const = 0;
    virtual uint64 getMemoryLatency() const = 0;
    
    // NUMA information
    virtual uint32 getNumaNodeCount() const = 0;
    virtual uint64 getNumaNodeMemory(uint32 node) const = 0;
    
    // Memory pressure
    virtual bool isMemoryPressureHigh() const = 0;
    virtual double getMemoryPressurePercentage() const = 0;
    
    // Memory fragmentation
    virtual double getMemoryFragmentation() const = 0;
    
    // Swap/Swapfile information
    virtual uint64 getSwapSize() const = 0;
    virtual uint64 getSwapUsed() const = 0;
    virtual double getSwapUsagePercentage() const = 0;
    
    // Memory commit limit
    virtual uint64 getCommitLimit() const = 0;
    virtual uint64 getCommittedMemory() const = 0;
};

// Factory function
Exs_MemoryInfoBase* Exs_CreateMemoryInfoInstance();

} // namespace MemoryInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_MEMORY_INFO_BASE_H
