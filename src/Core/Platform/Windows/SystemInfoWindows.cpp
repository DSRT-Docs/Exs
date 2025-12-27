// src/Core/Platform/Windows/SystemInfoWindows.cpp
#include "../internal/SystemInfoBase.h"
#include <windows.h>
#include <lmcons.h>
#include <wtsapi32.h>
#include <powrprof.h>
#include <sddl.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wininet.lib")

namespace Exs {
namespace Internal {
namespace SystemInfo {

class Exs_SystemInfoWindows : public Exs_SystemInfoBase {
private:
    mutable std::string computerNameCache;
    mutable std::string userNameCache;
    mutable std::string domainNameCache;
    
public:
    Exs_SystemInfoWindows() = default;
    virtual ~Exs_SystemInfoWindows() = default;
    
    std::string getComputerName() const override {
        if (!computerNameCache.empty()) {
            return computerNameCache;
        }
        
        wchar_t name[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        
        if (GetComputerNameW(name, &size)) {
            computerNameCache = std::string(name, name + size);
        }
        
        return computerNameCache;
    }
    
    std::string getUserName() const override {
        if (!userNameCache.empty()) {
            return userNameCache;
        }
        
        wchar_t name[UNLEN + 1];
        DWORD size = UNLEN + 1;
        
        if (GetUserNameW(name, &size)) {
            userNameCache = std::string(name, name + size - 1); // Exclude null terminator
        }
        
        return userNameCache;
    }
    
    std::string getDomainName() const override {
        if (!domainNameCache.empty()) {
            return domainNameCache;
        }
        
        wchar_t name[DNLEN + 1];
        DWORD size = DNLEN + 1;
        
        if (GetComputerObjectNameW(NameSamCompatible, name, &size)) {
            domainNameCache = std::string(name, name + size);
        }
        
        return domainNameCache;
    }
    
    std::string getOSName() const override {
        OSVERSIONINFOEXW osvi = { sizeof(osvi) };
        NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW) = nullptr;
        
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (ntdll) {
            RtlGetVersion = (NTSTATUS(WINAPI*)(LPOSVERSIONINFOEXW))GetProcAddress(ntdll, "RtlGetVersion");
        }
        
        if (RtlGetVersion) {
            RtlGetVersion(&osvi);
        } else {
            GetVersionExW((LPOSVERSIONINFOW)&osvi);
        }
        
        std::wstringstream wss;
        
        if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0) {
            if (osvi.wProductType == VER_NT_WORKSTATION) {
                wss << L"Windows 10";
                if (osvi.dwBuildNumber >= 22000) {
                    wss << L" / Windows 11";
                }
            } else {
                wss << L"Windows Server 2016/2019/2022";
            }
        } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3) {
            if (osvi.wProductType == VER_NT_WORKSTATION) {
                wss << L"Windows 8.1";
            } else {
                wss << L"Windows Server 2012 R2";
            }
        } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2) {
            if (osvi.wProductType == VER_NT_WORKSTATION) {
                wss << L"Windows 8";
            } else {
                wss << L"Windows Server 2012";
            }
        } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
            if (osvi.wProductType == VER_NT_WORKSTATION) {
                wss << L"Windows 7";
            } else {
                wss << L"Windows Server 2008 R2";
            }
        } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
            if (osvi.wProductType == VER_NT_WORKSTATION) {
                wss << L"Windows Vista";
            } else {
                wss << L"Windows Server 2008";
            }
        } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
            wss << L"Windows Server 2003 / Windows XP x64";
        } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
            wss << L"Windows XP";
        } else {
            wss << L"Windows (Unknown Version)";
        }
        
        return std::string(wss.str().begin(), wss.str().end());
    }
    
    std::string getOSVersion() const override {
        OSVERSIONINFOEXW osvi = { sizeof(osvi) };
        NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW) = nullptr;
        
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (ntdll) {
            RtlGetVersion = (NTSTATUS(WINAPI*)(LPOSVERSIONINFOEXW))GetProcAddress(ntdll, "RtlGetVersion");
        }
        
        if (RtlGetVersion) {
            RtlGetVersion(&osvi);
        } else {
            GetVersionExW((LPOSVERSIONINFOW)&osvi);
        }
        
        std::wstringstream wss;
        wss << osvi.dwMajorVersion << L"." << osvi.dwMinorVersion << L"." << osvi.dwBuildNumber;
        
        if (osvi.szCSDVersion[0]) {
            wss << L" " << osvi.szCSDVersion;
        }
        
        return std::string(wss.str().begin(), wss.str().end());
    }
    
    std::string getOSBuild() const override {
        HKEY hKey;
        DWORD buildNumber = 0;
        DWORD dataSize = sizeof(buildNumber);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"CurrentBuildNumber", nullptr, nullptr, 
                           (LPBYTE)&buildNumber, &dataSize);
            RegCloseKey(hKey);
        }
        
        return std::to_string(buildNumber);
    }
    
    std::string getOSEdition() const override {
        HKEY hKey;
        wchar_t productName[256] = {0};
        DWORD dataSize = sizeof(productName);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"ProductName", nullptr, nullptr, 
                           (LPBYTE)productName, &dataSize);
            RegCloseKey(hKey);
        }
        
        return std::string(productName, productName + wcslen(productName));
    }
    
    std::string getOSInstallDate() const override {
        HKEY hKey;
        wchar_t installDateStr[64] = {0};
        DWORD dataSize = sizeof(installDateStr);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"InstallDate", nullptr, nullptr, 
                           (LPBYTE)installDateStr, &dataSize);
            RegCloseKey(hKey);
        }
        
        DWORD installDate = _wtoi(installDateStr);
        if (installDate == 0) {
            return "Unknown";
        }
        
        std::time_t time = installDate;
        std::tm* tm = std::localtime(&time);
        
        std::stringstream ss;
        ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    Exs_SystemBootInfo getBootInfo() const override {
        Exs_SystemBootInfo info;
        
        // Get boot time using GetTickCount64
        ULONGLONG uptime = GetTickCount64() / 1000; // Convert to seconds
        info.uptimeSeconds = uptime;
        
        // Calculate boot time
        auto now = std::chrono::system_clock::now();
        auto bootTime = now - std::chrono::seconds(uptime);
        info.bootTime = std::chrono::system_clock::to_time_t(bootTime);
        
        // Get boot mode
        HKEY hKey;
        DWORD bootType = 0;
        DWORD dataSize = sizeof(bootType);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SYSTEM\\CurrentControlSet\\Control", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, L"SystemStartOptions", nullptr, nullptr, 
                               nullptr, nullptr) == ERROR_SUCCESS) {
                info.bootMode = "Advanced";
            } else {
                info.bootMode = "Normal";
            }
            RegCloseKey(hKey);
        }
        
        // Get boot count from registry
        DWORD bootCount = 0;
        dataSize = sizeof(bootCount);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"ClearPageFileAtShutdown", nullptr, nullptr, 
                           (LPBYTE)&bootCount, &dataSize);
            RegCloseKey(hKey);
            info.bootCount = bootCount;
        }
        
        return info;
    }
    
    Exs_SystemLocaleInfo getLocaleInfo() const override {
        Exs_SystemLocaleInfo info;
        
        // Get language
        wchar_t langName[LOCALE_NAME_MAX_LENGTH] = {0};
        if (GetUserDefaultLocaleName(langName, LOCALE_NAME_MAX_LENGTH) > 0) {
            info.language = std::string(langName, langName + wcslen(langName));
        }
        
        // Get country
        wchar_t country[256] = {0};
        if (GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SENGCOUNTRY, 
                          country, sizeof(country)/sizeof(country[0])) > 0) {
            info.country = std::string(country, country + wcslen(country));
        }
        
        // Get timezone
        TIME_ZONE_INFORMATION tzi;
        if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID) {
            std::wstringstream wss;
            wss << L"UTC";
            if (tzi.Bias < 0) {
                wss << L"+";
                tzi.Bias = -tzi.Bias;
            } else {
                wss << L"-";
            }
            wss << tzi.Bias / 60 << L":" << std::setw(2) << std::setfill(L'0') 
                << tzi.Bias % 60;
            info.timezone = std::string(wss.str().begin(), wss.str().end());
        }
        
        // Get encoding
        UINT codePage = GetACP();
        info.encoding = "CP" + std::to_string(codePage);
        
        // Get keyboard layout
        HKL keyboardLayout = GetKeyboardLayout(0);
        wchar_t klName[KL_NAMELENGTH] = {0};
        if (GetKeyboardLayoutNameW(klName)) {
            info.keyboardLayout = std::string(klName, klName + wcslen(klName));
        }
        
        return info;
    }
    
    Exs_SystemSecurityInfo getSecurityInfo() const override {
        Exs_SystemSecurityInfo info;
        
        // Check firewall status
        INetFwProfile* fwProfile = nullptr;
        HRESULT hr = CoCreateInstance(__uuidof(NetFwMgr), nullptr, CLSCTX_INPROC_SERVER, 
                                     __uuidof(INetFwMgr), (void**)&fwProfile);
        
        if (SUCCEEDED(hr) && fwProfile) {
            VARIANT_BOOL fwEnabled;
            if (SUCCEEDED(fwProfile->get_FirewallEnabled(&fwEnabled))) {
                info.isFirewallEnabled = fwEnabled == VARIANT_TRUE;
            }
            fwProfile->Release();
        }
        
        // Check antivirus (Windows Defender)
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (SUCCEEDED(hres)) {
            IWSCProductList* pProductList = nullptr;
            hres = CoCreateInstance(__uuidof(WSCProductList), nullptr, 
                                   CLSCTX_INPROC_SERVER, __uuidof(IWSCProductList), 
                                   (LPVOID*)&pProductList);
            
            if (SUCCEEDED(hres) && pProductList) {
                hres = pProductList->Initialize(WSC_SECURITY_PROVIDER_ANTIVIRUS);
                if (SUCCEEDED(hres)) {
                    LONG productCount;
                    pProductList->get_Count(&productCount);
                    info.isAntivirusInstalled = productCount > 0;
                }
                pProductList->Release();
            }
            CoUninitialize();
        }
        
        // Check Secure Boot
        HKEY hKey;
        DWORD secureBootEnabled = 0;
        DWORD dataSize = sizeof(secureBootEnabled);
        
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                         L"SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"UEFISecureBootEnabled", nullptr, nullptr, 
                           (LPBYTE)&secureBootEnabled, &dataSize);
            RegCloseKey(hKey);
            info.isSecureBootEnabled = secureBootEnabled != 0;
        }
        
        // Check TPM
        TBS_CONTEXT_PARAMS2 params = {TBS_CONTEXT_VERSION_TWO};
        TBS_HCONTEXT hContext;
        if (Tbsi_Context_Create(&params, &hContext) == TBS_SUCCESS) {
            info.isTPMAvailable = true;
            Tbsip_Context_Close(hContext);
        }
        
        // Security level (simple heuristic)
        info.securityLevel = 0;
        if (info.isFirewallEnabled) info.securityLevel++;
        if (info.isAntivirusInstalled) info.securityLevel++;
        if (info.isSecureBootEnabled) info.securityLevel++;
        if (info.isTPMAvailable) info.securityLevel++;
        
        return info;
    }
    
    uint32 getSystemDPI() const override {
        HDC hdc = GetDC(nullptr);
        int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(nullptr, hdc);
        return dpiX;
    }
    
    bool isHighContrastMode() const override {
        HIGHCONTRAST hc = { sizeof(HIGHCONTRAST) };
        if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, 0)) {
            return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
        }
        return false;
    }
    
    bool isTabletMode() const override {
        // Windows 10/11 Tablet Mode detection
        HKEY hKey;
        DWORD tabletMode = 0;
        DWORD dataSize = sizeof(tabletMode);
        
        if (RegOpenKeyExW(HKEY_CURRENT_USER, 
                         L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell", 
                         0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, L"TabletMode", nullptr, nullptr, 
                               (LPBYTE)&tabletMode, &dataSize) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return tabletMode == 1;
            }
            RegCloseKey(hKey);
        }
        return false;
    }
    
    bool isOnBatteryPower() const override {
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps)) {
            return sps.ACLineStatus == 0; // 0 = Offline (battery)
        }
        return false;
    }
    
    uint32 getBatteryPercentage() const override {
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps)) {
            return sps.BatteryLifePercent;
        }
        return 0;
    }
    
    bool isNetworkAvailable() const override {
        DWORD flags;
        return InternetGetConnectedState(&flags, 0) != 0;
    }
    
    bool isInternetAvailable() const override {
        return InternetCheckConnectionW(L"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0) != 0;
    }
    
    uint64 getSystemUptime() const override {
        return GetTickCount64() / 1000; // Convert to seconds
    }
    
    std::vector<std::string> getRecentSystemEvents(uint32 count) const override {
        std::vector<std::string> events;
        
        HANDLE hEventLog = OpenEventLogW(nullptr, L"System");
        if (hEventLog) {
            DWORD bytesRead = 0;
            DWORD bytesNeeded = 0;
            BYTE buffer[16384];
            EVENTLOGRECORD* record = (EVENTLOGRECORD*)buffer;
            
            while (ReadEventLogW(hEventLog, EVENTLOG_BACKWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                               0, record, sizeof(buffer), &bytesRead, &bytesNeeded)) {
                if (events.size() >= count) break;
                
                std::wstring wmessage;
                wchar_t* source = (wchar_t*)((BYTE*)record + sizeof(EVENTLOGRECORD));
                wchar_t* computer = source + wcslen(source) + 1;
                
                // Format message
                wchar_t* strings = (wchar_t*)((BYTE*)record + record->StringOffset);
                LPWSTR messageBuffer = nullptr;
                DWORD messageSize = FormatMessageW(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr, record->EventID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPWSTR)&messageBuffer, 0, nullptr);
                
                if (messageBuffer) {
                    wmessage = messageBuffer;
                    LocalFree(messageBuffer);
                } else {
                    wmessage = strings;
                }
                
                std::string eventStr = "[" + std::string(source, source + wcslen(source)) + "] " +
                                      std::string(wmessage.begin(), wmessage.end());
                events.push_back(eventStr);
            }
            
            CloseEventLog(hEventLog);
        }
        
        return events;
    }
    
    bool performSystemHealthCheck() const override {
        // Check critical system components
        bool health = true;
        
        // 1. Check disk space
        ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
        if (GetDiskFreeSpaceExW(L"C:", &freeBytes, &totalBytes, &totalFreeBytes)) {
            double freePercent = (double)freeBytes.QuadPart / totalBytes.QuadPart * 100.0;
            if (freePercent < 10.0) health = false; // Less than 10% free
        }
        
        // 2. Check memory
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            double memoryUsage = (double)memStatus.dwMemoryLoad;
            if (memoryUsage > 90.0) health = false; // Over 90% memory usage
        }
        
        // 3. Check system uptime (if too long, might need restart)
        uint64 uptime = getSystemUptime();
        if (uptime > 30 * 24 * 3600) { // 30 days
            health = false;
        }
        
        // 4. Check for critical system errors in event log
        HANDLE hEventLog = OpenEventLogW(nullptr, L"System");
        if (hEventLog) {
            DWORD bytesRead = 0;
            DWORD bytesNeeded = 0;
            BYTE buffer[8192];
            EVENTLOGRECORD* record = (EVENTLOGRECORD*)buffer;
            
            if (ReadEventLogW(hEventLog, EVENTLOG_BACKWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                           0, record, sizeof(buffer), &bytesRead, &bytesNeeded)) {
                // Check last 10 records for critical errors
                for (int i = 0; i < 10 && i < (int)(bytesRead / sizeof(EVENTLOGRECORD)); i++) {
                    if (record->EventType == EVENTLOG_ERROR_TYPE) {
                        health = false;
                        break;
                    }
                    record = (EVENTLOGRECORD*)((BYTE*)record + record->Length);
                }
            }
            CloseEventLog(hEventLog);
        }
        
        return health;
    }
};

// Factory function implementation
Exs_SystemInfoBase* Exs_CreateSystemInfoInstance() {
    return new Exs_SystemInfoWindows();
}

} // namespace SystemInfo
} // namespace Internal
} // namespace Exs
