#include "NetworkAdapter.h"
#include "../Utils/Logger.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#ifdef PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <iphlpapi.h>
    #include <comutil.h>
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "ws2_32.lib")
#elif defined(PLATFORM_MACOS)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <net/if_media.h>
    #include <sys/sysctl.h>
    #include <net/route.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <ifaddrs.h>
    #include <sys/sockio.h>
    #include <sys/ioctl.h>
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

#ifdef PLATFORM_WINDOWS
NetworkAdapter::NetworkAdapter(WmiManager& manager)
    : wmiManager(manager), initialized(false) {
    Logger::Debug("NetworkAdapter: Initializing Windows network adapter");
    Initialize();
}
#else
NetworkAdapter::NetworkAdapter()
    : initialized(false) {
    Logger::Debug("NetworkAdapter: Initializing network adapter");
    Initialize();
}
#endif

NetworkAdapter::~NetworkAdapter() {
    Cleanup();
}

void NetworkAdapter::Initialize() {
    Logger::Debug("NetworkAdapter: Starting initialization");
    try {
#ifdef PLATFORM_WINDOWS
        if (wmiManager.IsInitialized()) {
            Logger::Debug("NetworkAdapter: WMI initialized, querying adapter information");
            QueryAdapterInfo();
            initialized = true;
            Logger::Debug("NetworkAdapter: Windows initialization completed");
        } else {
            Logger::Error("WMI not initialized, cannot get network information");
        }
#elif defined(PLATFORM_MACOS)
        QueryMacNetworkAdapters();
        UpdateMacAdapterAddresses();
        initialized = true;
        Logger::Debug("NetworkAdapter: macOS initialization completed");
#elif defined(PLATFORM_LINUX)
        QueryLinuxNetworkAdapters();
        UpdateLinuxAdapterAddresses();
        initialized = true;
        Logger::Debug("NetworkAdapter: Linux initialization completed");
#endif
    }
    catch (const std::exception& e) {
        Logger::Error("NetworkAdapter initialization failed: " + std::string(e.what()));
        initialized = false;
    }
}

void NetworkAdapter::Cleanup() {
    Logger::Debug("NetworkAdapter: Cleaning network adapter data");
    adapters.clear();
    initialized = false;
}

void NetworkAdapter::Refresh() {
    Logger::Debug("NetworkAdapter: Refreshing network adapter information");
    Cleanup();
    Initialize();
}

void NetworkAdapter::QueryAdapterInfo() {
    Logger::Debug("NetworkAdapter: Querying adapter information");
    adapters.clear();
    
#ifdef PLATFORM_WINDOWS
    QueryWmiAdapterInfo();
    UpdateAdapterAddresses();
#elif defined(PLATFORM_MACOS)
    QueryMacNetworkAdapters();
    UpdateMacAdapterAddresses();
#elif defined(PLATFORM_LINUX)
    QueryLinuxNetworkAdapters();
    UpdateLinuxAdapterAddresses();
#endif
    
    Logger::Debug("NetworkAdapter: Adapter information query completed");
}

// macOS网络适配器查询实现
#ifdef PLATFORM_MACOS
void NetworkAdapter::QueryMacNetworkAdapters() {
    Logger::Debug("NetworkAdapter: Querying macOS network adapters");
    
    struct ifaddrs *ifap, *ifa;
    if (getifaddrs(&ifap) != 0) {
        Logger::Error("Failed to get network interface addresses");
        return;
    }
    
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_LINK) {
            continue;
        }
        
        AdapterInfo info;
        info.name = ifa->ifa_name;
        info.isEnabled = (ifa->ifa_flags & (IFF_UP | IFF_RUNNING)) != 0;
        info.isConnected = info.isEnabled;
        
        // 获取MAC地址
        struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
        if (sdl && sdl->sdl_alen == 6) {
            unsigned char* mac = (unsigned char*)LLADDR(sdl);
            info.mac = FormatMacAddress(mac, 6);
        }
        
        // 过滤虚拟适配器
        if (IsVirtualAdapter(info.name)) {
            Logger::Debug("NetworkAdapter: Skipping virtual adapter: " + info.name);
            continue;
        }
        
        // 获取接口类型和描述
        info.adapterType = DetermineAdapterType(info.name, "", 0);
        
        // 获取接口速度和统计信息
        GetInterfaceStats(info.name, info);
        
        adapters.push_back(info);
        Logger::Debug("NetworkAdapter: Added adapter - Name: " + info.name + 
                     ", MAC: " + info.mac + ", Type: " + info.adapterType);
    }
    
    freeifaddrs(ifap);
    Logger::Debug("NetworkAdapter: macOS adapter query completed");
}

