// src/Core/Platform/Windows/GraphicsInfoWindows.cpp
#include "../internal/GraphicsInfoBase.h"
#include <windows.h>
#include <dxgi.h>
#include <d3d12.h>
#include <d3d11.h>
#include <d3d9.h>
#include <comdef.h>
#include <wbemidl.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "wbemuuid.lib")

namespace Exs {
namespace Internal {
namespace GraphicsInfo {

class Exs_GraphicsInfoWindows : public Exs_GraphicsInfoBase {
private:
    mutable IDXGIFactory* dxgiFactory = nullptr;
    mutable bool dxgiInitialized = false;
    
public:
    Exs_GraphicsInfoWindows() {
        initializeDXGI();
    }
    
    virtual ~Exs_GraphicsInfoWindows() {
        cleanupDXGI();
    }
    
    std::vector<Exs_GPUAdapterInfo> getGPUAdapters() const override {
        std::vector<Exs_GPUAdapterInfo> adapters;
        
        if (!dxgiInitialized || !dxgiFactory) {
            return adapters;
        }
        
        IDXGIAdapter* adapter = nullptr;
        for (UINT i = 0; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
            Exs_GPUAdapterInfo info = getAdapterInfo(adapter, i);
            adapters.push_back(info);
            adapter->Release();
        }
        
        return adapters;
    }
    
    Exs_GPUAdapterInfo getPrimaryGPU() const override {
        auto adapters = getGPUAdapters();
        if (!adapters.empty()) {
            return adapters[0];
        }
        return Exs_GPUAdapterInfo();
    }
    
    uint32 getGPUCount() const override {
        if (!dxgiInitialized || !dxgiFactory) {
            return 0;
        }
        
        IDXGIAdapter* adapter = nullptr;
        UINT count = 0;
        
        while (dxgiFactory->EnumAdapters(count, &adapter) != DXGI_ERROR_NOT_FOUND) {
            adapter->Release();
            count++;
        }
        
        return count;
    }
    
    std::vector<Exs_DisplayInfo> getDisplays() const override {
        std::vector<Exs_DisplayInfo> displays;
        
        if (!dxgiInitialized || !dxgiFactory) {
            return displays;
        }
        
        IDXGIAdapter* adapter = nullptr;
        for (UINT adapterIndex = 0; 
             dxgiFactory->EnumAdapters(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; 
             adapterIndex++) {
            
            IDXGIOutput* output = nullptr;
            for (UINT outputIndex = 0; 
                 adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND; 
                 outputIndex++) {
                
                Exs_DisplayInfo info = getOutputInfo(output, adapterIndex, outputIndex);
                displays.push_back(info);
                output->Release();
            }
            adapter->Release();
        }
        
        return displays;
    }
    
    Exs_DisplayInfo getPrimaryDisplay() const override {
        auto displays = getDisplays();
        for (const auto& display : displays) {
            if (display.isPrimary) {
                return display;
            }
        }
        
        if (!displays.empty()) {
            return displays[0];
        }
        
        return Exs_DisplayInfo();
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
        auto features = getGPUFeatures();
        
        if (feature == "DirectX12") return supportsAPI(Exs_GraphicsAPI::Direct3D12);
        if (feature == "DirectX11") return supportsAPI(Exs_GraphicsAPI::Direct3D11);
        if (feature == "Vulkan") return supportsAPI(Exs_GraphicsAPI::Vulkan);
        if (feature == "OpenGL") return supportsAPI(Exs_GraphicsAPI::OpenGL);
        if (feature == "RayTracing") return features.supportsRayTracing;
        if (feature == "HDR") return features.supportsHDR;
        
        return false;
    }
    
    bool supportsAPI(Exs_GraphicsAPI api) const override {
        // Check for D3D12 support
        if (api == Exs_GraphicsAPI::Direct3D12) {
            return checkD3D12Support();
        }
        // Check for D3D11 support
        else if (api == Exs_GraphicsAPI::Direct3D11) {
            return checkD3D11Support();
        }
        // Check for D3D10 support
        else if (api == Exs_GraphicsAPI::Direct3D10) {
            return checkD3D10Support();
        }
        // Check for D3D9 support
        else if (api == Exs_GraphicsAPI::Direct3D9) {
            return checkD3D9Support();
        }
        // Check for OpenGL support
        else if (api == Exs_GraphicsAPI::OpenGL) {
            return checkOpenGLSupport();
        }
        // Check for Vulkan support (simplified)
        else if (api == Exs_GraphicsAPI::Vulkan) {
            HMODULE vulkan = LoadLibraryA("vulkan-1.dll");
            if (vulkan) {
                FreeLibrary(vulkan);
                return true;
            }
            return false;
        }
        
        return false;
    }
    
    Exs_GPUPerformanceMetrics getPerformanceMetrics() const override {
        return getPerformanceMetricsForGPU(0);
    }
    
    Exs_GPUPerformanceMetrics getPerformanceMetricsForGPU(uint32 gpuIndex) const override {
        Exs_GPUPerformanceMetrics metrics = {};
        
        // Try to get metrics via WMI
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            return metrics;
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
                                // Get metrics
                                VARIANT vtProp;
                                
                                // Adapter RAM
                                hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
                                if (SUCCEEDED(hr)) {
                                    metrics.dedicatedMemoryUsed = vtProp.ullVal;
                                    VariantClear(&vtProp);
                                }
                                
                                // Current refresh rate
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
        }
        
        CoUninitialize();
        
        // Try to get temperature via other means
        metrics.temperature = getGPUTemperatureForGPU(gpuIndex);
        
        return metrics;
    }
    
