// src/Core/Platform/Windows/NetworkInfoWindows.cpp
#include "../internal/NetworkInfoBase.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <wtsapi32.h>
#include <netioapi.h>
#include <icmpapi.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "netapi32.lib")

namespace Exs {
namespace Internal {
namespace NetworkInfo {

class Exs_NetworkInfoWindows : public Exs_NetworkInfoBase {
private:
    mutable bool monitoring = false;
    mutable HANDLE monitoringThread = nullptr;
    
public:
    Exs_NetworkInfoWindows() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }
    
    virtual ~Exs_NetworkInfoWindows() {
        stopNetworkMonitoring();
        WSACleanup();
    }
    
    std::vector<Exs_NetworkInterfaceInfo> getNetworkInterfaces() const override {
        std::vector<Exs_NetworkInterfaceInfo> interfaces;
        
        ULONG bufferSize = 0;
        GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &bufferSize);
        
        if (bufferSize == 0) {
            return interfaces;
        }
        
        IP_ADAPTER_ADDRESSES* adapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
        if (!adapterAddresses) {
            return interfaces;
        }
        
        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, 
                                adapterAddresses, &bufferSize) == NO_ERROR) {
            
            for (IP_ADAPTER_ADDRESSES* adapter = adapterAddresses; adapter != nullptr; 
                 adapter = adapter->Next) {
                
                Exs_NetworkInterfaceInfo info;
                
                // Basic info
                info.name = adapter->AdapterName;
                info.description = adapter->Description;
                
                // MAC address
                if (adapter->PhysicalAddressLength > 0) {
                    std::stringstream ss;
                    for (ULONG i = 0; i < adapter->PhysicalAddressLength; i++) {
                        if (i > 0) ss << ":";
                        ss << std::hex << std::setw(2) << std::setfill('0') 
                           << (int)adapter->PhysicalAddress[i];
                    }
                    info.macAddress = ss.str();
                }
                
                // Interface type
                switch (adapter->IfType) {
                    case IF_TYPE_ETHERNET_CSMACD:
                        info.type = Exs_NetworkInterfaceType::Ethernet;
                        break;
                    case IF_TYPE_IEEE80211:
                        info.type = Exs_NetworkInterfaceType::WiFi;
                        break;
                    case IF_TYPE_SOFTWARE_LOOPBACK:
                        info.type = Exs_NetworkInterfaceType::Loopback;
                        break;
                    case IF_TYPE_TUNNEL:
                        info.type = Exs_NetworkInterfaceType::Tunnel;
                        break;
                    default:
                        info.type = Exs_NetworkInterfaceType::Unknown;
                }
                
                // Status
                switch (adapter->OperStatus) {
                    case IfOperStatusUp:
                        info.status = Exs_NetworkInterfaceStatus::Connected;
                        break;
                    case IfOperStatusDown:
                        info.status = Exs_NetworkInterfaceStatus::Disconnected;
                        break;
                    case IfOperStatusTesting:
                    case IfOperStatusUnknown:
                    case IfOperStatusDormant:
                    case IfOperStatusNotPresent:
                    case IfOperStatusLowerLayerDown:
                        info.status = Exs_NetworkInterfaceStatus::Error;
                        break;
                    default:
                        info.status = Exs_NetworkInterfaceStatus::Unknown;
                }
                
                // Speed and MTU
                info.speed = adapter->TransmitLinkSpeed;
                info.mtu = adapter->Mtu;
                
                // IP addresses
                for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; 
                     unicast != nullptr; unicast = unicast->Next) {
                    
                    Exs_IPAddress ipAddr;
                    
                    if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
                        sockaddr_in* sa_in = (sockaddr_in*)unicast->Address.lpSockaddr;
                        ipAddr.type = Exs_IPAddressType::IPv4;
                        memcpy(ipAddr.address.data(), &sa_in->sin_addr, 4);
                        ipAddr.prefixLength = unicast->OnLinkPrefixLength;
                        
                        // Check if it's loopback
                        ipAddr.isLoopback = (ntohl(sa_in->sin_addr.s_addr) == 0x7F000001);
                        
                        // Check if it's private
                        uint32_t addr = ntohl(sa_in->sin_addr.s_addr);
                        ipAddr.isPrivate = 
                            (addr >= 0x0A000000 && addr <= 0x0AFFFFFF) || // 10.0.0.0/8
                            (addr >= 0xAC100000 && addr <= 0xAC1FFFFF) || // 172.16.0.0/12
                            (addr >= 0xC0A80000 && addr <= 0xC0A8FFFF);   // 192.168.0.0/16
                        
                        info.ipAddresses.push_back(ipAddr);
                    }
                    else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
                        sockaddr_in6* sa_in6 = (sockaddr_in6*)unicast->Address.lpSockaddr;
                        ipAddr.type = Exs_IPAddressType::IPv6;
                        memcpy(ipAddr.address.data(), &sa_in6->sin6_addr, 16);
                        ipAddr.prefixLength = unicast->OnLinkPrefixLength;
                        
                        // Check if it's loopback (::1)
                        uint8_t loopback[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
                        ipAddr.isLoopback = memcmp(ipAddr.address.data(), loopback, 16) == 0;
                        
                        // Check if it's link-local (fe80::/10)
                        ipAddr.isLinkLocal = (ipAddr.address[0] == 0xFE && 
                                             (ipAddr.address[1] & 0xC0) == 0x80);
                        
                        info.ipAddresses.push_back(ipAddr);
                    }
                }
                
