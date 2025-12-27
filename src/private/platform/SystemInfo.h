#pragma once
#ifndef EXS_SYSTEMINFO_H
#define EXS_SYSTEMINFO_H

#include "PlatformDetection.h"
#include "../Types/BasicTypes.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Exs {
namespace Internal {
namespace Platform {

// System information structure
struct EXS_API Exs_SystemInfo {
    // Operating System Information
    struct Exs_OSInfo {
        std::string name;               // "Windows", "Linux", "macOS"
        std::string version;            // "10.0.19044"
        std::string buildNumber;        // "19044"
        std::string edition;            // "Professional", "Home"
        std::string architecture;       // "x64", "x86", "ARM64"
        bool isServer;                  // True if server edition
        bool isEmbedded;                // True if embedded edition
        bool isVirtualized;             // True if running in VM
        uint32_t majorVersion;
        uint32_t minorVersion;
        uint32_t buildVersion;
        uint32_t platformId;
        uint32_t suiteMask;
        std::string servicePack;
        uint32_t servicePackMajor;
        uint32_t servicePackMinor;
    };
    
    // Computer Information
    struct Exs_ComputerInfo {
        std::string computerName;       // Network computer name
        std::string dnsHostName;        // DNS host name
        std::string domainName;         // Domain name
        std::string userName;           // Current user name
        std::string userDomain;         // User domain
        std::string machineGuid;        // Machine unique ID
        std::string productId;          // Windows Product ID
        std::string registeredOwner;    // Registered owner
        std::string registeredOrganization; // Registered organization
        std::string systemManufacturer; // System manufacturer
        std::string systemProductName;  // System product name
        std::string systemSKU;          // System SKU
        std::string biosVersion;        // BIOS version
        std::string biosDate;           // BIOS date
        std::string biosVendor;         // BIOS vendor
        std::string baseboardManufacturer; // Baseboard manufacturer
        std::string baseboardProduct;   // Baseboard product
        std::string chassisManufacturer; // Chassis manufacturer
        std::string chassisType;        // Chassis type
    };
    
    // Display Information
    struct Exs_DisplayInfo {
        uint32_t displayCount;          // Number of displays
        uint32_t primaryDisplayIndex;   // Primary display index
        
        struct Exs_Display {
            uint32_t index;             // Display index (0-based)
            bool isPrimary;             // Is primary display
            std::string name;           // Display name
            std::string deviceName;     // Device name
            uint32_t width;             // Width in pixels
            uint32_t height;            // Height in pixels
            uint32_t refreshRate;       // Refresh rate in Hz
            uint32_t bitsPerPixel;      // Color depth
            uint32_t dpiX;              // Horizontal DPI
            uint32_t dpiY;              // Vertical DPI
            float scalingFactor;        // Display scaling factor
            uint32_t orientation;       // Display orientation
            bool isActive;              // Is display active
            bool isAttached;            // Is display attached
            uint32_t manufacturerId;    // Manufacturer ID
            uint32_t productId;         // Product ID
            uint32_t serialNumber;      // Serial number
            uint32_t yearOfManufacture; // Year of manufacture
            std::string edid;           // EDID data
        };
        
        std::vector<Exs_Display> displays;
    };
    
    // Network Information
    struct Exs_NetworkInfo {
        uint32_t adapterCount;          // Number of network adapters
        
        struct Exs_NetworkAdapter {
            std::string name;           // Adapter name
            std::string description;    // Adapter description
            std::string macAddress;     // MAC address
            std::string ipAddress;      // IP address
            std::string subnetMask;     // Subnet mask
            std::string gateway;        // Default gateway
            std::string dnsServers;     // DNS servers
            std::string dhcpServer;     // DHCP server
            bool isEnabled;             // Is adapter enabled
            bool isConnected;           // Is adapter connected
            bool isWireless;            // Is wireless adapter
            uint32_t speed;             // Speed in Mbps
            uint32_t mtu;               // Maximum transmission unit
            std::string type;           // Adapter type
            std::string manufacturer;   // Adapter manufacturer
            std::string driverVersion;  // Driver version
            std::string driverDate;     // Driver date
        };
        
        std::vector<Exs_NetworkAdapter> adapters;
    };
    
    // Storage Information
    struct Exs_StorageInfo {
        uint32_t diskCount;             // Number of disks
        