    int32 getGPUTemperature() const override {
        return getGPUTemperatureForGPU(0);
    }
    
    int32 getGPUTemperatureForGPU(uint32 gpuIndex) const override {
        // Try to get temperature via WMI or vendor-specific APIs
        // This is vendor-specific and may require additional libraries
        
        // For NVIDIA: nvapi.dll
        // For AMD: atiadlxx.dll
        // For Intel: Not typically exposed
        
        return 0; // Default
    }
    
    uint32 getGPUUtilization() const override {
        // Would require vendor-specific APIs or Windows Performance Counters
        return 0;
    }
    
    uint32 getMemoryUtilization() const override {
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint32 getGPUClockSpeed() const override {
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint32 getMemoryClockSpeed() const override {
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint32 getGPUPowerUsage() const override {
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint32 getGPUPowerLimit() const override {
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint32 getGPUFanSpeed() const override {
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint32 getGPUFanCount() const override {
        // Would require vendor-specific APIs
        return 0;
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
        // Check for HDR support via DXGI
        if (!dxgiInitialized || !dxgiFactory) {
            return false;
        }
        
        IDXGIAdapter* adapter = nullptr;
        if (dxgiFactory->EnumAdapters(0, &adapter) == S_OK) {
            IDXGIOutput* output = nullptr;
            if (adapter->EnumOutputs(0, &output) == S_OK) {
                IDXGIOutput6* output6 = nullptr;
                if (output->QueryInterface(__uuidof(IDXGIOutput6), (void**)&output6) == S_OK) {
                    DXGI_OUTPUT_DESC1 desc;
                    if (output6->GetDesc1(&desc) == S_OK) {
                        output6->Release();
                        output->Release();
                        adapter->Release();
                        return desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
                    }
                    output6->Release();
                }
                output->Release();
            }
            adapter->Release();
        }
        
        return false;
    }
    
    bool isHDREnabled() const override {
        // Check current display mode for HDR
        return false; // Simplified
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
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint64 getVRAMTotal() const override {
        auto gpu = getPrimaryGPU();
        return gpu.dedicatedVideoMemory;
    }
    
    uint64 getVRAMFree() const override {
        uint64 total = getVRAMTotal();
        uint64 used = getVRAMUsage();
        return total > used ? total - used : 0;
    }
    
    std::string getDriverDate() const override {
        auto gpu = getPrimaryGPU();
        return ""; // Driver date not available from DXGI
    }
    
    std::string getDriverProvider() const override {
        auto gpu = getPrimaryGPU();
        switch (gpu.vendor) {
            case Exs_GPUVendor::NVIDIA: return "NVIDIA";
            case Exs_GPUVendor::AMD: return "AMD";
            case Exs_GPUVendor::Intel: return "Intel";
            default: return "Unknown";
        }
    }
    
    bool isDriverUpToDate() const override {
        // Would require checking against online database
        return false;
    }
    
    bool isOverclocked() const override {
        // Would require vendor-specific APIs
        return false;
    }
    
    uint32 getOverclockOffset() const override {
        // Would require vendor-specific APIs
        return 0;
    }
    
    uint32 getMaxTextureSize() const override {
        // Check D3D11 capabilities
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        
        D3D_FEATURE_LEVEL featureLevel;
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        
        if (D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                             featureLevels, 4, D3D11_SDK_VERSION, &device, 
                             &featureLevel, &context) == S_OK) {
            
            D3D11_FEATURE_DATA_D3D11_OPTIONS1 options;
            if (device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, 
                                          &options, sizeof(options)) == S_OK) {
                device->Release();
                if (context) context->Release();
                
                // Max texture dimension depends on feature level
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
            
            device->Release();
            if (context) context->Release();
        }
        
        return 4096; // Default
    }
    
    uint32 getMaxRenderTargets() const override {
        // D3D11 supports up to 8 simultaneous render targets
        return 8;
    }
    
    uint32 getMaxAnisotropy() const override {
        // D3D11 supports up to 16x anisotropy
        return 16;
    }
    
    uint32 getShaderModel() const override {
        // Check highest supported shader model
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        
        D3D_FEATURE_LEVEL featureLevel;
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        
        if (D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                             featureLevels, 4, D3D11_SDK_VERSION, &device, 
                             &featureLevel, &context) == S_OK) {
            
            device->Release();
            if (context) context->Release();
            
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
        
        return 30; // Default
    }
    
    uint32 getMaxComputeThreads() const override {
        // Would require checking hardware capabilities
        return 1024; // Typical default
    }
    
    uint32 getDirectXFeatureLevel() const override {
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        
        D3D_FEATURE_LEVEL featureLevel;
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        
        if (D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                             featureLevels, 4, D3D11_SDK_VERSION, &device, 
                             &featureLevel, &context) == S_OK) {
            
            device->Release();
            if (context) context->Release();
            
            return (uint32)featureLevel;
        }
        
        return 0;
    }
    
    uint32 getVulkanVersion() const override {
        // Simplified check
        HMODULE vulkan = LoadLibraryA("vulkan-1.dll");
        if (vulkan) {
            // Would query actual version via vkEnumerateInstanceVersion
            FreeLibrary(vulkan);
            return 100; // Vulkan 1.0
        }
        return 0;
    }
    
    std::vector<std::string> getVulkanExtensions() const override {
        std::vector<std::string> extensions;
        
        // Would require loading Vulkan and querying extensions
        HMODULE vulkan = LoadLibraryA("vulkan-1.dll");
        if (vulkan) {
            // Add common extensions
            extensions.push_back("VK_KHR_surface");
            extensions.push_back("VK_KHR_win32_surface");
            FreeLibrary(vulkan);
        }
        
        return extensions;
    }
    
    uint32 getOpenGLVersion() const override {
        // Simplified check
        HMODULE opengl = LoadLibraryA("opengl32.dll");
        if (opengl) {
            // Would create context and query version
            FreeLibrary(opengl);
            return 45; // OpenGL 4.5
        }
        return 0;
    }
    
    std::vector<std::string> getOpenGLExtensions() const override {
        std::vector<std::string> extensions;
        
        // Simplified - would require creating OpenGL context
        extensions.push_back("GL_ARB_multitexture");
        extensions.push_back("GL_ARB_texture_compression");
        extensions.push_back("GL_ARB_vertex_buffer_object");
        
        return extensions;
    }
    
private:
    void initializeDXGI() {
        if (dxgiInitialized) return;
        
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
        if (SUCCEEDED(hr)) {
            dxgiInitialized = true;
        }
    }
    
    void cleanupDXGI() {
        if (dxgiFactory) {
            dxgiFactory->Release();
            dxgiFactory = nullptr;
        }
        dxgiInitialized = false;
    }
    
    Exs_GPUAdapterInfo getAdapterInfo(IDXGIAdapter* adapter, UINT index) const {
        Exs_GPUAdapterInfo info;
        
        DXGI_ADAPTER_DESC desc;
        if (adapter->GetDesc(&desc) == S_OK) {
            // Name and description
            info.name = wideToString(desc.Description);
            info.description = wideToString(desc.Description);
            
            // Vendor
            info.vendorId = desc.VendorId;
            info.deviceId = desc.DeviceId;
            info.subSystemId = desc.SubSysId;
            info.revision = desc.Revision;
            
            // Determine vendor
            switch (desc.VendorId) {
                case 0x10DE: // NVIDIA
                    info.vendor = Exs_GPUVendor::NVIDIA;
                    break;
                case 0x1002: // AMD
                case 0x1022: // AMD (alternative)
                    info.vendor = Exs_GPUVendor::AMD;
                    break;
                case 0x8086: // Intel
                    info.vendor = Exs_GPUVendor::Intel;
                    break;
                case 0x1414: // Microsoft (Virtual)
                    info.vendor = Exs_GPUVendor::Microsoft;
                    break;
                default:
                    info.vendor = Exs_GPUVendor::Unknown;
            }
            
            // Memory
            info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
            info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
            info.sharedSystemMemory = desc.SharedSystemMemory;
            
            // Driver version
            LARGE_INTEGER version;
            if (adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &version) == S_OK) {
                std::stringstream ss;
                ss << HIWORD(version.HighPart) << "." 
                   << LOWORD(version.HighPart) << "." 
                   << HIWORD(version.LowPart) << "." 
                   << LOWORD(version.LowPart);
                info.driverVersion = ss.str();
            }
            
            // Features (simplified)
            info.features = getAdapterFeatures(adapter);
            
            // Supported APIs
            if (checkD3D12Support()) {
                info.supportedAPIs.push_back(Exs_GraphicsAPI::Direct3D12);
            }
            if (checkD3D11Support()) {
                info.supportedAPIs.push_back(Exs_GraphicsAPI::Direct3D11);
            }
            if (checkD3D10Support()) {
                info.supportedAPIs.push_back(Exs_GraphicsAPI::Direct3D10);
            }
            if (checkD3D9Support()) {
                info.supportedAPIs.push_back(Exs_GraphicsAPI::Direct3D9);
            }
            if (checkOpenGLSupport()) {
                info.supportedAPIs.push_back(Exs_GraphicsAPI::OpenGL);
            }
            
            // Try to get additional info via WMI
            getAdditionalAdapterInfo(info);
        }
        
        return info;
    }
    
    Exs_DisplayInfo getOutputInfo(IDXGIOutput* output, UINT adapterIndex, UINT outputIndex) const {
        Exs_DisplayInfo info;
        
        DXGI_OUTPUT_DESC desc;
        if (output->GetDesc(&desc) == S_OK) {
            // Basic info
            info.name = wideToString(desc.DeviceName);
            info.isPrimary = desc.AttachedToDesktop;
            info.isActive = desc.AttachedToDesktop;
            
            // Monitor coordinates
            info.x = desc.DesktopCoordinates.left;
            info.y = desc.DesktopCoordinates.top;
            info.width = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
            info.height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
            
            // Rotation
            switch (desc.Rotation) {
                case DXGI_MODE_ROTATION_IDENTITY:
                    info.rotation = 0;
                    break;
                case DXGI_MODE_ROTATION_ROTATE90:
                    info.rotation = 90;
                    break;
                case DXGI_MODE_ROTATION_ROTATE180:
                    info.rotation = 180;
                    break;
                case DXGI_MODE_ROTATION_ROTATE270:
                    info.rotation = 270;
                    break;
                default:
                    info.rotation = 0;
            }
            
            // Get current display mode
            DXGI_MODE_DESC mode;
            if (output->FindClosestMatchingMode(&mode, &mode, nullptr) == S_OK) {
                info.refreshRate = mode.RefreshRate.Numerator / mode.RefreshRate.Denominator;
                info.bitsPerPixel = DXGIFormatBitsPerPixel(mode.Format);
            }
            
            // Try to get EDID
            getEDIDInfo(output, info);
        }
        
        return info;
    }
    
    Exs_GPUFeatures getAdapterFeatures(IDXGIAdapter* adapter) const {
        Exs_GPUFeatures features = {};
        
        // Check D3D12 support for advanced features
        ID3D12Device* d3d12Device = nullptr;
        if (D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, 
                             __uuidof(ID3D12Device), (void**)&d3d12Device) == S_OK) {
            
            D3D12_FEATURE_DATA_D3D12_OPTIONS options;
            if (d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, 
                                                &options, sizeof(options)) == S_OK) {
                features.supportsRayTracing = options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
                features.supportsMeshShaders = options.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
                features.supportsVariableRateShading = options.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
            }
            
            d3d12Device->Release();
        }
        
        // Check D3D11 support
        ID3D11Device* d3d11Device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        
        D3D_FEATURE_LEVEL featureLevel;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        
        if (D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                             featureLevels, 1, D3D11_SDK_VERSION, &d3d11Device, 
                             &featureLevel, &context) == S_OK) {
            
            features.supportsComputeShaders = true;
            features.supportsTessellation = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
            features.supportsGeometryShaders = true;
            
            d3d11Device->Release();
            if (context) context->Release();
        }
        
        return features;
    }
    
    void getAdditionalAdapterInfo(Exs_GPUAdapterInfo& info) const {
        // Try to get additional info via WMI
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) return;
        
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
                                          bstr_t("SELECT * FROM Win32_VideoController"),
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
                            
                            // Get adapter RAM
                            hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                info.dedicatedVideoMemory = vtProp.ullVal;
                                VariantClear(&vtProp);
                            }
                            
