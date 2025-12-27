// src/Core/Platform/internal/BIOSInfoBase.h
#ifndef EXS_INTERNAL_BIOS_INFO_BASE_H
#define EXS_INTERNAL_BIOS_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <chrono>

namespace Exs {
namespace Internal {
namespace BIOSInfo {

// BIOS type enumeration
enum class Exs_BIOSType {
    Unknown = 0,
    Legacy = 1,
    UEFI = 2,
    Coreboot = 3,
    OpenBIOS = 4,
    OpenFirmware = 5,
    AppleEFI = 6
};

// BIOS security features
struct Exs_BIOSSecurityFeatures {
    bool secureBoot;
    bool tpmSupport;
    bool measuredBoot;
    bool iommuSupport;
    bool smmProtection;
    bool biosWriteProtection;
    bool adminPasswordSet;
    bool powerOnPasswordSet;
    bool bootFromExternalDisabled;
    bool bootFromNetworkDisabled;
    bool usbPortsDisabled;
};

// BIOS settings category
struct Exs_BIOSSettingCategory {
    std::string name;
    std::string description;
    std::vector<std::string> settings;
};

// BIOS setting value
struct Exs_BIOSSettingValue {
    std::string name;
    std::string currentValue;
    std::string defaultValue;
    std::vector<std::string> possibleValues;
    bool isReadOnly;
    bool requiresReboot;
    std::string description;
};

// Base BIOS info class
class Exs_BIOSInfoBase {
public:
    virtual ~Exs_BIOSInfoBase() = default;
    
    // BIOS identification
    virtual std::string getBIOSVendor() const = 0;
    virtual std::string getBIOSVersion() const = 0;
    virtual std::string getBIOSDate() const = 0;
    virtual std::string getBIOSReleaseDate() const = 0;
    virtual Exs_BIOSType getBIOSType() const = 0;
    
    // Firmware information
    virtual std::string getFirmwareVendor() const = 0;
    virtual std::string getFirmwareVersion() const = 0;
    virtual std::string getFirmwareRevision() const = 0;
    virtual uint32 getFirmwareSize() const = 0; // in KB
    
    // System information from BIOS
    virtual std::string getSystemManufacturer() const = 0;
    virtual std::string getSystemProductName() const = 0;
    virtual std::string getSystemVersion() const = 0;
    virtual std::string getSystemSerialNumber() const = 0;
    virtual std::string getSystemUUID() const = 0;
    virtual std::string getSystemSKU() const = 0;
    virtual std::string getSystemFamily() const = 0;
    
    // Baseboard information
    virtual std::string getBaseboardManufacturer() const = 0;
    virtual std::string getBaseboardProduct() const = 0;
    virtual std::string getBaseboardVersion() const = 0;
    virtual std::string getBaseboardSerialNumber() const = 0;
    virtual std::string getBaseboardAssetTag() const = 0;
    
    // Chassis information
    virtual std::string getChassisManufacturer() const = 0;
    virtual std::string getChassisType() const = 0;
    virtual std::string getChassisVersion() const = 0;
    virtual std::string getChassisSerialNumber() const = 0;
    virtual std::string getChassisAssetTag() const = 0;
    
    // BIOS capabilities
    virtual bool supportsUEFI() const = 0;
    virtual bool supportsLegacyBoot() const = 0;
    virtual bool supportsNetworkBoot() const = 0;
    virtual bool supportsSecureBoot() const = 0;
    virtual bool supportsTPM() const = 0;
    virtual bool supportsVirtualization() const = 0;
    
    // BIOS settings
    virtual std::vector<Exs_BIOSSettingCategory> getBIOSSettingCategories() const = 0;
    virtual std::vector<Exs_BIOSSettingValue> getBIOSSettings(const std::string& category) const = 0;
    virtual Exs_BIOSSettingValue getBIOSSetting(const std::string& name) const = 0;
    virtual bool setBIOSSetting(const std::string& name, const std::string& value) = 0;
    virtual bool resetBIOSSetting(const std::string& name) = 0;
    virtual bool resetAllBIOSSettings() = 0;
    
    // BIOS security
    virtual Exs_BIOSSecurityFeatures getSecurityFeatures() const = 0;
    virtual bool isSecureBootEnabled() const = 0;
    virtual bool enableSecureBoot(bool enable) = 0;
    virtual bool isTPMEnabled() const = 0;
    virtual bool enableTPM(bool enable) = 0;
    
