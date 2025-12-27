// src/Core/Platform/internal/PerformanceInfoBase.h
#ifndef EXS_INTERNAL_PERFORMANCE_INFO_BASE_H
#define EXS_INTERNAL_PERFORMANCE_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace Exs {
namespace Internal {
namespace PerformanceInfo {

// Performance counter types
enum class Exs_PerformanceCounterType {
    Unknown = 0,
    CPUUsage = 1,
    MemoryUsage = 2,
    DiskUsage = 3,
    NetworkUsage = 4,
    GPUUsage = 5,
    ProcessCount = 6,
    ThreadCount = 7,
    HandleCount = 8,
    Uptime = 9,
    Temperature = 10
};

// Performance threshold types
enum class Exs_PerformanceThresholdType {
    Unknown = 0,
    Warning = 1,
    Critical = 2,
    Maximum = 3,
    Minimum = 4
};

// Performance counter information
struct Exs_PerformanceCounterInfo {
    std::string name;
    std::string description;
    Exs_PerformanceCounterType type;
    std::string category;
    std::string instance;
    
    // Value information
    double value;
    double minValue;
    double maxValue;
    double averageValue;
    double standardDeviation;
    
    // Timestamps
    std::chrono::system_clock::time_point firstSampleTime;
    std::chrono::system_clock::time_point lastSampleTime;
    
    // Sampling information
    uint32 sampleCount;
    uint32 sampleInterval; // milliseconds
    
    // Thresholds
    double warningThreshold;
    double criticalThreshold;
    double maximumThreshold;
    double minimumThreshold;
    
    // Units
    std::string unit;
    uint32 scale; // scaling factor
};

// Performance alert information
struct Exs_PerformanceAlert {
    std::string id;
    std::string counterName;
    Exs_PerformanceThresholdType thresholdType;
    double thresholdValue;
    double actualValue;
    std::chrono::system_clock::time_point alertTime;
    std::string message;
    bool isAcknowledged;
    bool isResolved;
};

// Process performance information
struct Exs_ProcessPerformanceInfo {
    uint32 processId;
    std::string processName;
    std::string executablePath;
    
    // CPU usage
    double cpuUsage;              // percentage
    uint64 cpuTime;               // milliseconds
    uint64 userTime;              // milliseconds
    uint64 kernelTime;            // milliseconds
    uint32 priority;
    uint32 affinityMask;
    
    // Memory usage
    uint64 memoryUsage;           // bytes
    uint64 peakMemoryUsage;       // bytes
    uint64 workingSet;            // bytes
    uint64 privateBytes;          // bytes
    uint64 virtualBytes;          // bytes
    uint64 pagedPool;             // bytes
    uint64 nonPagedPool;          // bytes
    
    // I/O usage
    uint64 readOperationCount;
    uint64 writeOperationCount;
    uint64 readTransferCount;     // bytes
    uint64 writeTransferCount;    // bytes
    
    // Thread information
    uint32 threadCount;
    uint64 contextSwitches;
    
    // Handle information
    uint32 handleCount;
    
    // GPU usage (if applicable)
    double gpuUsage;              // percentage
    uint64 gpuMemoryUsage;        // bytes
    
    // Energy usage
    double energyUsage;           // joules
    
    // State information
    bool isResponding;
    bool isElevated;
    std::string state;
};

// System performance information
struct Exs_SystemPerformanceInfo {
    // CPU performance
    double totalCpuUsage;         // percentage
    std::vector<double> coreUsages; // per-core percentages
    uint32 contextSwitchesPerSec;
    uint32 interruptsPerSec;
    
    // Memory performance
    double memoryUsage;           // percentage
    uint64 availableMemory;       // bytes
    uint64 cachedMemory;          // bytes
    uint64 freeMemory;            // bytes
    uint64 totalMemory;           // bytes
    uint32 pageFaultsPerSec;
    uint32 pageReadsPerSec;
    
    // Disk performance
    std::vector<double> diskUsages; // per-disk percentages
    uint64 diskReadBytesPerSec;
    uint64 diskWriteBytesPerSec;
    uint32 diskReadOperationsPerSec;
    uint32 diskWriteOperationsPerSec;
    uint32 diskQueueLength;
    
    // Network performance
    uint64 networkReceivedBytesPerSec;
    uint64 networkSentBytesPerSec;
    uint32 networkReceivedPacketsPerSec;
    uint32 networkSentPacketsPerSec;
    uint32 networkErrorsPerSec;
    