                            // Get current refresh rate
                            hr = pclsObj->Get(L"CurrentRefreshRate", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr)) {
                                // info.currentRefreshRate = vtProp.uintVal;
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
    }
    
    void getEDIDInfo(IDXGIOutput* output, Exs_DisplayInfo& info) const {
        // Try to get EDID via registry
        HKEY hKey;
        std::wstring keyPath = L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\";
        keyPath += info.name.substr(0, info.name.find(L"\\"));
        keyPath += L"\\Device Parameters";
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            BYTE edidData[256];
            DWORD dataSize = sizeof(edidData);
            
            if (RegQueryValueExW(hKey, L"EDID", nullptr, nullptr, edidData, &dataSize) == ERROR_SUCCESS) {
                info.edidData.assign(edidData, edidData + dataSize);
                
                // Parse basic EDID info
                if (dataSize >= 8) {
                    // Manufacturer ID (bytes 8-9)
                    uint16_t manufacturerId = (edidData[8] << 8) | edidData[9];
                    char manufacturer[4] = {
                        char(((manufacturerId >> 10) & 0x1F) + 'A' - 1),
                        char(((manufacturerId >> 5) & 0x1F) + 'A' - 1),
                        char((manufacturerId & 0x1F) + 'A' - 1),
                        '\0'
                    };
                    info.manufacturer = manufacturer;
                    
                    // Product code (bytes 10-11)
                    uint16_t productCode = (edidData[11] << 8) | edidData[10];
                    std::stringstream ss;
                    ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << productCode;
                    info.model = ss.str();
                    
                    // Serial number (bytes 12-15)
                    uint32_t serial = (edidData[15] << 24) | (edidData[14] << 16) | 
                                     (edidData[13] << 8) | edidData[12];
                    info.serialNumber = std::to_string(serial);
                }
            }
            
            RegCloseKey(hKey);
        }
    }
    
