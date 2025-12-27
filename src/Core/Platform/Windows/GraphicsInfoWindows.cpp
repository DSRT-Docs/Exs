// src/Core/Platform/Windows/GraphicsInfoWindows.cpp
#include "../internal/GraphicsInfoBase.h"
#include <windows.h>
#include <dxgi.h>
#include <d3d12.h>
#include <d3d11.h>
#include <d3d9.h>
#include <comdef.h>
#include <wbemidl.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <mutex>
#include <memory>
#include <unordered_map>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "wbemuuid.lib")

namespace Exs {
namespace Internal {
namespace GraphicsInfo {

// RAII COM wrapper dengan thread safety
class ThreadSafeCOMInitializer {
public:
    ThreadSafeCOMInitializer() {
        std::lock_guard<std::mutex> lock(comMutex);
        if (comRefCount++ == 0) {
            comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            comInitialized = SUCCEEDED(comResult);
        }
    }
    
    ~ThreadSafeCOMInitializer() {
        std::lock_guard<std::mutex> lock(comMutex);
        if (--comRefCount == 0 && comInitialized) {
            CoUninitialize();
            comInitialized = false;
        }
    }
    
    bool isInitialized() const { return comInitialized; }
    HRESULT getResult() const { return comResult; }
    
    static uint32 getRefCount() {
        std::lock_guard<std::mutex> lock(comMutex);
        return comRefCount;
    }
    
private:
    static std::mutex comMutex;
    static uint32 comRefCount;
    static bool comInitialized;
    static HRESULT comResult;
};

std::mutex ThreadSafeCOMInitializer::comMutex;
uint32 ThreadSafeCOMInitializer::comRefCount = 0;
bool ThreadSafeCOMInitializer::comInitialized = false;
HRESULT ThreadSafeCOMInitializer::comResult = S_OK;

// Safe library loader dengan RAII
class SafeLibrary {
public:
    SafeLibrary(const char* libraryName) : handle(nullptr) {
        handle = LoadLibraryA(libraryName);
    }
    
    ~SafeLibrary() {
        if (handle) {
            FreeLibrary(handle);
            handle = nullptr;
        }
    }
    
    // Non-copyable
    SafeLibrary(const SafeLibrary&) = delete;
    SafeLibrary& operator=(const SafeLibrary&) = delete;
    