    // GPU performance
    double gpuUsage;              // percentage
    double gpuMemoryUsage;        // percentage
    uint32 gpuTemperature;        // Celsius
    uint32 gpuFanSpeed;           // RPM
    
    // Process and thread counts
    uint32 processCount;
    uint32 threadCount;
    uint32 handleCount;
    
    // System responsiveness
    double systemResponsiveness;  // percentage (0-100)
    uint32 uptime;                // seconds
    
    // Power performance
    double powerUsage;            // Watts
    double energyEfficiency;      // Performance per Watt
    
    // Thermal performance
    std::vector<int32> temperatures; // Celsius
    std::vector<uint32> fanSpeeds;   // RPM
};

// Performance benchmark result
struct Exs_PerformanceBenchmarkResult {
    std::string benchmarkName;
    std::string category;
    
    // Scores
    double score;
    double normalizedScore;       // 0-100
    uint32 ranking;               // 1-based
    
    // Timing information
    std::chrono::milliseconds duration;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    
    // Resource usage during benchmark
    double averageCpuUsage;
    double peakCpuUsage;
    uint64 averageMemoryUsage;
    uint64 peakMemoryUsage;
    double averageGpuUsage;
    double peakGpuUsage;
    
    // Performance metrics
    uint64 operationsPerSecond;
    double latency;               // milliseconds
    double throughput;            // MB/s
    double efficiency;            // operations per watt
    
    // Comparison data
    double baselineScore;
    double improvementPercentage;
    bool meetsRequirements;
    
    // Detailed results
    std::vector<std::pair<std::string, double>> detailedMetrics;
};

// Performance callback type
using Exs_PerformanceCallback = std::function<void(const Exs_PerformanceAlert& alert)>;

// Base performance info class
class Exs_PerformanceInfoBase {
public:
    virtual ~Exs_PerformanceInfoBase() = default;
    
    // Performance counters
    virtual std::vector<Exs_PerformanceCounterInfo> getPerformanceCounters() const = 0;
    virtual Exs_PerformanceCounterInfo getPerformanceCounter(const std::string& name) const = 0;
    virtual bool addPerformanceCounter(const std::string& name, Exs_PerformanceCounterType type) = 0;
    virtual bool removePerformanceCounter(const std::string& name) = 0;
    virtual bool startMonitoringCounter(const std::string& name) = 0;
    virtual bool stopMonitoringCounter(const std::string& name) = 0;
    
    // Counter values
    virtual double getCounterValue(const std::string& name) const = 0;
    virtual std::vector<double> getCounterHistory(const std::string& name, uint32 sampleCount) const = 0;
    virtual bool resetCounter(const std::string& name) = 0;
    
    // Thresholds and alerts
    virtual bool setCounterThreshold(const std::string& name, Exs_PerformanceThresholdType type, double value) = 0;
    virtual double getCounterThreshold(const std::string& name, Exs_PerformanceThresholdType type) const = 0;
    virtual std::vector<Exs_PerformanceAlert> getPerformanceAlerts() const = 0;
    virtual bool acknowledgeAlert(const std::string& alertId) = 0;
    virtual bool resolveAlert(const std::string& alertId) = 0;
    
    // System performance
    virtual Exs_SystemPerformanceInfo getSystemPerformance() const = 0;
    virtual double getSystemPerformanceScore() const = 0; // 0-100
    virtual bool isSystemUnderHighLoad() const = 0;
    virtual uint32 getSystemStabilityIndex() const = 0; // 0-100
    
    // Process performance
    virtual std::vector<Exs_ProcessPerformanceInfo> getProcessPerformance() const = 0;
    virtual Exs_ProcessPerformanceInfo getProcessPerformance(uint32 processId) const = 0;
    virtual std::vector<Exs_ProcessPerformanceInfo> getTopProcessesByCPU(uint32 count) const = 0;
    virtual std::vector<Exs_ProcessPerformanceInfo> getTopProcessesByMemory(uint32 count) const = 0;
    virtual std::vector<Exs_ProcessPerformanceInfo> getTopProcessesByIO(uint32 count) const = 0;
    
    // Resource usage
    virtual double getCPUUsage() const = 0;
    virtual std::vector<double> getCPUUsagePerCore() const = 0;
    virtual double getMemoryUsage() const = 0;
    virtual double getDiskUsage() const = 0;
    virtual double getNetworkUsage() const = 0;
    virtual double getGPUUsage() const = 0;
    