                // DNS servers
                for (IP_ADAPTER_DNS_SERVER_ADDRESS* dns = adapter->FirstDnsServerAddress; 
                     dns != nullptr; dns = dns->Next) {
                    
                    Exs_IPAddress dnsAddr;
                    
                    if (dns->Address.lpSockaddr->sa_family == AF_INET) {
                        sockaddr_in* sa_in = (sockaddr_in*)dns->Address.lpSockaddr;
                        dnsAddr.type = Exs_IPAddressType::IPv4;
                        memcpy(dnsAddr.address.data(), &sa_in->sin_addr, 4);
                    }
                    else if (dns->Address.lpSockaddr->sa_family == AF_INET6) {
                        sockaddr_in6* sa_in6 = (sockaddr_in6*)dns->Address.lpSockaddr;
                        dnsAddr.type = Exs_IPAddressType::IPv6;
                        memcpy(dnsAddr.address.data(), &sa_in6->sin6_addr, 16);
                    }
                    
                    info.dnsServers.push_back(dnsAddr);
                }
                
                // Gateways
                for (IP_ADAPTER_GATEWAY_ADDRESS_LH* gateway = adapter->FirstGatewayAddress; 
                     gateway != nullptr; gateway = gateway->Next) {
                    
                    Exs_IPAddress gatewayAddr;
                    
                    if (gateway->Address.lpSockaddr->sa_family == AF_INET) {
                        sockaddr_in* sa_in = (sockaddr_in*)gateway->Address.lpSockaddr;
                        gatewayAddr.type = Exs_IPAddressType::IPv4;
                        memcpy(gatewayAddr.address.data(), &sa_in->sin_addr, 4);
                    }
                    else if (gateway->Address.lpSockaddr->sa_family == AF_INET6) {
                        sockaddr_in6* sa_in6 = (sockaddr_in6*)gateway->Address.lpSockaddr;
                        gatewayAddr.type = Exs_IPAddressType::IPv6;
                        memcpy(gatewayAddr.address.data(), &sa_in6->sin6_addr, 16);
                    }
                    
                    info.gateways.push_back(gatewayAddr);
                }
                