        struct Exs_DiskDrive {
            std::string name;           // Disk name
            std::string model;          // Disk model
            std::string serialNumber;   // Serial number
            std::string firmwareVersion; // Firmware version
            std::string interfaceType;  // Interface type (SATA, NVMe, etc.)
            uint64_t size;              // Total size in bytes
            uint64_t freeSpace;         // Free space in bytes
            uint64_t usedSpace;         // Used space in bytes
            uint32_t sectorSize;        // Sector size in bytes
            uint32_t partitionCount;    // Number of partitions
            bool isSSD;                 // Is solid state drive
            bool isRemovable;           // Is removable drive
            bool isBootDrive;           // Is boot drive
            bool isSystemDrive;         // Is system drive
            uint32_t readSpeed;         // Read speed in MB/s
            uint32_t writeSpeed;        // Write speed in MB/s
            uint32_t temperature;       // Temperature in Celsius
            uint32_t powerOnHours;      // Power on hours
            uint32_t healthStatus;      // Health status percentage
        };
        
        std::vector<Exs_DiskDrive> disks;
        
        // Volume information
        struct Exs_Volume {
            std::string name;           // Volume name
            std::string mountPoint;     // Mount point
            std::string fileSystem;     // File system type
            uint64_t totalSize;         // Total size in bytes
            uint64_t freeSize;          // Free size in bytes
            uint64_t usedSize;          // Used size in bytes
            uint32_t serialNumber;      // Volume serial number
            bool isCompressed;          // Is volume compressed
            bool isEncrypted;           // Is volume encrypted
            bool isReadOnly;            // Is volume read-only
            bool isRemovable;           // Is volume removable
        };
        
        std::vector<Exs_Volume> volumes;
    };
    
    // Audio Information
    struct Exs_AudioInfo {
        uint32_t audioDeviceCount;      // Number of audio devices
        
        struct Exs_AudioDevice {
            std::string name;           // Device name
            std::string description;    // Device description
            std::string manufacturer;   // Manufacturer
            std::string driverVersion;  // Driver version
            bool isDefaultPlayback;     // Is default playback device
            bool isDefaultRecording;    // Is default recording device
            uint32_t channels;          // Number of channels
            uint32_t sampleRate;        // Sample rate in Hz
            uint32_t bitsPerSample;     // Bits per sample
            bool isActive;              // Is device active
            bool isPlugged;             // Is device plugged in
        };
        
        std::vector<Exs_AudioDevice> playbackDevices;
        std::vector<Exs_AudioDevice> recordingDevices;
    };
    
    // Input Device Information
    struct Exs_InputInfo {
        uint32_t keyboardCount;         // Number of keyboards
        uint32_t mouseCount;            // Number of mice
        uint32_t gamepadCount;          // Number of gamepads
        uint32_t touchDeviceCount;      // Number of touch devices
        
        struct Exs_InputDevice {
            std::string type;           // Device type
            std::string name;           // Device name
            std::string manufacturer;   // Manufacturer
            std::string productId;      // Product ID
            std::string vendorId;       // Vendor ID
            uint32_t buttons;           // Number of buttons
            uint32_t axes;              // Number of axes
            bool hasForceFeedback;      // Has force feedback
            bool isWireless;            // Is wireless device
            uint32_t batteryLevel;      // Battery level percentage
        };
        
        std::vector<Exs_InputDevice> devices;
    };
    
    // BIOS/UEFI Information
    struct Exs_BIOSInfo {
        std::string vendor;             // BIOS vendor
        std::string version;            // BIOS version
        std::string releaseDate;        // BIOS release date
        std::string firmwareType;       // BIOS or UEFI
        uint64_t size;                  // BIOS size
        std::string serialNumber;       // System serial number
        std::string assetTag;           // Asset tag
        std::string boardManufacturer;  // Board manufacturer
        std::string boardProduct;       // Board product
        std::string boardVersion;       // Board version
        std::string boardSerialNumber;  // Board serial number
    };
    