    // Performance monitoring
    virtual bool startPerformanceMonitoring() = 0;
    virtual bool stopPerformanceMonitoring() = 0;
    virtual bool isPerformanceMonitoring() const = 0;
    virtual void setMonitoringInterval(uint32 milliseconds) = 0;
    virtual uint32 getMonitoringInterval() const = 0;
    
    // Performance logging
    virtual bool startPerformanceLogging(const std::string& logFilePath) = 0;
    virtual bool stopPerformanceLogging() = 0;
    virtual bool isPerformanceLogging() const = 0;
    virtual std::string getPerformanceLog() const = 0;
    
    // Performance analysis
    virtual std::vector<std::string> analyzePerformanceBottlenecks() const = 0;
    virtual std::vector<std::string> getPerformanceRecommendations() const = 0;
    virtual bool optimizePerformance() = 0;
    virtual bool resetPerformanceSettings() = 0;
    
    // Benchmarking
    virtual Exs_PerformanceBenchmarkResult runBenchmark(const std::string& benchmarkName) = 0;
    virtual std::vector<Exs_PerformanceBenchmarkResult> runAllBenchmarks() = 0;
    virtual bool compareBenchmarkResults(const std::string& benchmark1, const std::string& benchmark2) const = 0;
    virtual std::vector<std::string> getAvailableBenchmarks() const = 0;
    
    // Performance trends
    virtual std::vector<double> getPerformanceTrend(Exs_PerformanceCounterType type, uint32 hours) const = 0;
    virtual double getPerformanceImprovement(Exs_PerformanceCounterType type, uint32 days) const = 0;
    virtual bool isPerformanceImproving(Exs_PerformanceCounterType type) const = 0;
    
    // Resource limits
    virtual bool setCPULimit(uint32 processId, double percentage) = 0;
    virtual bool setMemoryLimit(uint32 processId, uint64 bytes) = 0;
    virtual bool setIOLimit(uint32 processId, uint64 bytesPerSec) = 0;
    virtual bool removeResourceLimits(uint32 processId) = 0;
    
    // Priority management
    virtual bool setProcessPriority(uint32 processId, uint32 priority) = 0;
    virtual uint32 getProcessPriority(uint32 processId) const = 0;
    virtual bool setProcessAffinity(uint32 processId, uint64 affinityMask) = 0;
    virtual uint64 getProcessAffinity(uint32 processId) const = 0;
    
    // Performance callbacks
    virtual void registerPerformanceCallback(const Exs_PerformanceCallback& callback) = 0;
    virtual void unregisterPerformanceCallback() = 0;
    
    // Performance reports
    virtual std::string generatePerformanceReport() const = 0;
    virtual bool savePerformanceReport(const std::string& filePath) const = 0;
    virtual std::string generatePerformanceSummary() const = 0;
    
    // System health check
    virtual bool performSystemHealthCheck() const = 0;
    virtual std::vector<std::string> getSystemHealthIssues() const = 0;
    virtual bool fixSystemHealthIssues() = 0;
    
    // Power and performance profiles
    virtual bool setPerformanceProfile(const std::string& profile) = 0; // "Power Saver", "Balanced", "High Performance"
    virtual std::string getPerformanceProfile() const = 0;
    virtual bool createCustomPerformanceProfile(const std::string& name, const std::vector<std::string>& settings) = 0;
    
    // Thermal management
    virtual bool setThermalPolicy(const std::string& policy) = 0; // "Cool", "Quiet", "Performance"
    virtual std::string getThermalPolicy() const = 0;
    virtual std::vector<int32> getTemperatures() const = 0;
    virtual std::vector<uint32> getFanSpeeds() const = 0;
    
    // Performance tuning
    virtual bool tunePerformance(const std::vector<std::string>& parameters) = 0;
    virtual bool resetPerformanceTuning() = 0;
    virtual std::vector<std::string> getTuningParameters() const = 0;
    
    // Real-time performance monitoring
    virtual bool startRealTimeMonitoring() = 0;
    virtual bool stopRealTimeMonitoring() = 0;
    virtual std::vector<std::pair<std::string, double>> getRealTimeMetrics() const = 0;
    
    // Performance prediction
    virtual double predictPerformance(uint32 hoursFromNow) const = 0;
    virtual std::vector<std::pair<std::string, double>> getPerformancePredictions() const = 0;
};

// Factory function
Exs_PerformanceInfoBase* Exs_CreatePerformanceInfoInstance();

} // namespace PerformanceInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_PERFORMANCE_INFO_BASE_H