    // Moveable
    SafeLibrary(SafeLibrary&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    
    SafeLibrary& operator=(SafeLibrary&& other) noexcept {
        if (this != &other) {
            if (handle) FreeLibrary(handle);
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
    
    explicit operator bool() const { return handle != nullptr; }
    HMODULE getHandle() const { return handle; }
    
    template<typename T>
    T getProcAddress(const char* procName) const {
        if (!handle) return nullptr;
        return reinterpret_cast<T>(::GetProcAddress(handle, procName));
    }
    
private:
    HMODULE handle;
};

// Thread-safe cache untuk graphics data
class GraphicsCache {
private:
    struct CacheData {
        std::vector<Exs_GPUAdapterInfo> adapters;
        std::vector<Exs_DisplayInfo> displays;
        std::chrono::steady_clock::time_point lastUpdate;
        std::chrono::milliseconds cacheDuration;
        
        CacheData() : cacheDuration(std::chrono::seconds(5)) {}
        
        bool isExpired() const {
            auto now = std::chrono::steady_clock::now();
            return (now - lastUpdate) > cacheDuration;
        }
        
        void updateAdapters(std::vector<Exs_GPUAdapterInfo>&& newAdapters) {
            adapters = std::move(newAdapters);
            lastUpdate = std::chrono::steady_clock::now();
        }
        
        void updateDisplays(std::vector<Exs_DisplayInfo>&& newDisplays) {
            displays = std::move(newDisplays);
            lastUpdate = std::chrono::steady_clock::now();
        }
    };
    
public:
    static GraphicsCache& getInstance() {
        static GraphicsCache instance;
        return instance;
    }
    
    const std::vector<Exs_GPUAdapterInfo>& getAdapters(
        std::function<std::vector<Exs_GPUAdapterInfo>()> updateFunc) {
        
        std::lock_guard<std::mutex> lock(cacheMutex);
        
        if (cacheData.adapters.empty() || cacheData.isExpired()) {
            auto newAdapters = updateFunc();
            cacheData.updateAdapters(std::move(newAdapters));
        }
        
        return cacheData.adapters;
    }
    
    const std::vector<Exs_DisplayInfo>& getDisplays(
        std::function<std::vector<Exs_DisplayInfo>()> updateFunc) {
        
        std::lock_guard<std::mutex> lock(cacheMutex);
        
        if (cacheData.displays.empty() || cacheData.isExpired()) {
            auto newDisplays = updateFunc();
            cacheData.updateDisplays(std::move(newDisplays));
        }
        
        return cacheData.displays;
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(cacheMutex);
        cacheData.adapters.clear();
        cacheData.displays.clear();
    }
    
    void setCacheDuration(std::chrono::milliseconds duration) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        cacheData.cacheDuration = duration;
    }
    
private:
    GraphicsCache() = default;
    ~GraphicsCache() = default;
    
    // Non-copyable, non-movable
    GraphicsCache(const GraphicsCache&) = delete;
    GraphicsCache& operator=(const GraphicsCache&) = delete;
    GraphicsCache(GraphicsCache&&) = delete;
    GraphicsCache& operator=(GraphicsCache&&) = delete;
    
    CacheData cacheData;
    std::mutex cacheMutex;
};

class Exs_GraphicsInfoWindows : public Exs_GraphicsInfoBase {
private:
    mutable std::mutex performanceMetricsMutex;
    mutable std::unordered_map<uint32, Exs_GPUPerformanceMetrics> metricsCache;
    
public:
    Exs_GraphicsInfoWindows() = default;
    virtual ~Exs_GraphicsInfoWindows() = default;
    
    std::vector<Exs_GPUAdapterInfo> getGPUAdapters() const override {
        return GraphicsCache::getInstance().getAdapters([this]() {
            return this->updateGPUAdapters();
        });
    }
    
    Exs_GPUAdapterInfo getPrimaryGPU() const override {
        auto adapters = getGPUAdapters();
        return adapters.empty() ? Exs_GPUAdapterInfo() : adapters[0];
    }
    
    uint32 getGPUCount() const override {
        auto adapters = getGPUAdapters();
        return static_cast<uint32>(adapters.size());
    }
    
    std::vector<Exs_DisplayInfo> getDisplays() const override {
        return GraphicsCache::getInstance().getDisplays([this]() {
            return this->updateDisplays();
        });
    }
    
    Exs_DisplayInfo getPrimaryDisplay() const override {
        auto displays = getDisplays();
        for (const auto& display : displays) {
            if (display.isPrimary) {
                return display;
            }
        }
        return displays.empty() ? Exs_DisplayInfo() : displays[0];
    }
    
    uint32 getDisplayCount() const override {
        auto displays = getDisplays();
        return static_cast<uint32>(displays.size());
    }
    
    std::string getGPUName() const override {
        auto gpu = getPrimaryGPU();
        return gpu.name;
    }
    
    Exs_GPUVendor getGPUVendor() const override {
        auto gpu = getPrimaryGPU();
        return gpu.vendor;
    }
    
    std::string getDriverVersion() const override {
        auto gpu = getPrimaryGPU();
        return gpu.driverVersion;
    }
    
    uint64 getVideoMemory() const override {
        auto gpu = getPrimaryGPU();
        return gpu.dedicatedVideoMemory;
    }
    
    uint64 getSharedMemory() const override {
        auto gpu = getPrimaryGPU();
        return gpu.sharedSystemMemory;
    }
    
    Exs_GPUFeatures getGPUFeatures() const override {
        auto gpu = getPrimaryGPU();
        return gpu.features;
    }
    
    bool supportsFeature(const std::string& feature) const override {
        if (feature == "DirectX12") return supportsAPI(Exs_GraphicsAPI::Direct3D12);
        if (feature == "DirectX11") return supportsAPI(Exs_GraphicsAPI::Direct3D11);
        if (feature == "DirectX10") return supportsAPI(Exs_GraphicsAPI::Direct3D10);
        if (feature == "DirectX9") return supportsAPI(Exs_GraphicsAPI::Direct3D9);
        if (feature == "Vulkan") return supportsAPI(Exs_GraphicsAPI::Vulkan);
        if (feature == "OpenGL") return supportsAPI(Exs_GraphicsAPI::OpenGL);
        if (feature == "OpenGLES") return supportsAPI(Exs_GraphicsAPI::OpenGLES);
        
        auto features = getGPUFeatures();
        if (feature == "RayTracing") return features.supportsRayTracing;
        if (feature == "HDR") return features.supportsHDR;
        if (feature == "DLSS") return features.supportsDLSS;
        if (feature == "FSR") return features.supportsFSR;
        if (feature == "TensorCores") return features.supportsTensorCores;
        
        return false;
    }
    
    bool supportsAPI(Exs_GraphicsAPI api) const override {
        switch (api) {
            case Exs_GraphicsAPI::Direct3D12:
                return checkD3D12Support();
            case Exs_GraphicsAPI::Direct3D11:
                return checkD3D11Support();
            case Exs_GraphicsAPI::Direct3D10:
                return checkD3D10Support();
            case Exs_GraphicsAPI::Direct3D9:
                return checkD3D9Support();
            case Exs_GraphicsAPI::Vulkan:
                return checkVulkanSupport();
            case Exs_GraphicsAPI::OpenGL:
                return checkOpenGLSupport();
            case Exs_GraphicsAPI::OpenGLES:
                return checkOpenGLESSupport();
            default:
                return false;
        }
    }
    
    Exs_GPUPerformanceMetrics getPerformanceMetrics() const override {
        return getPerformanceMetricsForGPU(0);
    }
    
    Exs_GPUPerformanceMetrics getPerformanceMetricsForGPU(uint32 gpuIndex) const override {
        std::lock_guard<std::mutex> lock(performanceMetricsMutex);
        
        // Check cache first
        auto it = metricsCache.find(gpuIndex);
        if (it != metricsCache.end()) {
            // Check if cache is still valid (1 second)
            auto now = std::chrono::steady_clock::now();
            auto cacheTime = std::chrono::duration_cast<std::chrono::seconds>(
                now.time_since_epoch()).count();
            
            if (it->second.timestamp + 1 > cacheTime) {
                return it->second;
            }
        }
        
        // Update cache
        Exs_GPUPerformanceMetrics metrics = {};
        metrics.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        
        // Get basic metrics via WMI
        ThreadSafeCOMInitializer com;
        if (com.isInitialized()) {
            metrics = getWMIPerformanceMetrics(gpuIndex);
        }
        
        // Try vendor-specific APIs
        if (metrics.gpuUsage == 0) {
            metrics = getVendorPerformanceMetrics(gpuIndex);
        }
        
        // Cache the result
        metricsCache[gpuIndex] = metrics;
        return metrics;
    }
    
    int32 getGPUTemperature() const override {
        return getGPUTemperatureForGPU(0);
    }
    
    int32 getGPUTemperatureForGPU(uint32 gpuIndex) const override {
        // Try NVIDIA first
        int32 temp = getNVIDIATemperature(gpuIndex);
        if (temp != 0) return temp;
        
        // Try AMD
        temp = getAMDTemperature(gpuIndex);
        if (temp != 0) return temp;
        
        // Try Intel
        temp = getIntelTemperature(gpuIndex);
        
        return temp;
    }
    
    uint32 getGPUUtilization() const override {
        auto metrics = getPerformanceMetrics();
        return metrics.gpuUsage;
    }
    
    uint32 getMemoryUtilization() const override {
        auto metrics = getPerformanceMetrics();
        return metrics.memoryUsage;
    }
    
    uint32 getGPUClockSpeed() const override {
        auto metrics = getPerformanceMetrics();
        return metrics.clockSpeed;
    }
    
    uint32 getMemoryClockSpeed() const override {
        auto metrics = getPerformanceMetrics();
        return metrics.memoryClockSpeed;
    }
    
    uint32 getGPUPowerUsage() const override {
        auto metrics = getPerformanceMetrics();
        return metrics.powerUsage;
    }
    
    uint32 getGPUPowerLimit() const override {
        auto metrics = getPerformanceMetrics();
        return metrics.powerLimitMaximum;
    }
    
    uint32 getGPUFanSpeed() const override {
        auto metrics = getPerformanceMetrics();
        return metrics.fanSpeed;
    }
    
    uint32 getGPUFanCount() const override {
        // This typically requires vendor-specific APIs
        return 0; // Placeholder
    }
    
    uint32 getCurrentResolutionWidth() const override {
        DEVMODEW dm = {0};
        dm.dmSize = sizeof(dm);
        
        if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
            return dm.dmPelsWidth;
        }
        return 0;
    }
    
    uint32 getCurrentResolutionHeight() const override {
        DEVMODEW dm = {0};
        dm.dmSize = sizeof(dm);
        
        if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
            return dm.dmPelsHeight;
        }
        return 0;
    }
    