    bool checkD3D12Support() const {
        ID3D12Device* device = nullptr;
        return D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, 
                                __uuidof(ID3D12Device), (void**)&device) == S_OK;
    }
    
    bool checkD3D11Support() const {
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        D3D_FEATURE_LEVEL featureLevel;
        
        return D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                nullptr, 0, D3D11_SDK_VERSION, &device, 
                                &featureLevel, &context) == S_OK;
    }
    
    bool checkD3D10Support() const {
        ID3D10Device* device = nullptr;
        HMODULE d3d10 = LoadLibraryA("d3d10.dll");
        if (d3d10) {
            // Simplified check
            FreeLibrary(d3d10);
            return true;
        }
        return false;
    }
    
    bool checkD3D9Support() const {
        HMODULE d3d9 = LoadLibraryA("d3d9.dll");
        if (d3d9) {
            FreeLibrary(d3d9);
            return true;
        }
        return false;
    }
    
    bool checkOpenGLSupport() const {
        HMODULE opengl = LoadLibraryA("opengl32.dll");
        if (opengl) {
            FreeLibrary(opengl);
            return true;
        }
        return false;
    }
    
    std::wstring stringToWide(const std::string& str) const {
        if (str.empty()) return L"";
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
        return wstr;
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
    
    static UINT DXGIFormatBitsPerPixel(DXGI_FORMAT format) {
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
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
                return 64;
                
            case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            case DXGI_FORMAT_R10G10B10A2_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UINT:
            case DXGI_FORMAT_R11G11B10_FLOAT:
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_UINT:
            case DXGI_FORMAT_R8G8B8A8_SNORM:
            case DXGI_FORMAT_R8G8B8A8_SINT:
            case DXGI_FORMAT_R16G16_TYPELESS:
            case DXGI_FORMAT_R16G16_FLOAT:
            case DXGI_FORMAT_R16G16_UNORM:
            case DXGI_FORMAT_R16G16_UINT:
            case DXGI_FORMAT_R16G16_SNORM:
            case DXGI_FORMAT_R16G16_SINT:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_R32_FLOAT:
            case DXGI_FORMAT_R32_UINT:
            case DXGI_FORMAT_R32_SINT:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            case DXGI_FORMAT_R8G8_B8G8_UNORM:
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8X8_UNORM:
            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                return 32;
                
            case DXGI_FORMAT_R8G8_TYPELESS:
            case DXGI_FORMAT_R8G8_UNORM:
            case DXGI_FORMAT_R8G8_UINT:
            case DXGI_FORMAT_R8G8_SNORM:
            case DXGI_FORMAT_R8G8_SINT:
            case DXGI_FORMAT_R16_TYPELESS:
            case DXGI_FORMAT_R16_FLOAT:
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_R16_UNORM:
            case DXGI_FORMAT_R16_UINT:
            case DXGI_FORMAT_R16_SNORM:
            case DXGI_FORMAT_R16_SINT:
            case DXGI_FORMAT_B5G6R5_UNORM:
            case DXGI_FORMAT_B5G5R5A1_UNORM:
            case DXGI_FORMAT_A8P8:
            case DXGI_FORMAT_B4G4R4A4_UNORM:
                return 16;
                
            case DXGI_FORMAT_R8_TYPELESS:
            case DXGI_FORMAT_R8_UNORM:
            case DXGI_FORMAT_R8_UINT:
            case DXGI_FORMAT_R8_SNORM:
            case DXGI_FORMAT_R8_SINT:
            case DXGI_FORMAT_A8_UNORM:
            case DXGI_FORMAT_P8:
                return 8;
                
            case DXGI_FORMAT_R1_UNORM:
                return 1;
                
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