void NetworkAdapter::UpdateMacAdapterAddresses() {
    Logger::Debug("NetworkAdapter: Updating macOS adapter addresses");
    
    struct ifaddrs *ifap, *ifa;
    if (getifaddrs(&ifap) != 0) {
        Logger::Error("Failed to get network interface addresses for update");
        return;
    }
    
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        
        // 查找对应的适配器
        for (auto& adapter : adapters) {
            if (adapter.name == ifa->ifa_name) {
                // IPv4地址
                if (ifa->ifa_addr->sa_family == AF_INET) {
                    struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, INET_ADDRSTRLEN);
                    
                    // 添加到IP地址列表
                    if (std::find(adapter.ipAddresses.begin(), adapter.ipAddresses.end(), ip_str) == adapter.ipAddresses.end()) {
                        adapter.ipAddresses.push_back(ip_str);
                    }
                    
                    // 设置主IP地址（第一个）
                    if (adapter.ip.empty()) {
                        adapter.ip = ip_str;
                    }
                }
                // IPv6地址
                else if (ifa->ifa_addr->sa_family == AF_INET6) {
                    struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)ifa->ifa_addr;
                    char ip_str[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str, INET6_ADDRSTRLEN);
                    
                    adapter.ipAddresses.push_back(std::string(ip_str));
                }
            }
        }
    }
    
    freeifaddrs(ifap);
    Logger::Debug("NetworkAdapter: macOS adapter address update completed");
}

std::string NetworkAdapter::FormatMacAddress(const unsigned char* address, size_t length) const {
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0) ss << ":";
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(address[i]);
    }
    return ss.str();
}

std::string NetworkAdapter::FormatSpeed(uint64_t bitsPerSecond) const {
    const double GB = 1000000000.0;
    const double MB = 1000000.0;
    const double KB = 1000.0;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);

    if (bitsPerSecond >= GB) {
        ss << (bitsPerSecond / GB) << " Gbps";
    }
    else if (bitsPerSecond >= MB) {
        ss << (bitsPerSecond / MB) << " Mbps";
    }
    else if (bitsPerSecond >= KB) {
        ss << (bitsPerSecond / KB) << " Kbps";
    }
    else {
        ss << bitsPerSecond << " bps";
    }

    return ss.str();
}