                // DHCP info
                info.isDhcpEnabled = (adapter->Dhcpv4Enabled || adapter->Dhcpv6Enabled);
                if (adapter->Dhcpv4Server.Ipv4.sin_family == AF_INET) {
                    char dhcpStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &adapter->Dhcpv4Server.Ipv4.sin_addr, 
                             dhcpStr, INET_ADDRSTRLEN);
                    info.dhcpServer = dhcpStr;
                }
                
                // Get statistics
                MIB_IF_ROW2 ifRow;
                ifRow.InterfaceIndex = adapter->IfIndex;
                
                if (GetIfEntry2(&ifRow) == NO_ERROR) {
                    info.bytesSent = ifRow.OutOctets;
                    info.bytesReceived = ifRow.InOctets;
                    info.packetsSent = ifRow.OutUcastPkts + ifRow.OutNUcastPkts;
                    info.packetsReceived = ifRow.InUcastPkts + ifRow.InNUcastPkts;
                    info.errorsSent = ifRow.OutErrors;
                    info.errorsReceived = ifRow.InErrors;
                }
                
                interfaces.push_back(info);
            }
        }
        
        free(adapterAddresses);
        return interfaces;
    }
    
    Exs_NetworkInterfaceInfo getNetworkInterface(const std::string& name) const override {
        auto interfaces = getNetworkInterfaces();
        for (const auto& interface : interfaces) {
            if (interface.name == name) {
                return interface;
            }
        }
        return Exs_NetworkInterfaceInfo();
    }
    
    std::vector<Exs_NetworkConnectionInfo> getNetworkConnections() const override {
        std::vector<Exs_NetworkConnectionInfo> connections;
        
        // Get TCP connections
        PMIB_TCPTABLE_OWNER_PID tcpTable = nullptr;
        ULONG tcpSize = 0;
        
        GetExtendedTcpTable(nullptr, &tcpSize, FALSE, AF_INET, 
                           TCP_TABLE_OWNER_PID_ALL, 0);
        
        if (tcpSize > 0) {
            tcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(tcpSize);
            if (tcpTable && GetExtendedTcpTable(tcpTable, &tcpSize, FALSE, AF_INET,
                                               TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
                
                for (DWORD i = 0; i < tcpTable->dwNumEntries; i++) {
                    Exs_NetworkConnectionInfo conn;
                    
                    conn.protocol = "TCP";
                    
                    // Local address
                    char localAddr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &tcpTable->table[i].dwLocalAddr, 
                             localAddr, INET_ADDRSTRLEN);
                    conn.localAddress = localAddr;
                    conn.localPort = ntohs((u_short)tcpTable->table[i].dwLocalPort);
                    
                    // Remote address
                    char remoteAddr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &tcpTable->table[i].dwRemoteAddr, 
                             remoteAddr, INET_ADDRSTRLEN);
                    conn.remoteAddress = remoteAddr;
                    conn.remotePort = ntohs((u_short)tcpTable->table[i].dwRemotePort);
                    
                    // State
                    switch (tcpTable->table[i].dwState) {
                        case MIB_TCP_STATE_CLOSED: conn.state = "CLOSED"; break;
                        case MIB_TCP_STATE_LISTEN: conn.state = "LISTEN"; break;
                        case MIB_TCP_STATE_SYN_SENT: conn.state = "SYN_SENT"; break;
                        case MIB_TCP_STATE_SYN_RCVD: conn.state = "SYN_RCVD"; break;
                        case MIB_TCP_STATE_ESTAB: conn.state = "ESTABLISHED"; break;
                        case MIB_TCP_STATE_FIN_WAIT1: conn.state = "FIN_WAIT1"; break;
                        case MIB_TCP_STATE_FIN_WAIT2: conn.state = "FIN_WAIT2"; break;
                        case MIB_TCP_STATE_CLOSE_WAIT: conn.state = "CLOSE_WAIT"; break;
                        case MIB_TCP_STATE_CLOSING: conn.state = "CLOSING"; break;
                        case MIB_TCP_STATE_LAST_ACK: conn.state = "LAST_ACK"; break;
                        case MIB_TCP_STATE_TIME_WAIT: conn.state = "TIME_WAIT"; break;
                        case MIB_TCP_STATE_DELETE_TCB: conn.state = "DELETE"; break;
                        default: conn.state = "UNKNOWN";
                    }
                    
                    conn.processId = tcpTable->table[i].dwOwningPid;
                    
                    // Get process name
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                                                 FALSE, conn.processId);
                    if (hProcess) {
                        wchar_t processName[MAX_PATH];
                        DWORD size = MAX_PATH;
                        if (QueryFullProcessImageNameW(hProcess, 0, processName, &size)) {
                            std::wstring wname(processName);
                            conn.processName = std::string(wname.begin(), wname.end());
                        }
                        CloseHandle(hProcess);
                    }
                    
                    connections.push_back(conn);
                }
            }
            if (tcpTable) free(tcpTable);
        }
        
        // Get UDP connections
        PMIB_UDPTABLE_OWNER_PID udpTable = nullptr;
        ULONG udpSize = 0;
        
        GetExtendedUdpTable(nullptr, &udpSize, FALSE, AF_INET, 
                           UDP_TABLE_OWNER_PID, 0);
        
        if (udpSize > 0) {
            udpTable = (PMIB_UDPTABLE_OWNER_PID)malloc(udpSize);
            if (udpTable && GetExtendedUdpTable(udpTable, &udpSize, FALSE, AF_INET,
                                               UDP_TABLE_OWNER_PID, 0) == NO_ERROR) {
                
                for (DWORD i = 0; i < udpTable->dwNumEntries; i++) {
                    Exs_NetworkConnectionInfo conn;
                    
                    conn.protocol = "UDP";
                    
                    // Local address
                    char localAddr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &udpTable->table[i].dwLocalAddr, 
                             localAddr, INET_ADDRSTRLEN);
                    conn.localAddress = localAddr;
                    conn.localPort = ntohs((u_short)udpTable->table[i].dwLocalPort);
                    
                    conn.remoteAddress = "*";
                    conn.remotePort = 0;
                    conn.state = "LISTEN";
                    conn.processId = udpTable->table[i].dwOwningPid;
                    
                    // Get process name
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                                                 FALSE, conn.processId);
                    if (hProcess) {
                        wchar_t processName[MAX_PATH];
                        DWORD size = MAX_PATH;
                        if (QueryFullProcessImageNameW(hProcess, 0, processName, &size)) {
                            std::wstring wname(processName);
                            conn.processName = std::string(wname.begin(), wname.end());
                        }
                        CloseHandle(hProcess);
                    }
                    
                    connections.push_back(conn);
                }
            }
            if (udpTable) free(udpTable);
        }
        
        return connections;
    }
    
    std::vector<Exs_NetworkConnectionInfo> getNetworkConnectionsByProcess(uint32 pid) const override {
        auto connections = getNetworkConnections();
        std::vector<Exs_NetworkConnectionInfo> result;
        
        for (const auto& conn : connections) {
            if (conn.processId == pid) {
                result.push_back(conn);
            }
        }
        
        return result;
    }
    
    Exs_NetworkStatistics getNetworkStatistics() const override {
        Exs_NetworkStatistics stats = {};
        
        auto interfaces = getNetworkInterfaces();
        for (const auto& interface : interfaces) {
            stats.totalBytesSent += interface.bytesSent;
            stats.totalBytesReceived += interface.bytesReceived;
            stats.totalPacketsSent += interface.packetsSent;
            stats.totalPacketsReceived += interface.packetsReceived;
            stats.totalErrorsSent += interface.errorsSent;
            stats.totalErrorsReceived += interface.errorsReceived;
        }
        
        // Get TCP/UDP statistics
        MIB_TCPSTATS tcpStats;
        if (GetTcpStatistics(&tcpStats) == NO_ERROR) {
            stats.tcpConnections = tcpStats.dwNumConns;
        }
        
        MIB_UDPSTATS udpStats;
        if (GetUdpStatistics(&udpStats) == NO_ERROR) {
            stats.udpConnections = udpStats.dwNumAddrs;
        }
        
        // Get active connections
        auto connections = getNetworkConnections();
        for (const auto& conn : connections) {
            if (conn.state == "ESTABLISHED") {
                stats.activeConnections++;
            }
        }
        
        return stats;
    }
    
    Exs_NetworkStatistics getNetworkStatisticsForInterface(const std::string& interfaceName) const override {
        auto interface = getNetworkInterface(interfaceName);
        Exs_NetworkStatistics stats = {};
        
        if (!interface.name.empty()) {
            stats.totalBytesSent = interface.bytesSent;
            stats.totalBytesReceived = interface.bytesReceived;
            stats.totalPacketsSent = interface.packetsSent;
            stats.totalPacketsReceived = interface.packetsReceived;
            stats.totalErrorsSent = interface.errorsSent;
            stats.totalErrorsReceived = interface.errorsReceived;
        }
        
        return stats;
    }
    
    std::vector<Exs_IPAddress> getLocalIPAddresses() const override {
        std::vector<Exs_IPAddress> addresses;
        auto interfaces = getNetworkInterfaces();
        
        for (const auto& interface : interfaces) {
            if (interface.status == Exs_NetworkInterfaceStatus::Connected) {
                for (const auto& addr : interface.ipAddresses) {
                    if (!addr.isLoopback) {
                        addresses.push_back(addr);
                    }
                }
            }
        }
        
        return addresses;
    }
    
    Exs_IPAddress getPrimaryIPAddress() const override {
        auto addresses = getLocalIPAddresses();
        if (!addresses.empty()) {
            // Prefer IPv4 addresses
            for (const auto& addr : addresses) {
                if (addr.type == Exs_IPAddressType::IPv4 && !addr.isLinkLocal) {
                    return addr;
                }
            }
            return addresses[0];
        }
        return Exs_IPAddress();
    }
    
    std::string getHostName() const override {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return hostname;
        }
        return "";
    }
    
    std::string getDomainName() const override {
        FIXED_INFO* fixedInfo = nullptr;
        ULONG bufferSize = 0;
        
        if (GetNetworkParams(nullptr, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
            fixedInfo = (FIXED_INFO*)malloc(bufferSize);
            if (fixedInfo && GetNetworkParams(fixedInfo, &bufferSize) == NO_ERROR) {
                std::string domain = fixedInfo->DomainName;
                free(fixedInfo);
                return domain;
            }
            if (fixedInfo) free(fixedInfo);
        }
        return "";
    }
    
    std::vector<Exs_IPAddress> getDNSServers() const override {
        std::vector<Exs_IPAddress> dnsServers;
        
        FIXED_INFO* fixedInfo = nullptr;
        ULONG bufferSize = 0;
        
        if (GetNetworkParams(nullptr, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
            fixedInfo = (FIXED_INFO*)malloc(bufferSize);
            if (fixedInfo && GetNetworkParams(fixedInfo, &bufferSize) == NO_ERROR) {
                
                IP_ADDR_STRING* ipAddr = &fixedInfo->DnsServerList;
                while (ipAddr) {
                    Exs_IPAddress dnsAddr;
                    dnsAddr.type = Exs_IPAddressType::IPv4;
                    
                    // Parse IP address
                    struct in_addr addr;
                    inet_pton(AF_INET, ipAddr->IpAddress.String, &addr);
                    memcpy(dnsAddr.address.data(), &addr, 4);
                    
                    dnsServers.push_back(dnsAddr);
                    ipAddr = ipAddr->Next;
                }
            }
            if (fixedInfo) free(fixedInfo);
        }
        
        return dnsServers;
    }
    
    std::vector<std::string> resolveHostName(const std::string& hostname) const override {
        std::vector<std::string> addresses;
        
        ADDRINFOA hints = {0};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        
        ADDRINFOA* result = nullptr;
        if (getaddrinfo(hostname.c_str(), nullptr, &hints, &result) == 0) {
            for (ADDRINFOA* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
                char ipStr[INET6_ADDRSTRLEN];
                
                if (ptr->ai_family == AF_INET) {
                    sockaddr_in* ipv4 = (sockaddr_in*)ptr->ai_addr;
                    inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, sizeof(ipStr));
                }
                else if (ptr->ai_family == AF_INET6) {
                    sockaddr_in6* ipv6 = (sockaddr_in6*)ptr->ai_addr;
                    inet_ntop(AF_INET6, &ipv6->sin6_addr, ipStr, sizeof(ipStr));
                }
                
                addresses.push_back(ipStr);
            }
            freeaddrinfo(result);
        }
        
        return addresses;
    }
    
    std::string reverseLookup(const Exs_IPAddress& ip) const override {
        sockaddr_storage addr = {0};
        char hostname[NI_MAXHOST];
        
        if (ip.type == Exs_IPAddressType::IPv4) {
            sockaddr_in* ipv4 = (sockaddr_in*)&addr;
            ipv4->sin_family = AF_INET;
            memcpy(&ipv4->sin_addr, ip.address.data(), 4);
        }
        else if (ip.type == Exs_IPAddressType::IPv6) {
            sockaddr_in6* ipv6 = (sockaddr_in6*)&addr;
            ipv6->sin6_family = AF_INET6;
            memcpy(&ipv6->sin6_addr, ip.address.data(), 16);
        }
        
        if (getnameinfo((sockaddr*)&addr, 
                       (ip.type == Exs_IPAddressType::IPv4) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
                       hostname, NI_MAXHOST, nullptr, 0, 0) == 0) {
            return hostname;
        }
        
        return "";
    }
    
    bool isNetworkAvailable() const override {
        DWORD flags;
        return InternetGetConnectedState(&flags, 0) != 0;
    }
    
    bool isInternetAvailable() const override {
        return InternetCheckConnectionA("http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0) != 0;
    }
    
    Exs_NetworkQuality getNetworkQuality() const override {
        // Simple quality check based on latency and packet loss
        double latency = getNetworkLatency("8.8.8.8");
        
        if (latency == 0) {
            return Exs_NetworkQuality::Unknown;
        }
        else if (latency < 50) {
            return Exs_NetworkQuality::Excellent;
        }
        else if (latency < 100) {
            return Exs_NetworkQuality::Good;
        }
        else if (latency < 200) {
            return Exs_NetworkQuality::Fair;
        }
        else if (latency < 500) {
            return Exs_NetworkQuality::Poor;
        }
        else {
            return Exs_NetworkQuality::Bad;
        }
    }
    
    bool isFirewallEnabled() const override {
        INetFwProfile* fwProfile = nullptr;
        HRESULT hr = CoCreateInstance(__uuidof(NetFwMgr), nullptr, CLSCTX_INPROC_SERVER, 
                                     __uuidof(INetFwMgr), (void**)&fwProfile);
        
        if (SUCCEEDED(hr) && fwProfile) {
            VARIANT_BOOL fwEnabled;
            if (SUCCEEDED(fwProfile->get_FirewallEnabled(&fwEnabled))) {
                fwProfile->Release();
                return fwEnabled == VARIANT_TRUE;
            }
            fwProfile->Release();
        }
        
        return false;
    }
    
    bool isProxyEnabled() const override {
        WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxyConfig;
        if (WinHttpGetIEProxyConfigForCurrentUser(&proxyConfig)) {
            bool enabled = proxyConfig.lpszProxy != nullptr;
            
            if (proxyConfig.lpszProxy) GlobalFree(proxyConfig.lpszProxy);
            if (proxyConfig.lpszProxyBypass) GlobalFree(proxyConfig.lpszProxyBypass);
            if (proxyConfig.lpszAutoConfigUrl) GlobalFree(proxyConfig.lpszAutoConfigUrl);
            
            return enabled;
        }
        
        return false;
    }
    
    std::string getProxyServer() const override {
        WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxyConfig;
        if (WinHttpGetIEProxyConfigForCurrentUser(&proxyConfig)) {
            if (proxyConfig.lpszProxy) {
                std::wstring wproxy(proxyConfig.lpszProxy);
                std::string proxy(wproxy.begin(), wproxy.end());
                
                GlobalFree(proxyConfig.lpszProxy);
                if (proxyConfig.lpszProxyBypass) GlobalFree(proxyConfig.lpszProxyBypass);
                if (proxyConfig.lpszAutoConfigUrl) GlobalFree(proxyConfig.lpszAutoConfigUrl);
                
                return proxy;
            }
            
            if (proxyConfig.lpszProxy) GlobalFree(proxyConfig.lpszProxy);
            if (proxyConfig.lpszProxyBypass) GlobalFree(proxyConfig.lpszProxyBypass);
            if (proxyConfig.lpszAutoConfigUrl) GlobalFree(proxyConfig.lpszAutoConfigUrl);
        }
        
        return "";
    }
    
    bool supportsIPv4() const override {
        // All Windows versions support IPv4
        return true;
    }
    
    bool supportsIPv6() const override {
        // Check if IPv6 is installed and enabled
        ULONG bufferSize = 0;
        GetAdaptersAddresses(AF_INET6, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &bufferSize);
        return bufferSize > 0;
    }
    
    bool supportsJumboFrames() const override {
        // Would need to check NIC capabilities
        return false;
    }
    
    bool supportsWakeOnLAN() const override {
        // Would need to check NIC capabilities via WMI
        return false;
    }
    
    double getCurrentBandwidthUsage() const override {
        // Calculate bandwidth usage from interface statistics
        auto interfaces = getNetworkInterfaces();
        double totalSpeed = 0;
        double usedSpeed = 0;
        
        for (const auto& interface : interfaces) {
            if (interface.status == Exs_NetworkInterfaceStatus::Connected) {
                totalSpeed += interface.speed;
                
                // Estimate current usage (bytes per second)
                // This would need actual monitoring over time
            }
        }
        
        if (totalSpeed > 0) {
            return (usedSpeed / totalSpeed) * 100.0;
        }
        
        return 0.0;
    }
    
    double getMaxBandwidth() const override {
        auto interfaces = getNetworkInterfaces();
        double maxSpeed = 0;
        
        for (const auto& interface : interfaces) {
            if (interface.status == Exs_NetworkInterfaceStatus::Connected) {
                maxSpeed = std::max(maxSpeed, (double)interface.speed);
            }
        }
        
        return maxSpeed;
    }
    
    double getNetworkLatency(const std::string& host) const override {
        HANDLE hIcmp = IcmpCreateFile();
        if (hIcmp == INVALID_HANDLE_VALUE) {
            return 0.0;
        }
        
        // Resolve host to IP
        std::vector<std::string> addresses = resolveHostName(host);
        if (addresses.empty()) {
            IcmpCloseHandle(hIcmp);
            return 0.0;
        }
        
        // Convert IP string to in_addr
        struct in_addr targetAddr;
        inet_pton(AF_INET, addresses[0].c_str(), &targetAddr);
        
        // Send ping
        char sendData[32] = "Exs Network Latency Test";
        char replyBuffer[sizeof(ICMP_ECHO_REPLY) + 32];
        
        DWORD reply = IcmpSendEcho(hIcmp, targetAddr.s_addr, sendData, sizeof(sendData),
                                  nullptr, replyBuffer, sizeof(replyBuffer), 1000);
        
        double latency = 0.0;
        if (reply > 0) {
            PICMP_ECHO_REPLY echoReply = (PICMP_ECHO_REPLY)replyBuffer;
            latency = echoReply->RoundTripTime;
        }
        
        IcmpCloseHandle(hIcmp);
        return latency;
    }
    
    double getPacketLoss(const std::string& host) const override {
        // Send multiple pings to calculate packet loss
        const int NUM_PINGS = 10;
        int successfulPings = 0;
        
        for (int i = 0; i < NUM_PINGS; i++) {
            if (getNetworkLatency(host) > 0) {
                successfulPings++;
            }
            Sleep(100); // Small delay between pings
        }
        
        return ((NUM_PINGS - successfulPings) / (double)NUM_PINGS) * 100.0;
    }
    
    std::vector<std::string> getRoutingTable() const override {
        std::vector<std::string> routes;
        
        PMIB_IPFORWARDTABLE pIpForwardTable = nullptr;
        DWORD dwSize = 0;
        
        // Get required buffer size
        GetIpForwardTable(pIpForwardTable, &dwSize, FALSE);
        
        pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
        if (pIpForwardTable && GetIpForwardTable(pIpForwardTable, &dwSize, FALSE) == NO_ERROR) {
            
            for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++) {
                std::stringstream ss;
                
                // Destination
                char dest[INET_ADDRSTRLEN];
                in_addr destAddr;
                destAddr.s_addr = pIpForwardTable->table[i].dwForwardDest;
                inet_ntop(AF_INET, &destAddr, dest, INET_ADDRSTRLEN);
                
                // Mask
                char mask[INET_ADDRSTRLEN];
                in_addr maskAddr;
                maskAddr.s_addr = pIpForwardTable->table[i].dwForwardMask;
                inet_ntop(AF_INET, &maskAddr, mask, INET_ADDRSTRLEN);
                
                // Gateway
                char gateway[INET_ADDRSTRLEN];
                in_addr gatewayAddr;
                gatewayAddr.s_addr = pIpForwardTable->table[i].dwForwardNextHop;
                inet_ntop(AF_INET, &gatewayAddr, gateway, INET_ADDRSTRLEN);
                
                ss << dest << "/" << mask << " via " << gateway 
                   << " metric " << pIpForwardTable->table[i].dwForwardMetric1;
                
                routes.push_back(ss.str());
            }
        }
        
        if (pIpForwardTable) free(pIpForwardTable);
        return routes;
    }
    
    std::string getDefaultGateway() const override {
        auto routes = getRoutingTable();
        for (const auto& route : routes) {
            if (route.find("0.0.0.0/0.0.0.0") != std::string::npos) {
                return route;
            }
        }
        return "";
    }
    
    std::vector<std::pair<std::string, std::string>> getARPTable() const override {
        std::vector<std::pair<std::string, std::string>> arpTable;
        
        PMIB_IPNETTABLE pIpNetTable = nullptr;
        DWORD dwSize = 0;
        
        GetIpNetTable(pIpNetTable, &dwSize, FALSE);
        
        pIpNetTable = (PMIB_IPNETTABLE)malloc(dwSize);
        if (pIpNetTable && GetIpNetTable(pIpNetTable, &dwSize, FALSE) == NO_ERROR) {
            
            for (DWORD i = 0; i < pIpNetTable->dwNumEntries; i++) {
                // IP address
                char ipStr[INET_ADDRSTRLEN];
                in_addr ipAddr;
                ipAddr.s_addr = pIpNetTable->table[i].dwAddr;
                inet_ntop(AF_INET, &ipAddr, ipStr, INET_ADDRSTRLEN);
                
                // MAC address
                std::stringstream mac;
                for (int j = 0; j < (int)pIpNetTable->table[i].dwPhysAddrLen; j++) {
                    if (j > 0) mac << ":";
                    mac << std::hex << std::setw(2) << std::setfill('0') 
                        << (int)pIpNetTable->table[i].bPhysAddr[j];
                }
                
                arpTable.push_back({ipStr, mac.str()});
            }
        }
        
        if (pIpNetTable) free(pIpNetTable);
        return arpTable;
    }
    
    std::vector<std::string> getNetworkServices() const override {
        std::vector<std::string> services;
        
        SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
        if (scm) {
            DWORD bytesNeeded = 0;
            DWORD serviceCount = 0;
            DWORD resumeHandle = 0;
            
            EnumServicesStatusEx(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, 
                                SERVICE_STATE_ALL, nullptr, 0, &bytesNeeded,
                                &serviceCount, &resumeHandle, nullptr);
            
            if (bytesNeeded > 0) {
                LPBYTE buffer = (LPBYTE)malloc(bytesNeeded);
                if (buffer) {
                    if (EnumServicesStatusEx(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
                                            SERVICE_STATE_ALL, buffer, bytesNeeded,
                                            &bytesNeeded, &serviceCount, &resumeHandle, nullptr)) {
                        
                        ENUM_SERVICE_STATUS_PROCESS* servicesArray = 
                            (ENUM_SERVICE_STATUS_PROCESS*)buffer;
                        
                        for (DWORD i = 0; i < serviceCount; i++) {
                            services.push_back(servicesArray[i].lpServiceName);
                        }
                    }
                    free(buffer);
                }
            }
            CloseServiceHandle(scm);
        }
        
        return services;
    }
    
    bool isNetworkServiceRunning(const std::string& service) const override {
        SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (!scm) return false;
        
        SC_HANDLE svc = OpenService(scm, service.c_str(), SERVICE_QUERY_STATUS);
        if (!svc) {
            CloseServiceHandle(scm);
            return false;
        }
        
        SERVICE_STATUS_PROCESS status;
        DWORD bytesNeeded;
        bool isRunning = false;
        
        if (QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&status,
                                sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
            isRunning = (status.dwCurrentState == SERVICE_RUNNING);
        }
        
        CloseServiceHandle(svc);
        CloseServiceHandle(scm);
        
        return isRunning;
    }
    
    bool isVPNConnected() const override {
        auto interfaces = getNetworkInterfaces();
        for (const auto& interface : interfaces) {
            if (interface.type == Exs_NetworkInterfaceType::Tunnel && 
                interface.status == Exs_NetworkInterfaceStatus::Connected) {
                return true;
            }
        }
        return false;
    }
    
    std::string getVPNServer() const override {
        // VPN server info would be in registry or via RAS API
        return "";
    }
    
    bool isPortOpen(uint16 port) const override {
        auto connections = getNetworkConnections();
        for (const auto& conn : connections) {
            if (conn.localPort == port && 
               (conn.state == "LISTEN" || conn.state == "ESTABLISHED")) {
                return true;
            }
        }
        return false;
    }
    
    void startNetworkMonitoring() override {
        if (monitoring) return;
        
        monitoring = true;
        // Start monitoring thread
        // Implementation would use various Windows APIs to monitor network changes
    }
    
    void stopNetworkMonitoring() override {
        monitoring = false;
        if (monitoringThread) {
            WaitForSingleObject(monitoringThread, INFINITE);
            CloseHandle(monitoringThread);
            monitoringThread = nullptr;
        }
    }
    
    bool isNetworkMonitoring() const override {
        return monitoring;
    }
};

// Factory function implementation
Exs_NetworkInfoBase* Exs_CreateNetworkInfoInstance() {
    return new Exs_NetworkInfoWindows();
}

} // namespace NetworkInfo
} // namespace Internal
} // namespace Exs
