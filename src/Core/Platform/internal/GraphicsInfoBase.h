// src/Core/Platform/internal/GraphicsInfoBase.h
#ifndef EXS_INTERNAL_GRAPHICS_INFO_BASE_H
#define EXS_INTERNAL_GRAPHICS_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Exs {
namespace Internal {
namespace GraphicsInfo {

// Graphics API types
enum class Exs_GraphicsAPI {
    Unknown = 0,
    Direct3D9 = 1,
    Direct3D10 = 2,
    Direct3D11 = 3,
    Direct3D12 = 4,
    OpenGL = 5,
    OpenGLES = 6,
    Vulkan = 7,
    Metal = 8,
    WebGPU = 9
};

// GPU vendor enumeration
enum class Exs_GPUVendor {
    Unknown = 0,
    NVIDIA = 1,
    AMD = 2,
    Intel = 3,
    Apple = 4,
    Qualcomm = 5,
    ARM = 6,
    Imagination = 7,
    Microsoft = 8,
    VMware = 9,
    VirtualBox = 10
};

// GPU memory type
enum class Exs_GPUMemoryType {
    Unknown = 0,
    GDDR5 = 1,
    GDDR6 = 2,
    GDDR6X = 3,
    HBM = 4,
    HBM2 = 5,
    HBM2E = 6,
    HBM3 = 7,
    LPDDR = 8,
    Shared = 9
};

// GPU feature flags
struct Exs_GPUFeatures {
    // Basic features
    bool supportsComputeShaders : 1;
    bool supportsTessellation : 1;
    bool supportsGeometryShaders : 1;
    bool supportsRayTracing : 1;
    bool supportsMeshShaders : 1;
    
    // Advanced features
    bool supportsVariableRateShading : 1;
    bool supportsSamplerFeedback : 1;
    bool supportsMeshShader : 1;
    bool supportsDirectStorage : 1;
    
    // Memory features
    bool supportsResizableBAR : 1;
    bool supportsHardwareAcceleratedGPU : 1;
    
    // Display features
    bool supportsHDR : 1;
    bool supportsG_Sync : 1;
    bool supportsFreeSync : 1;
    bool supportsAdaptiveSync : 1;
    
    // API support
    bool supportsDirectX12Ultimate : 1;
    bool supportsVulkanRT : 1;
    bool supportsOpenGL46 : 1;
    
    // VR features
    bool supportsVR : 1;
    bool supportsMultiView : 1;
    
    // AI features
    bool supportsTensorCores : 1;
    bool supportsRTX : 1;
    bool supportsDLSS : 1;
    bool supportsFSR : 1;
    
    // Encoding/Decoding
    bool supportsNVENC : 1;
    bool supportsNVDEC : 1;
    bool supportsAV1 : 1;
    bool supportsHEVC : 1;
    
    // Power features
    bool supportsPowerManagement : 1;
    bool supportsDynamicBoost : 1;
};

// GPU adapter information
struct Exs_GPUAdapterInfo {
    std::string name;
    std::string description;
    Exs_GPUVendor vendor;
    uint32 vendorId;
    uint32 deviceId;
    uint32 subSystemId;
    uint32 revision;
    
    // Memory information
    uint64 dedicatedVideoMemory;
    uint64 dedicatedSystemMemory;
    uint64 sharedSystemMemory;
    Exs_GPUMemoryType memoryType;
    uint32 memoryBusWidth;
    uint32 memoryClockSpeed;
    
    // Core information
    uint32 coreCount;
    uint32 smCount; // Streaming Multiprocessors
    uint32 clockSpeed; // Base clock in MHz
    uint32 boostClockSpeed; // Boost clock in MHz
    uint32 shaderModel; // Shader model version
    
    // API support
    std::vector<Exs_GraphicsAPI> supportedAPIs;
    std::string driverVersion;
    std::string driverDate;
    
    // Features
    Exs_GPUFeatures features;
    
    // Display outputs
    uint32 maxDisplayCount;
    uint32 maxResolutionWidth;
    uint32 maxResolutionHeight;
    
    // Power
    uint32 tdp; // Thermal Design Power in Watts
    uint32 powerLimit;
    
    // PCI information
    uint32 pcieVersion;
    uint32 pcieLanes;
    std::string pcieSlot;
    
    // Thermal
    uint32 currentTemperature;
    uint32 maxTemperature;
    
    // Utilization
    uint32 gpuUtilization;
    uint32 memoryUtilization;
    uint32 encoderUtilization;
    uint32 decoderUtilization;
};

// Display information
struct Exs_DisplayInfo {
    std::string name;
    std::string manufacturer;
    std::string model;
    std::string serialNumber;
    
    uint32 width;
    uint32 height;
    uint32 refreshRate;
    uint32 bitsPerPixel;
    
    // Position and orientation
    int32 x;
    int32 y;
    uint32 rotation; // 0, 90, 180, 270
    
    // Color information
    uint32 colorDepth;
    bool isHDR;
    double maxBrightness;
    std::string colorGamut;
    
    // Timing
    uint32 horizontalFrequency;
    uint32 verticalFrequency;
    
    // Connector type
    std::string connectorType; // HDMI, DisplayPort, VGA, etc.
    
    // EDID information
    std::vector<uint8> edidData;
    