bool NetworkAdapter::IsVirtualAdapter(const std::string& name) const {
    const std::vector<std::string> virtualKeywords = {
        "lo", "loopback", "vmnet", "veth", "docker", "br-",
        "utun", "awdl", "p2p", "llw", "anpi"
    };

    for (const auto& keyword : virtualKeywords) {
        if (name.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string NetworkAdapter::DetermineAdapterType(const std::string& name, const std::string& description, unsigned int ifType) const {
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    // 无线适配器关键词
    const std::vector<std::string> wirelessKeywords = {
        "wi-fi", "wifi", "wireless", "wlan", "airport", "en0", "en1"
    };

    // 有线适配器关键词
    const std::vector<std::string> ethernetKeywords = {
        "ethernet", "gigabit", "en", "eth"
    };

    // 检查无线关键词
    for (const auto& keyword : wirelessKeywords) {
        if (lowerName.find(keyword) != std::string::npos) {
            return "Wireless";
        }
    }

    // 检查有线关键词
    for (const auto& keyword : ethernetKeywords) {
        if (lowerName.find(keyword) != std::string::npos) {
            return "Wired";
        }
    }

    return "Unknown Type";
}

void NetworkAdapter::GetInterfaceStats(const std::string& interfaceName, AdapterInfo& info) const {
    int mib[] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0};
    size_t len = 0;
    
    // 获取所需缓冲区大小
    if (sysctl(mib, 6, nullptr, &len, nullptr, 0) < 0) {
        Logger::Error("Failed to get interface stats buffer size");
        return;
    }
    
    std::vector<char> buffer(len);
    if (sysctl(mib, 6, buffer.data(), &len, nullptr, 0) < 0) {
        Logger::Error("Failed to get interface stats");
        return;
    }
    
    char *lim = buffer.data() + len;
    char *next = buffer.data();
    
    while (next < lim) {
        struct if_msghdr *ifm = (struct if_msghdr*)next;
        next += ifm->ifm_msglen;
        
        if (ifm->ifm_type == RTM_IFINFO2) {
            struct if_msghdr2 *ifm2 = (struct if_msghdr2*)ifm;
            char ifname[IF_NAMESIZE];
            if_indextoname(ifm2->ifm_index, ifname);
            
            if (std::string(ifname) == interfaceName) {
                // 设置速度（如果可用）
                if (ifm2->ifm_data.ifi_baudrate > 0) {
                    info.speed = ifm2->ifm_data.ifi_baudrate;
                    info.speedString = FormatSpeed(info.speed);
                }
                break;
            }
        }
    }
}
#endif

// Windows实现
bool NetworkAdapter::IsVirtualAdapter(const std::wstring& name) const {
    const std::vector<std::wstring> virtualKeywords = {
        L"VirtualBox",
        L"Hyper-V",
        L"Virtual",
        L"VPN",
        L"Bluetooth",
        L"VMware",
        L"Loopback",
        L"Microsoft Wi-Fi Direct"
    };

    for (const auto& keyword : virtualKeywords) {
        if (name.find(keyword) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

void NetworkAdapter::QueryWmiAdapterInfo() {
    Logger::Debug("NetworkAdapter: Starting WMI adapter query");
    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hres = wmiManager.GetWmiService()->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE PhysicalAdapter = True"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hres)) {
        Logger::Error("Network adapter WMI query failed: HRESULT=0x" + std::to_string(hres));
        return;
    }

    ULONG uReturn = 0;
    IWbemClassObject* pclsObj = nullptr;
    int adapterCount = 0;
    while (pEnumerator && pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        AdapterInfo info;
        info.isEnabled = false;

        // Get adapter name
        VARIANT vtName, vtDesc, vtStatus;
        VariantInit(&vtName);
        VariantInit(&vtDesc);
        VariantInit(&vtStatus);

        if (SUCCEEDED(pclsObj->Get(L"Name", 0, &vtName, 0, 0)) && vtName.vt == VT_BSTR) {
            info.name = vtName.bstrVal;
        }

        if (SUCCEEDED(pclsObj->Get(L"Description", 0, &vtDesc, 0, 0)) && vtDesc.vt == VT_BSTR) {
            info.description = vtDesc.bstrVal;
        }

        // Check adapter status
        if (SUCCEEDED(pclsObj->Get(L"NetEnabled", 0, &vtStatus, 0, 0))) {
            info.isEnabled = (vtStatus.boolVal == VARIANT_TRUE);
        }

        VariantClear(&vtName);
        VariantClear(&vtDesc);
        VariantClear(&vtStatus);

        // Filter virtual adapters
        if (IsVirtualAdapter(info.name) || IsVirtualAdapter(info.description)) {
            std::wstring msg = L"NetworkAdapter: Skipping virtual adapter: ";
            msg += info.name;
            Logger::Debug(msg);
            SafeRelease(pclsObj);
            continue;
        }

        // Get MAC address
        VARIANT vtMac;
        VariantInit(&vtMac);
        if (SUCCEEDED(pclsObj->Get(L"MACAddress", 0, &vtMac, 0, 0)) && vtMac.vt == VT_BSTR) {
            info.mac = vtMac.bstrVal;
        }
        VariantClear(&vtMac);

        // Initialize adapter type as Unknown
        info.adapterType = L"Unknown";

        if (!info.name.empty() && !info.mac.empty()) {
            std::wstring msg = L"NetworkAdapter: Added adapter - Name: ";
            msg += info.name;
            msg += L", MAC: ";
            msg += info.mac;
            Logger::Debug(msg);
            adapters.push_back(info);
            adapterCount++;
        }

        SafeRelease(pclsObj);
    }
    
    Logger::Debug("NetworkAdapter: WMI adapter query completed, found " + std::to_string(adapterCount) + " physical adapters");

    SafeRelease(pEnumerator);
}

std::wstring NetworkAdapter::FormatMacAddress(const unsigned char* address, size_t length) const {
    std::wstringstream ss;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0) ss << L":";
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill(L'0')
            << static_cast<int>(address[i]);
    }
    return ss.str();
}

void NetworkAdapter::UpdateAdapterAddresses() {
    Logger::Debug("NetworkAdapter: Updating adapter address information");
    ULONG bufferSize = 15000;
    std::vector<BYTE> buffer(bufferSize);
    PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&buffer[0]);

    DWORD result = GetAdaptersAddresses(AF_INET,
        GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS,  // Include gateway info
        nullptr,
        pAddresses,
        &bufferSize);

    if (result == ERROR_BUFFER_OVERFLOW) {
        Logger::Debug("NetworkAdapter: Buffer size insufficient, reallocating");
        buffer.resize(bufferSize);
        pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&buffer[0]);
        result = GetAdaptersAddresses(AF_INET,
            GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS,
            nullptr,
            pAddresses,
            &bufferSize);
    }

    if (result != NO_ERROR) {
        Logger::Error("Failed to get network adapter addresses: " + std::to_string(result));
        return;
    }

    int updatedCount = 0;
    for (PIP_ADAPTER_ADDRESSES adapter = pAddresses; adapter; adapter = adapter->Next) {
        if (adapter->IfType != IF_TYPE_ETHERNET_CSMACD &&
            adapter->IfType != IF_TYPE_IEEE80211) {
            continue;
        }

        std::wstring macAddress = FormatMacAddress(
            adapter->PhysicalAddress,
            adapter->PhysicalAddressLength);

        for (auto& adapterInfo : adapters) {
            if (adapterInfo.mac == macAddress) {
                std::wstring msg = L"NetworkAdapter: Updating adapter ";
                msg += adapterInfo.name;
                msg += L" information";
                Logger::Debug(msg);
                // Update connection status
                adapterInfo.isConnected = (adapter->OperStatus == IfOperStatusUp);
                std::wstring statusMsg = L"NetworkAdapter: Adapter connection status - ";
                statusMsg += (adapterInfo.isConnected ? L"Connected" : L"Disconnected");
                Logger::Debug(statusMsg);

                // Update network speed - fix for disconnected adapters showing abnormal speeds
                if (adapterInfo.isConnected) {
                    // Only record real speed when connected
                    adapterInfo.speed = adapter->TransmitLinkSpeed;
                    adapterInfo.speedString = FormatSpeed(adapter->TransmitLinkSpeed);
                    std::wstring speedMsg = L"NetworkAdapter: Adapter speed - ";
                    speedMsg += adapterInfo.speedString;
                    Logger::Debug(speedMsg);
                }
                else {
                    // Set to 0 when not connected
                    adapterInfo.speed = 0;
                    adapterInfo.speedString = L"Not connected";
                }

                // Determine adapter type
                adapterInfo.adapterType = DetermineAdapterType(
                    adapterInfo.name,
                    adapterInfo.description,
                    adapter->IfType
                );
                std::wstring typeMsg = L"NetworkAdapter: Adapter type - ";
                typeMsg += adapterInfo.adapterType;
                Logger::Debug(typeMsg);

                // Update IP address (only when connected)
                if (adapterInfo.isConnected) {
                    PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress;
                    while (address) {
                        if (address->Address.lpSockaddr->sa_family == AF_INET) {
                            char ipStr[INET_ADDRSTRLEN];
                            sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(address->Address.lpSockaddr);
                            inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN);
                            adapterInfo.ip = std::wstring(ipStr, ipStr + strlen(ipStr));
                            std::wstring ipMsg = L"NetworkAdapter: Adapter IP address - ";
                            ipMsg += adapterInfo.ip;
                            Logger::Debug(ipMsg);
                            break;
                        }
                        address = address->Next;
                    }
                }
                else {
                    adapterInfo.ip = L"Not connected";
                }
                updatedCount++;
                break;
            }
        }
    }
    
    Logger::Debug("NetworkAdapter: Adapter address update completed, updated " + std::to_string(updatedCount) + " adapters");
}

