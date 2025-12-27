// src/Core/Platform/internal/SecurityInfoBase.h
#ifndef EXS_INTERNAL_SECURITY_INFO_BASE_H
#define EXS_INTERNAL_SECURITY_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <chrono>

namespace Exs {
namespace Internal {
namespace SecurityInfo {

// Security provider types
enum class Exs_SecurityProvider {
    Unknown = 0,
    WindowsDefender = 1,
    WindowsSecurityCenter = 2,
    Antivirus = 3,
    Firewall = 4,
    AntiSpyware = 5,
    InternetSecurity = 6,
    TPM = 7,
    BitLocker = 8,
    CredentialGuard = 9,
    ApplicationGuard = 10
};

// Security product state
enum class Exs_SecurityProductState {
    Unknown = 0,
    Off = 1,
    On = 2,
    Snoozed = 3,
    Expired = 4,
    Disabled = 5,
    Enabled = 6
};

// Security threat level
enum class Exs_SecurityThreatLevel {
    Unknown = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Severe = 4
};

// Security product information
struct Exs_SecurityProductInfo {
    std::string name;
    std::string vendor;
    Exs_SecurityProvider provider;
    Exs_SecurityProductState state;
    std::string version;
    std::string definitionVersion;
    std::chrono::system_clock::time_point lastUpdate;
    std::chrono::system_clock::time_point expiryDate;
    bool isRealTimeProtectionEnabled;
    bool isAutoUpdateEnabled;
    bool isTamperProtected;
};

// Security threat information
struct Exs_SecurityThreatInfo {
    std::string id;
    std::string name;
    std::string description;
    Exs_SecurityThreatLevel level;
    std::string category; // Virus, Trojan, Spyware, etc.
    std::string source; // File path, URL, etc.
    std::chrono::system_clock::time_point detectionTime;
    bool isQuarantined;
    bool isRemoved;
    std::string actionTaken;
};

// Firewall rule information
struct Exs_FirewallRuleInfo {
    std::string name;
    std::string description;
    std::string direction; // Inbound, Outbound
    std::string action; // Allow, Block
    std::string protocol; // TCP, UDP, Any
    uint32 localPort;
    uint32 remotePort;
    std::string localAddress;
    std::string remoteAddress;
    std::string programPath;
    bool isEnabled;
    std::string profile; // Domain, Private, Public
};

// TPM (Trusted Platform Module) information
struct Exs_TPMInfo {
    std::string manufacturer;
    std::string version;
    uint32 specVersion;
    bool isPresent;
    bool isEnabled;
    bool isActivated;
    bool isOwned;
    std::string manufacturerId;
    std::string manufacturerVersion;
    uint32 physicalPresenceVersion;
    uint32 logicalDeviceCount;
    std::vector<std::string> supportedAlgorithms;
    uint32 maxNVBufferSize;
    uint32 totalNVSize;
};

// Encryption information
struct Exs_EncryptionInfo {
    bool isBitLockerEnabled;
    std::string bitLockerVersion;
    std::string bitLockerProtectionStatus;
    uint32 bitLockerEncryptionPercentage;
    std::string bitLockerRecoveryKeyId;
    
    bool isEFSEnabled;
    uint32 efsCertificateCount;
    
    bool isDeviceEncryptionEnabled;
    std::string encryptionMethod;
    std::string encryptionProvider;
};

// User account security
struct Exs_UserAccountSecurity {
    std::string username;
    bool isAdministrator;
    bool isPasswordProtected;
    bool isPasswordExpired;
    bool isAccountLocked;
    bool isAccountDisabled;
    std::chrono::system_clock::time_point passwordLastSet;
    std::chrono::system_clock::time_point accountExpires;
    uint32 failedLoginAttempts;
    std::chrono::system_clock::time_point lastLogin;
    std::vector<std::string> groupMemberships;
};

// Security policy information
struct Exs_SecurityPolicy {
    std::string name;
    std::string description;
    std::string category;
    std::string currentValue;
    std::string defaultValue;
    std::vector<std::string> possibleValues;
    bool isEnforced;
    std::string enforcementMethod;
};

// Base security info class
class Exs_SecurityInfoBase {
public:
    virtual ~Exs_SecurityInfoBase() = default;
    
    // Security products
    virtual std::vector<Exs_SecurityProductInfo> getSecurityProducts() const = 0;
    virtual Exs_SecurityProductInfo getAntivirusInfo() const = 0;
    virtual Exs_SecurityProductInfo getFirewallInfo() const = 0;
    virtual Exs_SecurityProductInfo getAntiSpywareInfo() const = 0;
    virtual bool isAntivirusEnabled() const = 0;
    virtual bool isFirewallEnabled() const = 0;
    
    // Threat detection
    virtual std::vector<Exs_SecurityThreatInfo> getDetectedThreats() const = 0;
    virtual uint32 getThreatCount() const = 0;
    virtual uint32 getThreatCountByLevel(Exs_SecurityThreatLevel level) const = 0;
    virtual bool scanForThreats(const std::string& path = "") = 0;
    virtual bool removeThreat(const std::string& threatId) = 0;
    virtual bool quarantineThreat(const std::string& threatId) = 0;
    
