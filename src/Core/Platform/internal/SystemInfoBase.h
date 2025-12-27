// src/Core/Platform/internal/SystemInfoBase.h
#ifndef EXS_INTERNAL_SYSTEM_INFO_BASE_H
#define EXS_INTERNAL_SYSTEM_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <chrono>

namespace Exs {
namespace Internal {
namespace SystemInfo {

// System boot information
struct Exs_SystemBootInfo {
    std::chrono::system_clock::time_point bootTime;
    uint64 uptimeSeconds;
    uint32 bootCount;
    std::string bootMode;
};

// System locale information
struct Exs_SystemLocaleInfo {
    std::string language;
    std::string country;
    std::string timezone;
    std::string encoding;
    std::string keyboardLayout;
};

// System security information
struct Exs_SystemSecurityInfo {
    bool isFirewallEnabled;
    bool isAntivirusInstalled;
    bool isSecureBootEnabled;
    bool isTPMAvailable;
    uint32 securityLevel;
};

// Base system info class
class Exs_SystemInfoBase {
public:
    virtual ~Exs_SystemInfoBase() = default;
    
    // System identification
    virtual std::string getComputerName() const = 0;
    virtual std::string getUserName() const = 0;
    virtual std::string getDomainName() const = 0;
    
    // Operating system information
    virtual std::string getOSName() const = 0;
    virtual std::string getOSVersion() const = 0;
    virtual std::string getOSBuild() const = 0;
    virtual std::string getOSEdition() const = 0;
    virtual std::string getOSInstallDate() const = 0;
    
    // System boot information
    virtual Exs_SystemBootInfo getBootInfo() const = 0;
    
    // Locale information
    virtual Exs_SystemLocaleInfo getLocaleInfo() const = 0;
    
    // Security information
    virtual Exs_SystemSecurityInfo getSecurityInfo() const = 0;
    
    // System metrics
    virtual uint32 getSystemDPI() const = 0;
    virtual bool isHighContrastMode() const = 0;
    virtual bool isTabletMode() const = 0;
    
    // Power status
    virtual bool isOnBatteryPower() const = 0;
    virtual uint32 getBatteryPercentage() const = 0;
    
    // Network availability
    virtual bool isNetworkAvailable() const = 0;
    virtual bool isInternetAvailable() const = 0;
    
    // System up time
    virtual uint64 getSystemUptime() const = 0;
    
    // Last system events
    virtual std::vector<std::string> getRecentSystemEvents(uint32 count) const = 0;
    
    // System health check
    virtual bool performSystemHealthCheck() const = 0;
};

// Factory function
Exs_SystemInfoBase* Exs_CreateSystemInfoInstance();

} // namespace SystemInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_SYSTEM_INFO_BASE_H