    uint32 getCurrentRefreshRate() const override {
        DEVMODEW dm = {0};
        dm.dmSize = sizeof(dm);
        
        if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
            return dm.dmDisplayFrequency;
        }
        return 0;
    }
    
    uint32 getMaxRefreshRate() const override {
        DEVMODEW dm = {0};
        dm.dmSize = sizeof(dm);
        uint32 maxRefreshRate = 0;
        
        for (int i = 0; EnumDisplaySettingsW(nullptr, i, &dm); i++) {
            if (dm.dmDisplayFrequency > maxRefreshRate) {
                maxRefreshRate = dm.dmDisplayFrequency;
            }
        }
        
        return maxRefreshRate;
    }
    
    bool isHDRSupported() const override {
        // Use DXGI 1.6 for HDR detection
        Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory6)))) {
            return false;
        }
        
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        if (FAILED(factory6->EnumAdapters(0, &adapter))) {
            return false;
        }
        
        Microsoft::WRL::ComPtr<IDXGIOutput> output;
        if (FAILED(adapter->EnumOutputs(0, &output))) {
            return false;
        }
        
        Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
        if (FAILED(output.As(&output6))) {
            return false;
        }
        
        DXGI_OUTPUT_DESC1 desc1;
        if (FAILED(output6->GetDesc1(&desc1))) {
            return false;
        }
        
        return desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
    }
    
    bool isHDREnabled() const override {
        // Check registry for HDR enablement
        HKEY hKey;
        DWORD hdrEnabled = 0;
        DWORD dataSize = sizeof(hdrEnabled);
        
        if (RegOpenKeyExW(HKEY_CURRENT_USER, 
                         L"Software\\Microsoft\\Windows\\CurrentVersion\\HDR", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"UserHDREnabled", nullptr, nullptr, 
                           (LPBYTE)&hdrEnabled, &dataSize);
            RegCloseKey(hKey);
        }
        
        return hdrEnabled != 0;
    }
    
    bool isMultiGPU() const override {
        return getGPUCount() > 1;
    }
    
    bool isSLIEnabled() const override {
        // Check NVIDIA SLI via registry
        HKEY hKey;
        DWORD sliEnabled = 0;
        DWORD dataSize = sizeof(sliEnabled);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SOFTWARE\\NVIDIA Corporation\\Global\\NVSMI", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, L"EnableSli", nullptr, nullptr, 
                               (LPBYTE)&sliEnabled, &dataSize) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return sliEnabled != 0;
            }
            RegCloseKey(hKey);
        }
        
        return false;
    }
    
    bool isCrossFireEnabled() const override {
        // Check AMD CrossFire via registry
        HKEY hKey;
        DWORD crossfireEnabled = 0;
        DWORD dataSize = sizeof(crossfireEnabled);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SOFTWARE\\AMD\\CrossFire", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, L"Enabled", nullptr, nullptr, 
                               (LPBYTE)&crossfireEnabled, &dataSize) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return crossfireEnabled != 0;
            }
            RegCloseKey(hKey);
        }
        
        return false;
    }
    
    uint64 getVRAMUsage() const override {
        // Try NVIDIA first
        uint64 usage = getNVIDIAVRAMUsage(0);
        if (usage > 0) return usage;
        
        // Try AMD
        usage = getAMDVRAMUsage(0);
        if (usage > 0) return usage;
        
        // Fallback to DXGI shared memory
        auto gpu = getPrimaryGPU();
        return gpu.dedicatedVideoMemory - (gpu.dedicatedVideoMemory / 4); // Estimate 75% usage
    }
    
    uint64 getVRAMTotal() const override {
        auto gpu = getPrimaryGPU();
        return gpu.dedicatedVideoMemory;
    }
    
    uint64 getVRAMFree() const override {
        uint64 total = getVRAMTotal();
        uint64 used = getVRAMUsage();
        return (total > used) ? (total - used) : 0;
    }
    
    std::string getDriverDate() const override {
        // Get driver date from registry
        HKEY hKey;
        wchar_t driverDate[256] = {0};
        DWORD dataSize = sizeof(driverDate);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\PnpResources\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Video", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            // This is a simplified path, actual location varies
            RegCloseKey(hKey);
        }
        
        return driverDate[0] ? wideToString(driverDate) : "Unknown";
    }
    
    std::string getDriverProvider() const override {
        auto vendor = getGPUVendor();
        switch (vendor) {
            case Exs_GPUVendor::NVIDIA: return "NVIDIA Corporation";
            case Exs_GPUVendor::AMD: return "Advanced Micro Devices, Inc.";
            case Exs_GPUVendor::Intel: return "Intel Corporation";
            case Exs_GPUVendor::Microsoft: return "Microsoft Corporation";
            default: return "Unknown";
        }
    }
    
    bool isDriverUpToDate() const override {
        // This would require comparing against online database
        // Simplified implementation
        return true;
    }
    
    bool isOverclocked() const override {
        // Check registry for overclocking profiles
        return false;
    }
    
    uint32 getOverclockOffset() const override {
        return 0;
    }
    
    uint32 getMaxTextureSize() const override {
        // Query D3D11 device for capabilities
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL featureLevel;
        
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        
        if (SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                       featureLevels, 4, D3D11_SDK_VERSION, &device, 
                                       &featureLevel, &context))) {
            switch (featureLevel) {
                case D3D_FEATURE_LEVEL_11_1:
                case D3D_FEATURE_LEVEL_11_0:
                    return 16384; // 16K
                case D3D_FEATURE_LEVEL_10_1:
                case D3D_FEATURE_LEVEL_10_0:
                    return 8192; // 8K
                default:
                    return 4096; // 4K
            }
        }
        
        return 4096;
    }
    
    uint32 getMaxRenderTargets() const override {
        // D3D11 supports 8 simultaneous render targets
        return 8;
    }
    
    uint32 getMaxAnisotropy() const override {
        // D3D11 supports 16x anisotropy
        return 16;
    }
    
    uint32 getShaderModel() const override {
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL featureLevel;
        
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        
        if (SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                       featureLevels, 4, D3D11_SDK_VERSION, &device, 
                                       &featureLevel, &context))) {
            switch (featureLevel) {
                case D3D_FEATURE_LEVEL_11_1:
                case D3D_FEATURE_LEVEL_11_0:
                    return 50; // Shader Model 5.0
                case D3D_FEATURE_LEVEL_10_1:
                    return 41; // Shader Model 4.1
                case D3D_FEATURE_LEVEL_10_0:
                    return 40; // Shader Model 4.0
                default:
                    return 30; // Shader Model 3.0
            }
        }
        
        return 30;
    }
    
    uint32 getMaxComputeThreads() const override {
        // Typical value for modern GPUs
        return 1024;
    }
    
    uint32 getDirectXFeatureLevel() const override {
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL featureLevel;
        
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        
        if (SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                       featureLevels, 4, D3D11_SDK_VERSION, &device, 
                                       &featureLevel, &context))) {
            return static_cast<uint32>(featureLevel);
        }
        
        return 0;
    }
    
    uint32 getVulkanVersion() const override {
        SafeLibrary vulkan("vulkan-1.dll");
        if (!vulkan) return 0;
        
        // Try to get vkEnumerateInstanceVersion
        auto vkEnumerateInstanceVersion = vulkan.getProcAddress<PFN_vkEnumerateInstanceVersion>(
            "vkEnumerateInstanceVersion");
        
        if (vkEnumerateInstanceVersion) {
            uint32_t version = 0;
            if (vkEnumerateInstanceVersion(&version) == VK_SUCCESS) {
                return VK_VERSION_MAJOR(version) * 100 + VK_VERSION_MINOR(version) * 10;
            }
        }
        
        return 100; // Vulkan 1.0 minimum
    }
    
    std::vector<std::string> getVulkanExtensions() const override {
        std::vector<std::string> extensions;
        
        SafeLibrary vulkan("vulkan-1.dll");
        if (!vulkan) return extensions;
        
        // Simplified list of common extensions
        extensions.push_back("VK_KHR_surface");
        extensions.push_back("VK_KHR_win32_surface");
        extensions.push_back("VK_KHR_swapchain");
        extensions.push_back("VK_KHR_maintenance1");
        
        return extensions;
    }
    
    uint32 getOpenGLVersion() const override {
        // This would require creating an OpenGL context
        // Simplified check
        SafeLibrary opengl("opengl32.dll");
        return opengl ? 45 : 0; // Assume 4.5 if OpenGL is available
    }
    
    std::vector<std::string> getOpenGLExtensions() const override {
        std::vector<std::string> extensions;
        
        // Common OpenGL extensions
        extensions.push_back("GL_ARB_multitexture");
        extensions.push_back("GL_ARB_texture_compression");
        extensions.push_back("GL_ARB_vertex_buffer_object");
        extensions.push_back("GL_ARB_shader_objects");
        extensions.push_back("GL_ARB_vertex_shader");
        extensions.push_back("GL_ARB_fragment_shader");
        
        return extensions;
    }
    