    // Security Information
    struct Exs_SecurityInfo {
        bool isSecureBootEnabled;       // Is Secure Boot enabled
        bool isTPMAvailable;            // Is TPM available
        bool isTPMEnabled;              // Is TPM enabled
        std::string tpmVersion;         // TPM version
        std::string tpmManufacturer;    // TPM manufacturer
        bool isVirtualizationEnabled;   // Is CPU virtualization enabled
        bool isNXEnabled;               // Is NX/XD bit enabled
        bool isDEPEnabled;              // Is Data Execution Prevention enabled
        bool isASLREnabled;             // Is ASLR enabled
        bool isFirewallEnabled;         // Is firewall enabled
        bool isAntivirusInstalled;      // Is antivirus installed
        bool isAntivirusEnabled;        // Is antivirus enabled
        std::string antivirusName;      // Antivirus name
        bool isAntiSpywareInstalled;    // Is anti-spyware installed
        bool isAntiSpywareEnabled;      // Is anti-spyware enabled
        std::string antiSpywareName;    // Anti-spyware name
    };
    
    // Virtualization Information
    struct Exs_VirtualizationInfo {
        bool isVirtualMachine;          // Is running in VM
        std::string virtualizationType; // Hypervisor type
        std::string virtualMachineName; // VM name
        std::string vendor;             // Hypervisor vendor
        std::string version;            // Hypervisor version
        bool isNestedVirtualization;    // Is nested virtualization
        bool isDockerAvailable;         // Is Docker available
        bool isWSLAvailable;            // Is WSL available
        bool isWSL2Available;           // Is WSL 2 available
    };
    
    // Performance Information
    struct Exs_PerformanceInfo {
        uint64_t uptime;                // System uptime in seconds
        uint32_t processCount;          // Number of running processes
        uint32_t threadCount;           // Number of running threads
        uint32_t handleCount;           // Number of open handles
        uint32_t sessionCount;          // Number of user sessions
        uint64_t commitTotal;           // Total commit charge
        uint64_t commitLimit;           // Commit charge limit
        uint64_t commitPeak;            // Peak commit charge
        uint64_t kernelTotal;           // Total kernel memory
        uint64_t kernelPaged;           // Paged kernel memory
        uint64_t kernelNonPaged;        // Non-paged kernel memory
        uint32_t systemCacheSize;       // System cache size
        uint32_t pageFaultCount;        // Page fault count
        uint32_t systemCallsCount;      // System calls count
        float interruptTime;            // Time spent in interrupts
        float dpcTime;                  // Time spent in DPCs
        float systemTime;               // Time spent in system
        float userTime;                 // Time spent in user mode
        float idleTime;                 // Time spent idle
    };
    
    // Environment Information
    struct Exs_EnvironmentInfo {
        std::string computerName;       // %COMPUTERNAME%
        std::string userProfile;        // %USERPROFILE%
        std::string systemRoot;         // %SystemRoot%
        std::string programFiles;       // %ProgramFiles%
        std::string programFilesX86;    // %ProgramFiles(x86)%
        std::string tempPath;           // %TEMP%
        std::string windir;             // %WINDIR%
        std::string path;               // %PATH%
        std::string processorArchitecture; // %PROCESSOR_ARCHITECTURE%
        std::string processorIdentifier; // %PROCESSOR_IDENTIFIER%
        std::string processorLevel;     // %PROCESSOR_LEVEL%
        std::string processorRevision;  // %PROCESSOR_REVISION%
        uint32_t processorCount;        // %NUMBER_OF_PROCESSORS%
        std::string os;                 // %OS%
        std::string userName;           // %USERNAME%
        std::string userDomain;         // %USERDOMAIN%
        std::string logonServer;        // %LOGONSERVER%
    };
    
    // All information categories
    Exs_OSInfo os;
    Exs_ComputerInfo computer;
    Exs_DisplayInfo display;
    Exs_NetworkInfo network;
    Exs_StorageInfo storage;
    Exs_AudioInfo audio;
    Exs_InputInfo input;
    Exs_BIOSInfo bios;
    Exs_SecurityInfo security;
    Exs_VirtualizationInfo virtualization;
    Exs_PerformanceInfo performance;
    Exs_EnvironmentInfo environment;
    
    // Timestamp
    uint64_t timestamp;                 // When info was collected
};

// System Information Manager
class EXS_API Exs_SystemInfoManager {
private:
    static Exs_SystemInfoManager* Exs_instance;
    Exs_SystemInfo Exs_currentInfo;
    bool Exs_initialized;
    
