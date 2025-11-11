#pragma once
#include "../common/PlatformDefs.h"
#include <string>
#include <vector>

#ifdef PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <iphlpapi.h>
    #include "WmiManager.h"
#elif defined(PLATFORM_MACOS)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <sys/sysctl.h>
    #include <net/route.h>
    #include <sys/types.h>
    #include <unistd.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <fstream>
    #include <sstream>
#endif

class NetworkAdapter {
public:
    struct AdapterInfo {
        std::string name;
        std::string mac;
        std::string ip;
        std::string description;
        std::string adapterType;
        bool isEnabled;
        bool isConnected;
        uint64_t speed;
        std::string speedString;
        
        // macOS特有字段
        std::vector<std::string> ipAddresses;  // 支持多个IP地址
        std::string gateway;
        std::string subnetMask;
        std::string dnsServers;
    };

#ifdef PLATFORM_WINDOWS
    explicit NetworkAdapter(WmiManager& manager);
#else
    NetworkAdapter();
#endif
    ~NetworkAdapter();

    const std::vector<AdapterInfo>& GetAdapters() const;
    void Refresh();

private:
    void Initialize();
    void Cleanup();
    void QueryAdapterInfo();
    
#ifdef PLATFORM_WINDOWS
    void QueryWmiAdapterInfo();
    void UpdateAdapterAddresses();
    std::wstring FormatMacAddress(const unsigned char* address, size_t length) const;
    std::wstring FormatSpeed(uint64_t bitsPerSecond) const;
    bool IsVirtualAdapter(const std::wstring& name) const;
    void SafeRelease(IUnknown* pInterface);
    std::wstring DetermineAdapterType(const std::wstring& name, const std::wstring& description, DWORD ifType) const;
    WmiManager& wmiManager;
#elif defined(PLATFORM_MACOS)
    void QueryMacNetworkAdapters();
    void UpdateMacAdapterAddresses();
    std::string FormatMacAddress(const unsigned char* address, size_t length) const;
    std::string FormatSpeed(uint64_t bitsPerSecond) const;
    bool IsVirtualAdapter(const std::string& name) const;
    std::string DetermineAdapterType(const std::string& name, const std::string& description, unsigned int ifType) const;
    void GetInterfaceMTU(const std::string& interfaceName, AdapterInfo& info) const;
    void GetInterfaceStats(const std::string& interfaceName, AdapterInfo& info) const;
#elif defined(PLATFORM_LINUX)
    void QueryLinuxNetworkAdapters();
    void UpdateLinuxAdapterAddresses();
    std::string FormatMacAddress(const unsigned char* address, size_t length) const;
    std::string FormatSpeed(uint64_t bitsPerSecond) const;
    bool IsVirtualAdapter(const std::string& name) const;
    std::string DetermineAdapterType(const std::string& name, const std::string& description, const std::string& driver) const;
#endif

    std::vector<AdapterInfo> adapters;
    bool initialized;
};