private:
    std::vector<Exs_GPUAdapterInfo> updateGPUAdapters() const {
        std::vector<Exs_GPUAdapterInfo> adapters;
        
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;
        if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
            if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
                return adapters;
            }
        }
        
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
            try {
                DXGI_ADAPTER_DESC desc;
                if (SUCCEEDED(adapter->GetDesc(&desc))) {
                    Exs_GPUAdapterInfo info;
                    
                    // Basic info
                    info.name = wideToString(desc.Description);
                    info.description = wideToString(desc.Description);
                    info.vendorId = desc.VendorId;
                    info.deviceId = desc.DeviceId;
                    info.subSystemId = desc.SubSysId;
                    info.revision = desc.Revision;
                    
                    // Vendor
                    switch (desc.VendorId) {
                        case 0x10DE: info.vendor = Exs_GPUVendor::NVIDIA; break;
                        case 0x1002: case 0x1022: info.vendor = Exs_GPUVendor::AMD; break;
                        case 0x8086: info.vendor = Exs_GPUVendor::Intel; break;
                        case 0x1414: info.vendor = Exs_GPUVendor::Microsoft; break;
                        default: info.vendor = Exs_GPUVendor::Unknown;
                    }
                    
                    // Memory
                    info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
                    info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
                    info.sharedSystemMemory = desc.SharedSystemMemory;
                    
                    // Driver version
                    LARGE_INTEGER version;
                    if (SUCCEEDED(adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &version))) {
                        std::stringstream ss;
                        ss << HIWORD(version.HighPart) << "." 
                           << LOWORD(version.HighPart) << "." 
                           << HIWORD(version.LowPart) << "." 
                           << LOWORD(version.LowPart);
                        info.driverVersion = ss.str();
                    }
                    
                    // Features
                    info.features = getAdapterFeatures(adapter.Get());
                    
                    // Cache info
                    std::vector<Exs_CPUCacheInfo> cacheInfo = getCacheInfo(adapter.Get());
                    for (const auto& cache : cacheInfo) {
                        if (cache.level == 1) info.l1CacheSize = cache.sizeKB * 1024;
                        else if (cache.level == 2) info.l2CacheSize = cache.sizeKB * 1024;
                        else if (cache.level == 3) info.l3CacheSize = cache.sizeKB * 1024;
                    }
                    
                    adapters.push_back(info);
                }
            }
            catch (...) {
                // Continue with next adapter
            }
            
            adapter.Reset();
        }
        
        return adapters;
    }
    
    std::vector<Exs_DisplayInfo> updateDisplays() const {
        std::vector<Exs_DisplayInfo> displays;
        
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;
        if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
            return displays;
        }
        
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        for (UINT adapterIndex = 0; 
             factory->EnumAdapters(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; 
             adapterIndex++) {
            
            Microsoft::WRL::ComPtr<IDXGIOutput> output;
            for (UINT outputIndex = 0; 
                 adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND; 
                 outputIndex++) {
                
                try {
                    DXGI_OUTPUT_DESC desc;
                    if (SUCCEEDED(output->GetDesc(&desc))) {
                        Exs_DisplayInfo info;
                        
                        info.name = wideToString(desc.DeviceName);
                        info.isPrimary = desc.AttachedToDesktop;
                        info.isActive = desc.AttachedToDesktop;
                        
                        info.x = desc.DesktopCoordinates.left;
                        info.y = desc.DesktopCoordinates.top;
                        info.width = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
                        info.height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
                        
                        switch (desc.Rotation) {
                            case DXGI_MODE_ROTATION_IDENTITY: info.rotation = 0; break;
                            case DXGI_MODE_ROTATION_ROTATE90: info.rotation = 90; break;
                            case DXGI_MODE_ROTATION_ROTATE180: info.rotation = 180; break;
                            case DXGI_MODE_ROTATION_ROTATE270: info.rotation = 270; break;
                            default: info.rotation = 0;
                        }
                        
                        // Get current display mode
                        DXGI_MODE_DESC mode;
                        if (SUCCEEDED(output->FindClosestMatchingMode(&mode, &mode, nullptr))) {
                            info.refreshRate = mode.RefreshRate.Numerator / mode.RefreshRate.Denominator;
                            info.bitsPerPixel = getDXGIFormatBitsPerPixel(mode.Format);
                        }
                        
                        displays.push_back(info);
                    }
                }
                catch (...) {
                    // Continue with next output
                }
                
                output.Reset();
            }
            
            adapter.Reset();
        }
        
        return displays;
    }
    
    Exs_GPUFeatures getAdapterFeatures(IDXGIAdapter* adapter) const {
        Exs_GPUFeatures features = {};
        
        // Check D3D12 features
        Microsoft::WRL::ComPtr<ID3D12Device> d3d12Device;
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, 
                                       IID_PPV_ARGS(&d3d12Device)))) {
            D3D12_FEATURE_DATA_D3D12_OPTIONS options;
            if (SUCCEEDED(d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, 
                                                         &options, sizeof(options)))) {
                features.supportsRayTracing = options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
                features.supportsMeshShaders = options.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
                features.supportsVariableRateShading = options.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
            }
            
            D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_6 };
            if (SUCCEEDED(d3d12Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, 
                                                         &shaderModel, sizeof(shaderModel)))) {
                features.supportsShaderModel6 = shaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_0;
            }
        }
        
        // Check D3D11 features
        Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL featureLevel;
        
        if (SUCCEEDED(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                                       nullptr, 0, D3D11_SDK_VERSION, &d3d11Device, 
                                       &featureLevel, &context))) {
            features.supportsComputeShaders = true;
            features.supportsTessellation = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
            features.supportsGeometryShaders = true;
            features.supportsDirectX12Ultimate = (featureLevel >= D3D_FEATURE_LEVEL_12_1);
        }
        
        return features;
    }
    
    std::vector<Exs_CPUCacheInfo> getCacheInfo(IDXGIAdapter* adapter) const {
        std::vector<Exs_CPUCacheInfo> cacheInfo;
        
        // This would require querying the adapter for cache information
        // Simplified implementation
        Exs_CPUCacheInfo l1Cache;
        l1Cache.level = 1;
        l1Cache.sizeKB = 64; // Typical L1 cache size
        l1Cache.type = "Data";
        cacheInfo.push_back(l1Cache);
        
        Exs_CPUCacheInfo l2Cache;
        l2Cache.level = 2;
        l2Cache.sizeKB = 512; // Typical L2 cache size
        l2Cache.type = "Unified";
        cacheInfo.push_back(l2Cache);
        
        Exs_CPUCacheInfo l3Cache;
        l3Cache.level = 3;
        l3Cache.sizeKB = 4096; // Typical L3 cache size
        l3Cache.type = "Unified";
        cacheInfo.push_back(l3Cache);
        
        return cacheInfo;
    }
    
    bool checkD3D12Support() const {
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        return SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, 
                                          IID_PPV_ARGS(&device)));
    }
    
    bool checkD3D11Support() const {
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL featureLevel;
        
        return SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                          nullptr, 0, D3D11_SDK_VERSION, &device, 
                                          &featureLevel, &context));
    }
    
    bool checkD3D10Support() const {
        SafeLibrary d3d10("d3d10.dll");
        return static_cast<bool>(d3d10);
    }
    
    bool checkD3D9Support() const {
        SafeLibrary d3d9("d3d9.dll");
        return static_cast<bool>(d3d9);
    }
    
    bool checkVulkanSupport() const {
        SafeLibrary vulkan("vulkan-1.dll");
        return static_cast<bool>(vulkan);
    }
    
    bool checkOpenGLSupport() const {
        SafeLibrary opengl("opengl32.dll");
        return static_cast<bool>(opengl);
    }
    
    bool checkOpenGLESSupport() const {
        SafeLibrary opengles("libGLESv2.dll");
        return static_cast<bool>(opengles);
    }
    
    Exs_GPUPerformanceMetrics getWMIPerformanceMetrics(uint32 gpuIndex) const {
        Exs_GPUPerformanceMetrics metrics = {};
        
        IWbemLocator* pLoc = nullptr;
        HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
                                       IID_IWbemLocator, (LPVOID*)&pLoc);
        if (FAILED(hres) || !pLoc) return metrics;
        
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
                                      bstr_t("SELECT * FROM Win32_VideoController"),
                                      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                                      nullptr, &pEnumerator);
                
                if (SUCCEEDED(hres)) {
                    IWbemClassObject* pclsObj = nullptr;
                    ULONG uReturn = 0;
                    uint32 currentIndex = 0;
                    
                    while (pEnumerator) {
                        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
                                                     &pclsObj, &uReturn);
                        if (0 == uReturn) break;
                        
                        if (currentIndex == gpuIndex) {
                            VARIANT vtProp;
                            
                            // Get adapter RAM
                            hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                metrics.dedicatedMemoryUsed = vtProp.ullVal;
                                VariantClear(&vtProp);
                            }
                            
                            // Get current refresh rate
                            hr = pclsObj->Get(L"CurrentRefreshRate", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                metrics.fps = vtProp.uintVal;
                                VariantClear(&vtProp);
                            }
                            
                            pclsObj->Release();
                            break;
                        }
                        
                        currentIndex++;
                        pclsObj->Release();
                    }
                    
                    pEnumerator->Release();
                }
            }
            pSvc->Release();
        }
        
        pLoc->Release();
        return metrics;
    }
    
    Exs_GPUPerformanceMetrics getVendorPerformanceMetrics(uint32 gpuIndex) const {
        Exs_GPUPerformanceMetrics metrics = {};
        
        // Try NVIDIA
        metrics = getNVIDIAMetrics(gpuIndex);
        if (metrics.gpuUsage > 0) return metrics;
        
        // Try AMD
        metrics = getAMDMetrics(gpuIndex);
        if (metrics.gpuUsage > 0) return metrics;
        
        // Try Intel
        metrics = getIntelMetrics(gpuIndex);
        
        return metrics;
    }
    
    Exs_GPUPerformanceMetrics getNVIDIAMetrics(uint32 gpuIndex) const {
        Exs_GPUPerformanceMetrics metrics = {};
        
        SafeLibrary nvapi("nvapi64.dll");
        if (!nvapi) {
            nvapi = SafeLibrary("nvapi.dll");
            if (!nvapi) return metrics;
        }
        
        // NVIDIA API functions would be called here
        // This is simplified - actual implementation requires NVIDIA SDK
        
        return metrics;
    }
    
    Exs_GPUPerformanceMetrics getAMDMetrics(uint32 gpuIndex) const {
        Exs_GPUPerformanceMetrics metrics = {};
        
        SafeLibrary amd("atiadlxx.dll");
        if (!amd) {
            amd = SafeLibrary("atiadlxy.dll");
            if (!amd) return metrics;
        }
        
        // AMD ADL functions would be called here
        // This is simplified - actual implementation requires AMD ADL SDK
        
        return metrics;
    }
    
    Exs_GPUPerformanceMetrics getIntelMetrics(uint32 gpuIndex) const {
        Exs_GPUPerformanceMetrics metrics = {};
        
        // Intel graphics metrics typically through DXGI or WMI
        return metrics;
    }
    
    int32 getNVIDIATemperature(uint32 gpuIndex) const {
        SafeLibrary nvapi("nvapi64.dll");
        if (!nvapi) {
            nvapi = SafeLibrary("nvapi.dll");
            if (!nvapi) return 0;
        }
        
        // NVIDIA temperature query would go here
        return 0;
    }
    
    int32 getAMDTemperature(uint32 gpuIndex) const {
        SafeLibrary amd("atiadlxx.dll");
        if (!amd) {
            amd = SafeLibrary("atiadlxy.dll");
            if (!amd) return 0;
        }
        
        // AMD temperature query would go here
        return 0;
    }
    
    int32 getIntelTemperature(uint32 gpuIndex) const {
        // Intel GPU temperature is usually not exposed
        return 0;
    }
    
    uint64 getNVIDIAVRAMUsage(uint32 gpuIndex) const {
        SafeLibrary nvapi("nvapi64.dll");
        if (!nvapi) {
            nvapi = SafeLibrary("nvapi.dll");
            if (!nvapi) return 0;
        }
        
        // NVIDIA VRAM query would go here
        return 0;
    }
    
    uint64 getAMDVRAMUsage(uint32 gpuIndex) const {
        SafeLibrary amd("atiadlxx.dll");
        if (!amd) {
            amd = SafeLibrary("atiadlxy.dll");
            if (!amd) return 0;
        }
        
        // AMD VRAM query would go here
        return 0;
    }
    
    std::string wideToString(const std::wstring& wstr) const {
        if (wstr.empty()) return "";
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), 
                                             nullptr, 0, nullptr, nullptr);
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), 
                           &str[0], size_needed, nullptr, nullptr);
        return str;
    }
    
    static UINT getDXGIFormatBitsPerPixel(DXGI_FORMAT format) {
        switch (format) {
            case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
            case DXGI_FORMAT_R32G32B32A32_UINT:
            case DXGI_FORMAT_R32G32B32A32_SINT:
                return 128;
                
            case DXGI_FORMAT_R32G32B32_TYPELESS:
            case DXGI_FORMAT_R32G32B32_FLOAT:
            case DXGI_FORMAT_R32G32B32_UINT:
            case DXGI_FORMAT_R32G32B32_SINT:
                return 96;
                
            case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R16G16B16A16_UNORM:
            case DXGI_FORMAT_R16G16B16A16_UINT:
            case DXGI_FORMAT_R16G16B16A16_SNORM:
            case DXGI_FORMAT_R16G16B16A16_SINT:
            case DXGI_FORMAT_R32G32_TYPELESS:
            case DXGI_FORMAT_R32G32_FLOAT:
            case DXGI_FORMAT_R32G32_UINT:
            case DXGI_FORMAT_R32G32_SINT:
                return 64;
                
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_UINT:
            case DXGI_FORMAT_R8G8B8A8_SNORM:
            case DXGI_FORMAT_R8G8B8A8_SINT:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8X8_UNORM:
                return 32;
                
            case DXGI_FORMAT_R8G8_TYPELESS:
            case DXGI_FORMAT_R8G8_UNORM:
            case DXGI_FORMAT_R8G8_UINT:
            case DXGI_FORMAT_R8G8_SNORM:
            case DXGI_FORMAT_R8G8_SINT:
                return 16;
                
            case DXGI_FORMAT_R8_TYPELESS:
            case DXGI_FORMAT_R8_UNORM:
            case DXGI_FORMAT_R8_UINT:
            case DXGI_FORMAT_R8_SNORM:
            case DXGI_FORMAT_R8_SINT:
                return 8;
                
            default:
                return 0;
        }
    }
};

// Factory function implementation
Exs_GraphicsInfoBase* Exs_CreateGraphicsInfoInstance() {
    return new Exs_GraphicsInfoWindows();
}

} // namespace GraphicsInfo
} // namespace Internal
} // namespace Exs
