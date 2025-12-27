// src/Core/Platform/Windows/PlatformWindows.cpp
#include "../internal/PlatformBase.h"
#include <windows.h>
#include <versionhelpers.h>
#include <intrin.h>
#include <string>
#include <sstream>
#include <vector>

namespace Exs {
namespace Internal {
namespace Platform {

class Exs_PlatformWindows : public Exs_PlatformBase {
private:
    mutable std::string platformNameCache;
    mutable std::string platformVersionCache;
    mutable Exs_PlatformType platformTypeCache = Exs_PlatformType::Unknown;
    
public:
    Exs_PlatformWindows() {
        // Initialize platform detection
        detectPlatform();
    }
    
    virtual ~Exs_PlatformWindows() = default;
    
    Exs_PlatformType getPlatformType() const override {
        if (platformTypeCache == Exs_PlatformType::Unknown) {
            detectPlatform();
        }
        return platformTypeCache;
    }
    
    Exs_Architecture getArchitecture() const override {
        SYSTEM_INFO sysInfo;
        GetNativeSystemInfo(&sysInfo);
        
        switch (sysInfo.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                return Exs_Architecture::x64;
            case PROCESSOR_ARCHITECTURE_INTEL:
                return Exs_Architecture::x86;
            case PROCESSOR_ARCHITECTURE_ARM:
                return Exs_Architecture::ARM;
            case PROCESSOR_ARCHITECTURE_ARM64:
                return Exs_Architecture::ARM64;
            default:
                return Exs_Architecture::Unknown;
        }
    }
    
    Exs_Endianness getEndianness() const override {
        // Windows on x86/x64 is always little-endian
        return Exs_Endianness::Little;
    }
    
    std::string getPlatformName() const override {
        if (!platformNameCache.empty()) {
            return platformNameCache;
        }
        
        std::string name = "Windows";
        
        if (IsWindowsServer()) {
            name += " Server";
        } else if (IsWindowsVersionOrGreater(10, 0, 0)) {
            name += " 10/11";
        } else if (IsWindows8Point1OrGreater()) {
            name += " 8.1";
        } else if (IsWindows8OrGreater()) {
            name += " 8";
        } else if (IsWindows7OrGreater()) {
            name += " 7";
        } else if (IsWindowsVistaOrGreater()) {
            name += " Vista";
        } else if (IsWindowsXPOrGreater()) {
            name += " XP";
        }
        
        platformNameCache = name;
        return platformNameCache;
    }
    
    std::string getPlatformVersion() const override {
        if (!platformVersionCache.empty()) {
            return platformVersionCache;
        }
        
        OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
        DWORDLONG const dwlConditionMask = VerSetConditionMask(
            VerSetConditionMask(
                VerSetConditionMask(
                    0, VER_MAJORVERSION, VER_GREATER_EQUAL),
                VER_MINORVERSION, VER_GREATER_EQUAL),
            VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
        
        std::wstringstream versionStream;
        
        if (IsWindows10OrGreater()) {
            versionStream << L"10.0";
        } else if (IsWindows8Point1OrGreater()) {
            versionStream << L"6.3";
        } else if (IsWindows8OrGreater()) {
            versionStream << L"6.2";
        } else if (IsWindows7SP1OrGreater()) {
            versionStream << L"6.1 SP1";
        } else if (IsWindows7OrGreater()) {
            versionStream << L"6.1";
        } else if (IsWindowsVistaSP2OrGreater()) {
            versionStream << L"6.0 SP2";
        } else if (IsWindowsVistaSP1OrGreater()) {
            versionStream << L"6.0 SP1";
        } else if (IsWindowsVistaOrGreater()) {
            versionStream << L"6.0";
        } else if (IsWindowsXPSP3OrGreater()) {
            versionStream << L"5.1 SP3";
        } else if (IsWindowsXPSP2OrGreater()) {
            versionStream << L"5.1 SP2";
        } else if (IsWindowsXPSP1OrGreater()) {
            versionStream << L"5.1 SP1";
        } else if (IsWindowsXPOrGreater()) {
            versionStream << L"5.1";
        } else {
            versionStream << L"Unknown";
        }
        
        platformVersionCache = std::string(versionStream.str().begin(), versionStream.str().end());
        return platformVersionCache;
    }
    
    std::string getPlatformVendor() const override {
        return "Microsoft Corporation";
    }
    
    bool isMobilePlatform() const override {
        return IsWindowsIoT();
    }
    
    bool isDesktopPlatform() const override {
        return !IsWindowsServer() && !IsWindowsIoT();
    }
    
    bool isConsolePlatform() const override {
        return false; // Windows is not a console platform
    }
    
    bool supportsSIMD() const override {
        int cpuInfo[4] = {0};
        __cpuid(cpuInfo, 1);
        return (cpuInfo[3] & (1 << 25)) != 0; // Check SSE bit
    }
    
    bool supportsAVX() const override {
        int cpuInfo[4] = {0};
        __cpuidex(cpuInfo, 1, 0);
        
        // Check OSXSAVE and AVX bits
        bool osxsave = (cpuInfo[2] & (1 << 27)) != 0;
        bool avx = (cpuInfo[2] & (1 << 28)) != 0;
        
        if (!osxsave || !avx) return false;
        
        // Check if OS supports AVX
        uint32_t xcr0;
        #if defined(_MSC_VER)
        xcr0 = (uint32_t)_xgetbv(0);
        #else
        __asm__ ("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx");
        #endif
        
        return (xcr0 & 0x6) == 0x6;
    }
    
    bool supportsNEON() const override {
        // NEON is ARM-specific, not supported on Windows x86/x64
        return false;
    }
    
    std::string getHomeDirectory() const override {
        wchar_t path[MAX_PATH];
        if (SHGetFolderPathW(nullptr, CSIDL_PROFILE, nullptr, 0, path) == S_OK) {
            return std::string(path, path + wcslen(path));
        }
        return "";
    }
    
    std::string getTempDirectory() const override {
        wchar_t path[MAX_PATH];
        if (GetTempPathW(MAX_PATH, path) > 0) {
            return std::string(path, path + wcslen(path));
        }
        return "";
    }
    
    std::string getAppDataDirectory() const override {
        wchar_t path[MAX_PATH];
        if (SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path) == S_OK) {
            return std::string(path, path + wcslen(path));
        }
        return "";
    }
    