    // BIOS passwords
    virtual bool hasAdminPassword() const = 0;
    virtual bool hasPowerOnPassword() const = 0;
    virtual bool setAdminPassword(const std::string& password) = 0;
    virtual bool setPowerOnPassword(const std::string& password) = 0;
    virtual bool clearAdminPassword() = 0;
    virtual bool clearPowerOnPassword() = 0;
    
    // Boot order
    virtual std::vector<std::string> getBootOrder() const = 0;
    virtual bool setBootOrder(const std::vector<std::string>& order) = 0;
    virtual bool addBootOption(const std::string& name, const std::string& device, const std::string& path) = 0;
    virtual bool removeBootOption(const std::string& name) = 0;
    
    // BIOS updates
    virtual bool isBIOSUpdateAvailable() const = 0;
    virtual std::string getAvailableBIOSVersion() const = 0;
    virtual bool updateBIOS(const std::string& updateFile) = 0;
    virtual bool backupBIOS(const std::string& backupFile) = 0;
    virtual bool restoreBIOS(const std::string& backupFile) = 0;
    
    // BIOS health
    virtual bool isBIOSCorrupted() const = 0;
    virtual bool performBIOSHealthCheck() const = 0;
    virtual uint32 getBIOSChecksum() const = 0;
    
    // Power management
    virtual bool isWakeOnLANEnabled() const = 0;
    virtual bool enableWakeOnLAN(bool enable) = 0;
    virtual bool isWakeOnRTCEnabled() const = 0;
    virtual bool enableWakeOnRTC(bool enable) = 0;
    
    // Hardware monitoring
    virtual std::vector<std::string> getMonitoredTemperatures() const = 0;
    virtual std::vector<std::string> getMonitoredVoltages() const = 0;
    virtual std::vector<std::string> getMonitoredFanSpeeds() const = 0;
    
    // Event log
    virtual std::vector<std::string> getBIOSEventLog() const = 0;
    virtual bool clearBIOSEventLog() = 0;
    
    // ACPI information
    virtual std::string getACPIVersion() const = 0;
    virtual bool supportsACPI() const = 0;
    virtual std::vector<std::string> getACPITables() const = 0;
    virtual std::string getACPITable(const std::string& tableName) const = 0;
    
    // SMBIOS information
    virtual std::string getSMBIOSVersion() const = 0;
    virtual bool supportsSMBIOS() const = 0;
    virtual std::vector<std::string> getSMBIOSTables() const = 0;
    
    // BIOS memory map
    virtual std::vector<std::tuple<uint64, uint64, std::string>> getBIOSMemoryMap() const = 0;
    virtual uint64 getBIOSReservedMemory() const = 0;
    
    // Boot time
    virtual uint32 getBIOSBootTime() const = 0; // in milliseconds
    virtual bool setFastBoot(bool enable) = 0;
    virtual bool isFastBootEnabled() const = 0;
    
    // Overclocking support
    virtual bool supportsOverclocking() const = 0;
    virtual std::vector<std::string> getOverclockingOptions() const = 0;
    
    // BIOS backup and restore
    virtual std::vector<uint8> dumpBIOS() const = 0;
    virtual bool flashBIOS(const std::vector<uint8>& biosImage) = 0;
    
    // BIOS verification
    virtual bool verifyBIOSSignature() const = 0;
    virtual std::string getBIOSSignature() const = 0;
    
    // Legacy features
    virtual bool supportsPS2Emulation() const = 0;
    virtual bool supportsUSBEmulation() const = 0;
    virtual bool supportsPXE() const = 0;
    
    // Debug features
    virtual bool supportsSerialConsole() const = 0;
    virtual bool supportsPOSTCodeDisplay() const = 0;
    
    // Manufacturer specific
    virtual std::string getManufacturerSupportURL() const = 0;
    virtual std::string getManufacturerSupportPhone() const = 0;
    virtual std::string getManufacturerWarrantyInfo() const = 0;
    
    // Custom logo
    virtual bool hasCustomBootLogo() const = 0;
    virtual bool setCustomBootLogo(const std::vector<uint8>& logoData) = 0;
    virtual bool removeCustomBootLogo() = 0;
};

// Factory function
Exs_BIOSInfoBase* Exs_CreateBIOSInfoInstance();

} // namespace BIOSInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_BIOS_INFO_BASE_H
