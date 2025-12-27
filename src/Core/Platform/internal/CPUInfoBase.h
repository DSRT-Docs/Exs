// src/Core/Platform/internal/CPUInfoBase.h
#ifndef EXS_INTERNAL_CPU_INFO_BASE_H
#define EXS_INTERNAL_CPU_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Exs {
namespace Internal {
namespace CPUInfo {

// CPU Cache information
struct Exs_CPUCacheInfo {
    uint32 level;
    uint32 sizeKB;
    uint32 lineSize;
    uint32 associativity;
    std::string type; // "Data", "Instruction", "Unified"
};

// CPU Core information
struct Exs_CPUCoreInfo {
    uint32 coreId;
    uint32 physicalId;
    uint32 socketId;
    uint32 numaNodeId;
    uint32 maxFrequencyMHz;
    uint32 currentFrequencyMHz;
    uint32 temperatureCelsius;
    double utilizationPercentage;
    bool isHyperThread;
};

// CPU Vendor enumeration
enum class Exs_CPUVendor {
    Unknown = 0,
    Intel = 1,
    AMD = 2,
    ARM = 3,
    Apple = 4,
    Qualcomm = 5,
    Samsung = 6,
    IBM = 7,
    VIA = 8
};

// CPU Feature flags
struct Exs_CPUFeatures {
    // x86/x64 features
    bool mmx : 1;
    bool sse : 1;
    bool sse2 : 1;
    bool sse3 : 1;
    bool ssse3 : 1;
    bool sse4_1 : 1;
    bool sse4_2 : 1;
    bool avx : 1;
    bool avx2 : 1;
    bool avx512 : 1;
    bool fma : 1;
    bool aes : 1;
    
    // ARM features
    bool neon : 1;
    bool crypto : 1;
    bool fp16 : 1;
    bool asimd : 1;
    
    // Virtualization
    bool vmx : 1;
    bool svm : 1;
    bool hypervisor : 1;
    
    // Security
    bool sgx : 1;
    bool tpm : 1;
    bool mte : 1;
    
    // Power management
    bool speedStep : 1;
    bool turboBoost : 1;
    bool powerNow : 1;
};

// Base CPU info class
class Exs_CPUInfoBase {
public:
    virtual ~Exs_CPUInfoBase() = default;
    
    // CPU identification
    virtual std::string getCPUName() const = 0;
    virtual Exs_CPUVendor getCPUVendor() const = 0;
    virtual std::string getCPUVendorString() const = 0;
    virtual std::string getCPUFamily() const = 0;
    virtual std::string getCPUModel() const = 0;
    virtual std::string getCPUStepping() const = 0;
    
    // CPU capabilities
    virtual uint32 getPhysicalCoreCount() const = 0;
    virtual uint32 getLogicalCoreCount() const = 0;
    virtual uint32 getSocketCount() const = 0;
    virtual uint32 getNumaNodeCount() const = 0;
    
    // CPU frequency
    virtual uint32 getBaseFrequencyMHz() const = 0;
    virtual uint32 getMaxTurboFrequencyMHz() const = 0;
    virtual uint32 getCurrentFrequencyMHz() const = 0;
    
    // CPU cache
    virtual std::vector<Exs_CPUCacheInfo> getCacheInfo() const = 0;
    virtual uint32 getCacheSize(uint32 level, const std::string& type) const = 0;
    
    // CPU cores
    virtual std::vector<Exs_CPUCoreInfo> getCoreInfo() const = 0;
    virtual Exs_CPUCoreInfo getCoreInfoById(uint32 coreId) const = 0;
    
    // CPU features
    virtual Exs_CPUFeatures getCPUFeatures() const = 0;
    virtual bool supportsFeature(const std::string& feature) const = 0;
    
    // CPU utilization
    virtual double getTotalCPUUsage() const = 0;
    virtual double getCoreUsage(uint32 coreId) const = 0;
    virtual std::vector<double> getAllCoreUsage() const = 0;
    
    // CPU temperature
    virtual int32 getCPUTemperature() const = 0;
    virtual std::vector<int32> getCoreTemperatures() const = 0;
    
    // CPU power
    virtual double getCPUPowerUsage() const = 0;
    virtual double getCPUPowerLimit() const = 0;
    
    // CPU instructions
    virtual uint64 getInstructionsPerCycle() const = 0;
    virtual uint64 getTotalInstructions() const = 0;
    
    // CPU events (counters)
    virtual uint64 getCacheMisses() const = 0;
    virtual uint64 getBranchMisses() const = 0;
    virtual uint64 getCycles() const = 0;
    
    // CPU topology
    virtual std::string getTopologyString() const = 0;
    
    // CPU capabilities check
    virtual bool supportsVirtualization() const = 0;
    virtual bool supports64Bit() const = 0;
    virtual bool supportsHyperThreading() const = 0;
};

// Factory function
Exs_CPUInfoBase* Exs_CreateCPUInfoInstance();

} // namespace CPUInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_CPU_INFO_BASE_H