    std::string getExecutableDirectory() const override {
        wchar_t path[MAX_PATH];
        if (GetModuleFileNameW(nullptr, path, MAX_PATH) > 0) {
            std::wstring wpath(path);
            size_t pos = wpath.find_last_of(L"\\/");
            if (pos != std::wstring::npos) {
                wpath = wpath.substr(0, pos);
            }
            return std::string(wpath.begin(), wpath.end());
        }
        return "";
    }
    
    void executeCommand(const std::string& command) const override {
        system(command.c_str());
    }
    
    int32 executeCommandWithResult(const std::string& command) const override {
        return system(command.c_str());
    }
    
    uint32 getCurrentThreadId() const override {
        return GetCurrentThreadId();
    }
    
    uint32 getCurrentProcessId() const override {
        return GetCurrentProcessId();
    }
    
    uint64 getHighResolutionTimer() const override {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter.QuadPart;
    }
    
    double getHighResolutionTimerFrequency() const override {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return static_cast<double>(frequency.QuadPart);
    }
    
    uint32 getMemoryPageSize() const override {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwPageSize;
    }
    
    uint32 getPhysicalCoreCount() const override {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* buffer = nullptr;
        DWORD bufferSize = 0;
        
        GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return 0;
        }
        
        buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(bufferSize);
        if (!buffer) {
            return 0;
        }
        
        uint32 coreCount = 0;
        if (GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &bufferSize)) {
            BYTE* ptr = (BYTE*)buffer;
            DWORD offset = 0;
            
            while (offset < bufferSize) {
                SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info = 
                    (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(ptr + offset);
                
                if (info->Relationship == RelationProcessorCore) {
                    coreCount++;
                }
                
                offset += info->Size;
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
    
private:
    void detectPlatform() const {
        if (IsWindowsServer()) {
            platformTypeCache = Exs_PlatformType::Windows;
        } else if (IsWindows10OrGreater()) {
            platformTypeCache = Exs_PlatformType::Windows;
        } else if (IsWindows8Point1OrGreater()) {
            platformTypeCache = Exs_PlatformType::Windows;
        } else if (IsWindows8OrGreater()) {
            platformTypeCache = Exs_PlatformType::Windows;
        } else if (IsWindows7OrGreater()) {
            platformTypeCache = Exs_PlatformType::Windows;
        } else {
            platformTypeCache = Exs_PlatformType::Unknown;
        }
    }
};

// Factory function implementation
Exs_PlatformBase* Exs_CreatePlatformInstance() {
    return new Exs_PlatformWindows();
}

} // namespace Platform
} // namespace Internal
} // namespace Exs