    // Private constructor for singleton
    Exs_SystemInfoManager();
    ~Exs_SystemInfoManager();
    
    // Detection methods
    void Exs_DetectOSInfo();
    void Exs_DetectComputerInfo();
    void Exs_DetectDisplayInfo();
    void Exs_DetectNetworkInfo();
    void Exs_DetectStorageInfo();
    void Exs_DetectAudioInfo();
    void Exs_DetectInputInfo();
    void Exs_DetectBIOSInfo();
    void Exs_DetectSecurityInfo();
    void Exs_DetectVirtualizationInfo();
    void Exs_DetectPerformanceInfo();
    void Exs_DetectEnvironmentInfo();
    
    // Platform-specific implementations
#if defined(EXS_PLATFORM_WINDOWS)
    void Exs_DetectWindowsOSInfo();
    void Exs_DetectWindowsComputerInfo();
    void Exs_DetectWindowsDisplayInfo();
    void Exs_DetectWindowsNetworkInfo();
    void Exs_DetectWindowsStorageInfo();
    void Exs_DetectWindowsAudioInfo();
    void Exs_DetectWindowsInputInfo();
    void Exs_DetectWindowsBIOSInfo();
    void Exs_DetectWindowsSecurityInfo();
    void Exs_DetectWindowsVirtualizationInfo();
    void Exs_DetectWindowsPerformanceInfo();
    void Exs_DetectWindowsEnvironmentInfo();
#elif defined(EXS_PLATFORM_LINUX)
    void Exs_DetectLinuxOSInfo();
    void Exs_DetectLinuxComputerInfo();
    void Exs_DetectLinuxDisplayInfo();
    void Exs_DetectLinuxNetworkInfo();
    void Exs_DetectLinuxStorageInfo();
    void Exs_DetectLinuxAudioInfo();
    void Exs_DetectLinuxInputInfo();
    void Exs_DetectLinuxBIOSInfo();
    void Exs_DetectLinuxSecurityInfo();
    void Exs_DetectLinuxVirtualizationInfo();
    void Exs_DetectLinuxPerformanceInfo();
    void Exs_DetectLinuxEnvironmentInfo();
#elif defined(EXS_PLATFORM_APPLE)
    void Exs_DetectMacOSInfo();
    void Exs_DetectMacComputerInfo();
    void Exs_DetectMacDisplayInfo();
    void Exs_DetectMacNetworkInfo();
    void Exs_DetectMacStorageInfo();
    void Exs_DetectMacAudioInfo();
    void Exs_DetectMacInputInfo();
    void Exs_DetectMacBIOSInfo();
    void Exs_DetectMacSecurityInfo();
    void Exs_DetectMacVirtualizationInfo();
    void Exs_DetectMacPerformanceInfo();
    void Exs_DetectMacEnvironmentInfo();
#endif
    
public:
    // Singleton access
    static Exs_SystemInfoManager& Exs_GetInstance();
    static void Exs_Destroy();
    
    // Initialization
    void Exs_Initialize();
    void Exs_Shutdown();
    bool Exs_IsInitialized() const { return Exs_initialized; }
    
    // Information access
    const Exs_SystemInfo& Exs_GetSystemInfo() const { return Exs_currentInfo; }
    void Exs_RefreshSystemInfo();
    
    // Individual category access
    const Exs_SystemInfo::Exs_OSInfo& Exs_GetOSInfo() const { return Exs_currentInfo.os; }
    const Exs_SystemInfo::Exs_ComputerInfo& Exs_GetComputerInfo() const { return Exs_currentInfo.computer; }
    const Exs_SystemInfo::Exs_DisplayInfo& Exs_GetDisplayInfo() const { return Exs_currentInfo.display; }
    const Exs_SystemInfo::Exs_NetworkInfo& Exs_GetNetworkInfo() const { return Exs_currentInfo.network; }
    const Exs_SystemInfo::Exs_StorageInfo& Exs_GetStorageInfo() const { return Exs_currentInfo.storage; }
    const Exs_SystemInfo::Exs_AudioInfo& Exs_GetAudioInfo() const { return Exs_currentInfo.audio; }
    const Exs_SystemInfo::Exs_InputInfo& Exs_GetInputInfo() const { return Exs_currentInfo.input; }
    const Exs_SystemInfo::Exs_BIOSInfo& Exs_GetBIOSInfo() const { return Exs_currentInfo.bios; }
    const Exs_SystemInfo::Exs_SecurityInfo& Exs_GetSecurityInfo() const { return Exs_currentInfo.security; }
    const Exs_SystemInfo::Exs_VirtualizationInfo& Exs_GetVirtualizationInfo() const { return Exs_currentInfo.virtualization; }
    const Exs_SystemInfo::Exs_PerformanceInfo& Exs_GetPerformanceInfo() const { return Exs_currentInfo.performance; }
    const Exs_SystemInfo::Exs_EnvironmentInfo& Exs_GetEnvironmentInfo() const { return Exs_currentInfo.environment; }
    
