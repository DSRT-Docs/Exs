// src/Core/Platform/internal/NetworkInfoBase.h
#ifndef EXS_INTERNAL_NETWORK_INFO_BASE_H
#define EXS_INTERNAL_NETWORK_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <array>

namespace Exs {
namespace Internal {
namespace NetworkInfo {

// IP address types
enum class Exs_IPAddressType {
    Unknown = 0,
    IPv4 = 1,
    IPv6 = 2
};

// Network interface types
enum class Exs_NetworkInterfaceType {
    Unknown = 0,
    Ethernet = 1,
    WiFi = 2,
    Bluetooth = 3,
    Cellular = 4,
    Virtual = 5,
    Loopback = 6,
    Tunnel = 7
};

// Network interface status
enum class Exs_NetworkInterfaceStatus {
    Unknown = 0,
    Disconnected = 1,
    Connecting = 2,
    Connected = 3,
    Disconnecting = 4,
    Error = 5
};

// IP address structure
struct Exs_IPAddress {
    Exs_IPAddressType type;
    std::array<uint8, 16> address; // IPv4 or IPv6
    uint32 prefixLength; // Subnet mask in CIDR notation
    bool isLoopback;
    bool isPrivate;
    bool isLinkLocal;
    
    std::string toString() const;
    static Exs_IPAddress fromString(const std::string& str);
};

// Network interface information
struct Exs_NetworkInterfaceInfo {
    std::string name;
    std::string description;
    Exs_NetworkInterfaceType type;
    Exs_NetworkInterfaceStatus status;
    std::string macAddress;
    uint64 speed; // in bits per second
    uint32 mtu;
    
    std::vector<Exs_IPAddress> ipAddresses;
    std::vector<Exs_IPAddress> dnsServers;
    std::vector<Exs_IPAddress> gateways;
    
    bool isDhcpEnabled;
    bool isDnsEnabled;
    bool isFirewallEnabled;
    
    uint64 bytesSent;
    uint64 bytesReceived;
    uint64 packetsSent;
    uint64 packetsReceived;
    uint64 errorsSent;
    uint64 errorsReceived;
    
    std::string dhcpServer;
    std::string dhcpLeaseObtained;
    std::string dhcpLeaseExpires;
};

// Network connection information
struct Exs_NetworkConnectionInfo {
    std::string protocol;
    std::string localAddress;
    uint16 localPort;
    std::string remoteAddress;
    uint16 remotePort;
    std::string state;
    uint32 processId;
    std::string processName;
    
    uint64 bytesSent;
    uint64 bytesReceived;
    std::chrono::system_clock::time_point connectionTime;
};

// Network adapter statistics
struct Exs_NetworkStatistics {
    uint64 totalBytesSent;
    uint64 totalBytesReceived;
    uint64 totalPacketsSent;
    uint64 totalPacketsReceived;
    uint64 totalErrorsSent;
    uint64 totalErrorsReceived;
    uint64 totalCollisions;
    
    uint64 tcpConnections;
    uint64 udpConnections;
    uint64 activeConnections;
    uint64 failedConnections;
    
    double bandwidthUsage; // Percentage
    double packetLossRate;
    double latency; // milliseconds
};

// Network quality
enum class Exs_NetworkQuality {
    Unknown = 0,
    Excellent = 1,
    Good = 2,
    Fair = 3,
    Poor = 4,
    Bad = 5
};

// Base network info class
class Exs_NetworkInfoBase {
public:
    virtual ~Exs_NetworkInfoBase() = default;
    
    // Network interfaces
    virtual std::vector<Exs_NetworkInterfaceInfo> getNetworkInterfaces() const = 0;
    virtual Exs_NetworkInterfaceInfo getNetworkInterface(const std::string& name) const = 0;
    
    // Network connections
    virtual std::vector<Exs_NetworkConnectionInfo> getNetworkConnections() const = 0;
    virtual std::vector<Exs_NetworkConnectionInfo> getNetworkConnectionsByProcess(uint32 pid) const = 0;
    
    // Network statistics
    virtual Exs_NetworkStatistics getNetworkStatistics() const = 0;
    virtual Exs_NetworkStatistics getNetworkStatisticsForInterface(const std::string& interfaceName) const = 0;
    
    // IP address information
    virtual std::vector<Exs_IPAddress> getLocalIPAddresses() const = 0;
    virtual Exs_IPAddress getPrimaryIPAddress() const = 0;
    virtual std::string getHostName() const = 0;
    virtual std::string getDomainName() const = 0;
    
    // DNS information
    virtual std::vector<Exs_IPAddress> getDNSServers() const = 0;
    virtual std::vector<std::string> resolveHostName(const std::string& hostname) const = 0;
    virtual std::string reverseLookup(const Exs_IPAddress& ip) const = 0;
    
    // Network status
    virtual bool isNetworkAvailable() const = 0;
    virtual bool isInternetAvailable() const = 0;
    virtual Exs_NetworkQuality getNetworkQuality() const = 0;
    
    // Network configuration
    virtual bool isFirewallEnabled() const = 0;
    virtual bool isProxyEnabled() const = 0;
    virtual std::string getProxyServer() const = 0;
    
    // Network adapter capabilities
    virtual bool supportsIPv4() const = 0;
    virtual bool supportsIPv6() const = 0;
    virtual bool supportsJumboFrames() const = 0;
    virtual bool supportsWakeOnLAN() const = 0;
    
    // Network throughput
    virtual double getCurrentBandwidthUsage() const = 0;
    virtual double getMaxBandwidth() const = 0;
    
    // Network latency
    virtual double getNetworkLatency(const std::string& host = "8.8.8.8") const = 0;
    virtual double getPacketLoss(const std::string& host = "8.8.8.8") const = 0;
    
    // Network routes
    virtual std::vector<std::string> getRoutingTable() const = 0;
    virtual std::string getDefaultGateway() const = 0;
    
    // ARP table
    virtual std::vector<std::pair<std::string, std::string>> getARPTable() const = 0;
    
    // Network services
    virtual std::vector<std::string> getNetworkServices() const = 0;
    virtual bool isNetworkServiceRunning(const std::string& service) const = 0;
    
    // Network security
    virtual bool isVPNConnected() const = 0;
    virtual std::string getVPNServer() const = 0;
    virtual bool isPortOpen(uint16 port) const = 0;
    
    // Network monitoring
    virtual void startNetworkMonitoring() = 0;
    virtual void stopNetworkMonitoring() = 0;
    virtual bool isNetworkMonitoring() const = 0;
};

// Factory function
Exs_NetworkInfoBase* Exs_CreateNetworkInfoInstance();

} // namespace NetworkInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_NETWORK_INFO_BASE_H