    // Primary display flag
    bool isPrimary;
    bool isActive;
};

// GPU performance metrics
struct Exs_GPUPerformanceMetrics {
    uint32 gpuUsage; // Percentage
    uint32 memoryUsage; // Percentage
    uint32 fanSpeed; // RPM
    uint32 temperature; // Celsius
    uint32 powerUsage; // Watts
    uint32 voltage; // mV
    uint32 clockSpeed; // MHz
    uint32 memoryClockSpeed; // MHz
    
    // Frame information
    uint32 fps;
    uint32 frameTime; // milliseconds
    uint32 drawCalls;
    uint32 triangles;
    
    // Memory information
    uint64 dedicatedMemoryUsed;
    uint64 sharedMemoryUsed;
    uint64 systemMemoryUsed;
    
    // Power limits
    uint32 powerLimitCurrent;
    uint32 powerLimitDefault;
    uint32 powerLimitMaximum;
    
    // Thermal limits
    uint32 thermalLimitCurrent;
    uint32 thermalLimitMaximum;
    
    // Utilization breakdown
    uint32 smUtilization; // Streaming Multiprocessor utilization
    uint32 memoryControllerUtilization;
    uint32 videoEncoderUtilization;
    uint32 videoDecoderUtilization;
};

// Base graphics info class
class Exs_GraphicsInfoBase {
public:
    virtual ~Exs_GraphicsInfoBase() = default;
    
    // GPU enumeration
    virtual std::vector<Exs_GPUAdapterInfo> getGPUAdapters() const = 0;
    virtual Exs_GPUAdapterInfo getPrimaryGPU() const = 0;
    virtual uint32 getGPUCount() const = 0;
    
    // Display enumeration
    virtual std::vector<Exs_DisplayInfo> getDisplays() const = 0;
    virtual Exs_DisplayInfo getPrimaryDisplay() const = 0;
    virtual uint32 getDisplayCount() const = 0;
    
    // GPU information
    virtual std::string getGPUName() const = 0;
    virtual Exs_GPUVendor getGPUVendor() const = 0;
    virtual std::string getDriverVersion() const = 0;
    virtual uint64 getVideoMemory() const = 0;
    virtual uint64 getSharedMemory() const = 0;
    
    // Feature detection
    virtual Exs_GPUFeatures getGPUFeatures() const = 0;
    virtual bool supportsFeature(const std::string& feature) const = 0;
    virtual bool supportsAPI(Exs_GraphicsAPI api) const = 0;
    
    // Performance monitoring
    virtual Exs_GPUPerformanceMetrics getPerformanceMetrics() const = 0;
    virtual Exs_GPUPerformanceMetrics getPerformanceMetricsForGPU(uint32 gpuIndex) const = 0;
    
    // Temperature monitoring
    virtual int32 getGPUTemperature() const = 0;
    virtual int32 getGPUTemperatureForGPU(uint32 gpuIndex) const = 0;
    
    // Utilization monitoring
    virtual uint32 getGPUUtilization() const = 0;
    virtual uint32 getMemoryUtilization() const = 0;
    
    // Clock speeds
    virtual uint32 getGPUClockSpeed() const = 0;
    virtual uint32 getMemoryClockSpeed() const = 0;
    
    // Power information
    virtual uint32 getGPUPowerUsage() const = 0;
    virtual uint32 getGPUPowerLimit() const = 0;
    
    // Fan information
    virtual uint32 getGPUFanSpeed() const = 0;
    virtual uint32 getGPUFanCount() const = 0;
    
    // Display information
    virtual uint32 getCurrentResolutionWidth() const = 0;
    virtual uint32 getCurrentResolutionHeight() const = 0;
    virtual uint32 getCurrentRefreshRate() const = 0;
    virtual uint32 getMaxRefreshRate() const = 0;
    
    // Color information
    virtual bool isHDRSupported() const = 0;
    virtual bool isHDREnabled() const = 0;
    
    // Multi-GPU information
    virtual bool isMultiGPU() const = 0;
    virtual bool isSLIEnabled() const = 0;
    virtual bool isCrossFireEnabled() const = 0;
    
    // VRAM information
    virtual uint64 getVRAMUsage() const = 0;
    virtual uint64 getVRAMTotal() const = 0;
    virtual uint64 getVRAMFree() const = 0;
    
    // Driver information
    virtual std::string getDriverDate() const = 0;
    virtual std::string getDriverProvider() const = 0;
    virtual bool isDriverUpToDate() const = 0;
    
    // Overclocking information
    virtual bool isOverclocked() const = 0;
    virtual uint32 getOverclockOffset() const = 0;
    
    // Rendering capabilities
    virtual uint32 getMaxTextureSize() const = 0;
    virtual uint32 getMaxRenderTargets() const = 0;
    virtual uint32 getMaxAnisotropy() const = 0;
    
    // Shader information
    virtual uint32 getShaderModel() const = 0;
    virtual uint32 getMaxComputeThreads() const = 0;
    
    // DirectX feature level
    virtual uint32 getDirectXFeatureLevel() const = 0;
    
    // Vulkan information
    virtual uint32 getVulkanVersion() const = 0;
    virtual std::vector<std::string> getVulkanExtensions() const = 0;
    
    // OpenGL information
    virtual uint32 getOpenGLVersion() const = 0;
    virtual std::vector<std::string> getOpenGLExtensions() const = 0;
};

// Factory function
Exs_GraphicsInfoBase* Exs_CreateGraphicsInfoInstance();

} // namespace GraphicsInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_GRAPHICS_INFO_BASE_H