// Helper method to format network speed
std::wstring NetworkAdapter::FormatSpeed(uint64_t bitsPerSecond) const {
    const double GB = 1000000000.0;
    const double MB = 1000000.0;
    const double KB = 1000.0;

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1);

    if (bitsPerSecond >= GB) {
        ss << (bitsPerSecond / GB) << L" Gbps";
    }
    else if (bitsPerSecond >= MB) {
        ss << (bitsPerSecond / MB) << L" Mbps";
    }
    else if (bitsPerSecond >= KB) {
        ss << (bitsPerSecond / KB) << L" Kbps";
    }
    else {
        ss << bitsPerSecond << L" bps";
    }

    return ss.str();
}

// New method to determine adapter type
std::wstring NetworkAdapter::DetermineAdapterType(const std::wstring& name, const std::wstring& description, DWORD ifType) const {
    // First, judge by Windows interface type
    if (ifType == IF_TYPE_IEEE80211) {
        return L"Wireless";
    }
    else if (ifType == IF_TYPE_ETHERNET_CSMACD) {
        return L"Wired";
    }

    // If interface type is not clear, judge by name and description
    std::wstring combinedText = name + L" " + description;

    // Convert to lowercase for comparison
    std::wstring lowerText = combinedText;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::towlower);

    // Wireless adapter keywords
    const std::vector<std::wstring> wirelessKeywords = {
        L"wi-fi", L"wifi", L"wireless", L"802.11", L"wlan",
        L"ac", L"ax", L"n", L"g",
        L"realtek 8822ce", L"intel wireless", L"qualcomm atheros",
        L"broadcom", L"ralink", L"mediatek"
    };

    // Wired adapter keywords
    const std::vector<std::wstring> ethernetKeywords = {
        L"ethernet", L"gigabit", L"fast ethernet", L"lan",
        L"realtek pcie gbe", L"intel ethernet", L"killer ethernet"
    };

    // Check for wireless keywords
    for (const auto& keyword : wirelessKeywords) {
        if (lowerText.find(keyword) != std::wstring::npos) {
            return L"Wireless";
        }
    }

    // Check for wired keywords
    for (const auto& keyword : ethernetKeywords) {
        if (lowerText.find(keyword) != std::wstring::npos) {
            return L"Wired";
        }
    }

    // By default, infer from interface type
    if (ifType == IF_TYPE_ETHERNET_CSMACD || ifType == IF_TYPE_FASTETHER ||
        ifType == IF_TYPE_GIGABITETHERNET) {
        return L"Wired";
    }

    return L"Unknown Type";
}

void NetworkAdapter::SafeRelease(IUnknown* pInterface) {
    if (pInterface) {
        pInterface->Release();
    }
}

const std::vector<NetworkAdapter::AdapterInfo>& NetworkAdapter::GetAdapters() const {
    return adapters;
}