    // Firewall management
    virtual std::vector<Exs_FirewallRuleInfo> getFirewallRules() const = 0;
    virtual bool addFirewallRule(const Exs_FirewallRuleInfo& rule) = 0;
    virtual bool removeFirewallRule(const std::string& ruleName) = 0;
    virtual bool enableFirewallRule(const std::string& ruleName) = 0;
    virtual bool disableFirewallRule(const std::string& ruleName) = 0;
    virtual bool isFirewallRuleEnabled(const std::string& ruleName) const = 0;
    
    // TPM management
    virtual Exs_TPMInfo getTPMInfo() const = 0;
    virtual bool isTPMPresent() const = 0;
    virtual bool isTPMEnabled() const = 0;
    virtual bool enableTPM() = 0;
    virtual bool disableTPM() = 0;
    virtual bool clearTPM() = 0;
    virtual std::string getTPMManufacturer() const = 0;
    virtual std::vector<uint8> getTPMEndorsementKey() const = 0;
    
    // Encryption management
    virtual Exs_EncryptionInfo getEncryptionInfo() const = 0;
    virtual bool isBitLockerEnabled() const = 0;
    virtual bool enableBitLocker(const std::string& password) = 0;
    virtual bool disableBitLocker() = 0;
    virtual bool suspendBitLocker() = 0;
    virtual bool resumeBitLocker() = 0;
    virtual std::string getBitLockerRecoveryKey() const = 0;
    
    // User account security
    virtual std::vector<Exs_UserAccountSecurity> getUserAccounts() const = 0;
    virtual Exs_UserAccountSecurity getCurrentUserAccount() const = 0;
    virtual bool isUserAdministrator() const = 0;
    virtual bool changeUserPassword(const std::string& newPassword) = 0;
    virtual bool lockUserAccount(const std::string& username) = 0;
    virtual bool unlockUserAccount(const std::string& username) = 0;
    
    // Security policies
    virtual std::vector<Exs_SecurityPolicy> getSecurityPolicies() const = 0;
    virtual Exs_SecurityPolicy getSecurityPolicy(const std::string& name) const = 0;
    virtual bool setSecurityPolicy(const std::string& name, const std::string& value) = 0;
    virtual bool resetSecurityPolicy(const std::string& name) = 0;
    
    // Security updates
    virtual bool areSecurityUpdatesAvailable() const = 0;
    virtual std::vector<std::string> getAvailableSecurityUpdates() const = 0;
    virtual bool installSecurityUpdates() = 0;
    virtual std::chrono::system_clock::time_point getLastUpdateTime() const = 0;
    
    // Security auditing
    virtual bool enableSecurityAuditing() = 0;
    virtual bool disableSecurityAuditing() = 0;
    virtual bool isSecurityAuditingEnabled() const = 0;
    virtual std::vector<std::string> getSecurityAuditLogs() const = 0;
    virtual bool clearSecurityAuditLogs() = 0;
    
    // Certificate management
    virtual std::vector<std::string> getInstalledCertificates() const = 0;
    virtual bool installCertificate(const std::string& certificatePath) = 0;
    virtual bool removeCertificate(const std::string& thumbprint) = 0;
    virtual bool isCertificateValid(const std::string& thumbprint) const = 0;
    
    // Network security
    virtual bool isNetworkSecurityEnabled() const = 0;
    virtual std::string getNetworkSecurityLevel() const = 0;
    virtual bool setNetworkSecurityLevel(const std::string& level) = 0;
    
    // Application security
    virtual std::vector<std::string> getRunningApplications() const = 0;
    virtual bool isApplicationTrusted(const std::string& path) const = 0;
    virtual bool trustApplication(const std::string& path) = 0;
    virtual bool untrustApplication(const std::string& path) = 0;
    
    // Browser security
    virtual bool isBrowserProtected() const = 0;
    virtual std::vector<std::string> getBrowserExtensions() const = 0;
    virtual bool isBrowserExtensionSafe(const std::string& extensionId) const = 0;
    
    // Email security
    virtual bool isEmailProtected() const = 0;
    virtual bool isPhishingProtectionEnabled() const = 0;
    virtual bool enablePhishingProtection(bool enable) = 0;
    
    // Privacy settings
    virtual bool isLocationTrackingEnabled() const = 0;
    virtual bool enableLocationTracking(bool enable) = 0;
    virtual bool isCameraAccessEnabled() const = 0;
    virtual bool enableCameraAccess(bool enable) = 0;
    virtual bool isMicrophoneAccessEnabled() const = 0;
    virtual bool enableMicrophoneAccess(bool enable) = 0;
    
    // Security score
    virtual uint32 getSecurityScore() const = 0; // 0-100
    virtual std::vector<std::string> getSecurityRecommendations() const = 0;
    
    // Malware protection
    virtual bool isRealTimeProtectionEnabled() const = 0;
    virtual bool enableRealTimeProtection(bool enable) = 0;
    virtual bool isCloudProtectionEnabled() const = 0;
    virtual bool enableCloudProtection(bool enable) = 0;
    
    // Parental controls
    virtual bool areParentalControlsEnabled() const = 0;
    virtual bool enableParentalControls(bool enable) = 0;
    virtual std::vector<std::string> getParentalControlSettings() const = 0;
    
    // Data loss prevention
    virtual bool isDLPEnabled() const = 0;
    virtual bool enableDLP(bool enable) = 0;
    virtual std::vector<std::string> getDLPPolicies() const = 0;
};

// Factory function
Exs_SecurityInfoBase* Exs_CreateSecurityInfoInstance();

} // namespace SecurityInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_SECURITY_INFO_BASE_H