    // Utility functions
    std::string Exs_ToString() const;
    void Exs_PrintInfo() const;
    void Exs_SaveToFile(const std::string& filename) const;
    void Exs_LoadFromFile(const std::string& filename);
    
    // Query functions
    bool Exs_Is64BitOS() const;
    bool Exs_IsServerEdition() const;
    bool Exs_IsVirtualMachine() const;
    bool Exs_IsPortableComputer() const;
    bool Exs_IsTabletMode() const;
    bool Exs_IsHighContrastMode() const;
    bool Exs_IsRemoteSession() const;
    bool Exs_IsTerminalServicesEnabled() const;
    
    // Version checks
    bool Exs_IsWindowsVersionOrGreater(uint32_t major, uint32_t minor, uint32_t build) const;
    bool Exs_IsLinuxDistribution(const std::string& distro) const;
    bool Exs_IsMacOSVersionOrGreater(uint32_t major, uint32_t minor, uint32_t patch) const;
    
    // Feature checks
    bool Exs_IsFeaturePresent(const std::string& feature) const;
    bool Exs_IsCapabilityPresent(const std::string& capability) const;
    
    // Performance monitoring
    void Exs_StartPerformanceMonitoring();
    void Exs_StopPerformanceMonitoring();
    void Exs_GetPerformanceCounters(std::vector<float>& counters) const;
    
    // System metrics
    uint32_t Exs_GetSystemMetric(uint32_t metric) const;
    uint32_t Exs_GetSystemColor(uint32_t colorIndex) const;
    uint32_t Exs_GetSystemParameter(uint32_t parameter) const;
    
    // Power status
    bool Exs_GetSystemPowerStatus(uint32_t& batteryLifePercent, uint32_t& batteryLifeTime, 
                                 uint32_t& batteryFullLifeTime, uint8_t& batteryFlag, 
                                 uint8_t& acLineStatus) const;
    
    // System events
    void Exs_RegisterSystemEventHandler(void (*handler)(uint32_t event, void* data));
    void Exs_UnregisterSystemEventHandler(void (*handler)(uint32_t event, void* data));
    
    // System configuration
    bool Exs_SetSystemConfiguration(const std::string& key, const std::string& value);
    std::string Exs_GetSystemConfiguration(const std::string& key) const;
    
    // System commands
    bool Exs_ExecuteSystemCommand(const std::string& command, std::string& output);
    bool Exs_ExecutePrivilegedCommand(const std::string& command, std::string& output);
    
private:
    // Prevent copying
    Exs_SystemInfoManager(const Exs_SystemInfoManager&) = delete;
    Exs_SystemInfoManager& operator=(const Exs_SystemInfoManager&) = delete;
};

// Global helper functions
EXS_API const Exs_SystemInfo& Exs_GetSystemInfo();
EXS_API std::string Exs_GetOSName();
EXS_API std::string Exs_GetOSVersion();
EXS_API std::string Exs_GetComputerName();
EXS_API std::string Exs_GetUserName();
EXS_API bool Exs_Is64BitOperatingSystem();
EXS_API bool Exs_IsAdministrator();
EXS_API uint64_t Exs_GetSystemUptime();
EXS_API uint32_t Exs_GetSystemLanguage();
EXS_API std::string Exs_GetSystemLocale();
EXS_API std::string Exs_GetTimeZone();
EXS_API bool Exs_IsDaylightSavingTime();

} // namespace Platform
} // namespace Internal
} // namespace Exs

#endif // EXS_SYSTEMINFO_H
