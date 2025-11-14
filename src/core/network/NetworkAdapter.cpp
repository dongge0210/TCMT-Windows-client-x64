#include "NetworkAdapter.h"
#include "../Utils/Logger.h"
#include "../Utils/CrossPlatformSystemInfo.h"
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
    #include <net/if.h>
    #include <net/if_dl.h>
    #include <ifaddrs.h>
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
#else
NetworkAdapter::NetworkAdapter() : initialized(false) {
    Logger::Debug("NetworkAdapter: Initializing cross-platform network adapter");
#endif
    
    // 初始化跨平台系统信息管理器
    auto& systemInfo = CrossPlatformSystemInfo::GetInstance();
    systemInfo.Initialize();
    
    Initialize();
}

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
// 跨平台网络适配器检测实现
void NetworkAdapter::Initialize() {
    if (initialized) {
        return;
    }

    adapters.clear();
    
#ifdef PLATFORM_WINDOWS
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        Logger::Error("NetworkAdapter: WSAStartup failed");
        return;
    }
#endif

    QueryAdapterInfo();
    initialized = true;
}

void NetworkAdapter::QueryAdapterInfo() {
    adapters.clear();
    
#ifdef PLATFORM_WINDOWS
    QueryWmiAdapterInfo();
    UpdateAdapterAddresses();
#else
    // 使用跨平台系统信息管理器
    auto& systemInfo = CrossPlatformSystemInfo::GetInstance();
    if (systemInfo.Initialize()) {
        auto networkDevices = systemInfo.GetNetworkAdapters();
        
        for (const auto& device : networkDevices) {
            AdapterInfo adapter;
            
            // 转换设备信息到适配器数据结构
            adapter.name = device.name;
            adapter.description = device.description;
            adapter.deviceId = device.deviceId;
            
            // 从属性中提取信息
            auto ipIt = device.properties.find("ip_address");
            if (ipIt != device.properties.end()) {
                adapter.ip = ipIt->second;
                adapter.ipAddresses.push_back(ipIt->second);
            }
            
            auto gatewayIt = device.properties.find("gateway");
            if (gatewayIt != device.properties.end()) {
                adapter.gateway = gatewayIt->second;
            }
            
            auto macIt = device.properties.find("mac_address");
            if (macIt != device.properties.end()) {
                adapter.mac = macIt->second;
            }
            
            auto speedIt = device.properties.find("speed");
            if (speedIt != device.properties.end()) {
                try {
                    adapter.speed = std::stoull(speedIt->second);
                    adapter.speedString = FormatSpeed(adapter.speed);
                }
                catch (...) {
                    adapter.speed = 0;
                    adapter.speedString = "Unknown";
                }
            }
            
            // 设置状态信息
            adapter.isEnabled = true;
            adapter.isConnected = (adapter.speed > 0);
            adapter.isVirtual = IsVirtualAdapter(adapter.name);
            adapter.isWireless = (adapter.name.find("wl") == 0 || adapter.name.find("wlan") == 0);
            
            // 连接状态
            if (adapter.isConnected) {
                adapter.connectionStatus = "Connected";
            } else {
                adapter.connectionStatus = "Disconnected";
            }
            
            // 默认值
            adapter.mtu = 1500;
            adapter.bytesReceived = 0;
            adapter.bytesSent = 0;
            adapter.packetsReceived = 0;
            adapter.packetsSent = 0;
            
            adapters.push_back(adapter);
        }
    }
    
    // 如果跨平台管理器没有找到适配器，使用平台特定方法
    if (adapters.empty()) {
#if defined(PLATFORM_MACOS)
        QueryMacNetworkAdapters();
        UpdateMacAdapterAddresses();
#elif defined(PLATFORM_LINUX)
        QueryLinuxNetworkAdapters();
        UpdateLinuxAdapterAddresses();
#endif
    }
#endif
}

#ifdef PLATFORM_WINDOWS
void NetworkAdapter::QueryWmiAdapterInfo() {
    try {
        std::wstring query = L"SELECT * FROM Win32_NetworkAdapter WHERE NetConnectionStatus = 2";
        auto results = wmiManager.ExecuteQuery(query);
        
        for (const auto& result : results) {
            AdapterInfo info;
            
            // 基本信息
            auto name = result[L"Name"];
            auto description = result[L"Description"];
            auto mac = result[L"MACAddress"];
            auto speed = result[L"Speed"];
            auto adapterType = result[L"AdapterType"];
            auto netConnectionStatus = result[L"NetConnectionStatus"];
            
            if (!name.empty()) {
                info.name = std::string(name.begin(), name.end());
            }
            
            if (!description.empty()) {
                info.description = std::string(description.begin(), description.end());
            }
            
            if (!mac.empty()) {
                info.mac = std::string(mac.begin(), mac.end());
            }
            
            if (!speed.empty()) {
                try {
                    info.speed = std::stoull(speed);
                    info.speedString = FormatSpeed(info.speed);
                } catch (...) {
                    info.speed = 0;
                    info.speedString = "Unknown";
                }
            }
            
            if (!adapterType.empty()) {
                info.adapterType = std::string(adapterType.begin(), adapterType.end());
            }
            
            // 连接状态
            info.isConnected = (netConnectionStatus == L"2");
            info.isEnabled = true;
            
            // 检测虚拟适配器
            info.isVirtual = IsVirtualAdapter(name);
            
            // 检测无线适配器
            info.isWireless = (name.find(L"Wireless") != std::wstring::npos || 
                              name.find(L"Wi-Fi") != std::wstring::npos ||
                              description.find(L"Wireless") != std::wstring::npos);
            
            // 连接状态描述
            if (info.isConnected) {
                info.connectionStatus = "Connected";
            } else {
                info.connectionStatus = "Disconnected";
            }
            
            // 初始化统计字段
            info.bytesReceived = 0;
            info.bytesSent = 0;
            info.packetsReceived = 0;
            info.packetsSent = 0;
            info.mtu = 1500; // 默认MTU
            
            adapters.push_back(info);
        }
    }
    catch (const std::exception& e) {
        Logger::Error("NetworkAdapter: WMI query failed: " + std::string(e.what()));
    }
}

void NetworkAdapter::UpdateAdapterAddresses() {
    ULONG bufferSize = 0;
    
    // 获取所需的缓冲区大小
    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_GATEWAYS, nullptr, nullptr, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
        std::vector<BYTE> buffer(bufferSize);
        PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
        
        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_GATEWAYS, nullptr, pAddresses, &bufferSize) == NO_ERROR) {
            for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != nullptr; pCurrAddresses = pCurrAddresses->Next) {
                // 查找对应的适配器
                for (auto& adapter : adapters) {
                    std::wstring adapterName(pCurrAddresses->FriendlyName);
                    std::string adapterNameStr(adapterName.begin(), adapterName.end());
                    
                    if (adapter.name == adapterNameStr) {
                        // 更新IP地址
                        adapter.ipAddresses.clear();
                        
                        // IPv4地址
                        for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next) {
                            if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                                sockaddr_in* pSockAddr = reinterpret_cast<sockaddr_in*>(pUnicast->Address.lpSockaddr);
                                char strBuffer[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, &pSockAddr->sin_addr, strBuffer, INET_ADDRSTRLEN);
                                adapter.ipAddresses.push_back(strBuffer);
                                if (adapter.ip.empty()) {
                                    adapter.ip = strBuffer;
                                }
                            } else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                                sockaddr_in6* pSockAddr = reinterpret_cast<sockaddr_in6*>(pUnicast->Address.lpSockaddr);
                                char strBuffer[INET6_ADDRSTRLEN];
                                inet_ntop(AF_INET6, &pSockAddr->sin6_addr, strBuffer, INET6_ADDRSTRLEN);
                                adapter.ipv6Address = strBuffer;
                            }
                        }
                        
                        // 网关
                        for (PIP_ADAPTER_GATEWAY_ADDRESS pGateway = pCurrAddresses->FirstGatewayAddress; pGateway != nullptr; pGateway = pGateway->Next) {
                            if (pGateway->Address.lpSockaddr->sa_family == AF_INET) {
                                sockaddr_in* pSockAddr = reinterpret_cast<sockaddr_in*>(pGateway->Address.lpSockaddr);
                                char strBuffer[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, &pSockAddr->sin_addr, strBuffer, INET_ADDRSTRLEN);
                                adapter.gateway = strBuffer;
                            } else if (pGateway->Address.lpSockaddr->sa_family == AF_INET6) {
                                sockaddr_in6* pSockAddr = reinterpret_cast<sockaddr_in6*>(pGateway->Address.lpSockaddr);
                                char strBuffer[INET6_ADDRSTRLEN];
                                inet_ntop(AF_INET6, &pSockAddr->sin6_addr, strBuffer, INET6_ADDRSTRLEN);
                                adapter.ipv6Gateway = strBuffer;
                            }
                        }
                        
                        // MTU
                        adapter.mtu = pCurrAddresses->Mtu;
                        
                        // 获取统计信息
                        if (pCurrAddresses->OperStatus == IfOperStatusUp) {
                            MIB_IF_ROW2 ifRow;
                            memset(&ifRow, 0, sizeof(MIB_IF_ROW2));
                            ifRow.InterfaceLuid = pCurrAddresses->Luid;
                            
                            if (GetIfEntry2(&ifRow) == NO_ERROR) {
                                adapter.bytesReceived = ifRow.InOctets;
                                adapter.bytesSent = ifRow.OutOctets;
                                adapter.packetsReceived = ifRow.InUcastPkts + ifRow.InNUcastPkts;
                                adapter.packetsSent = ifRow.OutUcastPkts + ifRow.OutNUcastPkts;
                            }
                        }
                        
                        break;
                    }
                }
            }
        }
    }
}

#elif defined(PLATFORM_MACOS)
void NetworkAdapter::QueryMacNetworkAdapters() {
    struct ifaddrs* ifaddrsPtr = nullptr;
    
    if (getifaddrs(&ifaddrsPtr) == 0) {
        for (struct ifaddrs* ifa = ifaddrsPtr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr || (ifa->ifa_flags & IFF_UP) == 0) {
                continue;
            }
            
            std::string interfaceName(ifa->ifa_name);
            
            // 检查是否已经处理过这个接口
            bool found = false;
            for (const auto& adapter : adapters) {
                if (adapter.name == interfaceName) {
                    found = true;
                    break;
                }
            }
            
            if (found) {
                continue;
            }
            
            AdapterInfo info;
            info.name = interfaceName;
            info.isEnabled = (ifa->ifa_flags & IFF_UP) != 0;
            info.isConnected = (ifa->ifa_flags & IFF_RUNNING) != 0;
            info.isVirtual = IsVirtualAdapter(interfaceName);
            
            // 检测无线适配器
            info.isWireless = (interfaceName.find("en") == 0);
            
            // 连接状态
            if (info.isConnected) {
                info.connectionStatus = "Connected";
            } else {
                info.connectionStatus = "Disconnected";
            }
            
            // 获取MAC地址
            if (ifa->ifa_addr->sa_family == AF_LINK) {
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
                if (sdl->sdl_alen == 6) {
                    unsigned char* mac = (unsigned char*)LLADDR(sdl);
                    info.mac = FormatMacAddress(mac, 6);
                }
            }
            
            // 获取接口类型和速度
            GetInterfaceMTU(interfaceName, info);
            GetInterfaceStats(interfaceName, info);
            
            // 默认值
            info.speed = 0;
            info.speedString = "Unknown";
            info.adapterType = "Ethernet";
            
            adapters.push_back(info);
        }
        
        freeifaddrs(ifaddrsPtr);
    }
}

void NetworkAdapter::UpdateMacAdapterAddresses() {
    struct ifaddrs* ifaddrsPtr = nullptr;
    
    if (getifaddrs(&ifaddrsPtr) == 0) {
        for (struct ifaddrs* ifa = ifaddrsPtr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) {
                continue;
            }
            
            std::string interfaceName(ifa->ifa_name);
            
            // 查找对应的适配器
            for (auto& adapter : adapters) {
                if (adapter.name == interfaceName) {
                    if (ifa->ifa_addr->sa_family == AF_INET) {
                        struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
                        char strBuffer[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(addr_in->sin_addr), strBuffer, INET_ADDRSTRLEN);
                        
                        // 检查是否已存在
                        bool exists = false;
                        for (const auto& ip : adapter.ipAddresses) {
                            if (ip == strBuffer) {
                                exists = true;
                                break;
                            }
                        }
                        
                        if (!exists) {
                            adapter.ipAddresses.push_back(strBuffer);
                            if (adapter.ip.empty()) {
                                adapter.ip = strBuffer;
                            }
                        }
                    } else if (ifa->ifa_addr->sa_family == AF_INET6) {
                        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)ifa->ifa_addr;
                        char strBuffer[INET6_ADDRSTRLEN];
                        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), strBuffer, INET6_ADDRSTRLEN);
                        adapter.ipv6Address = strBuffer;
                    }
                    break;
                }
            }
        }
        
        freeifaddrs(ifaddrsPtr);
    }
    
    // 获取网关信息
    int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_DUMP, 0};
    size_t len;
    
    if (sysctl(mib, 6, nullptr, &len, nullptr, 0) == 0) {
        std::vector<char> buffer(len);
        if (sysctl(mib, 6, buffer.data(), &len, nullptr, 0) == 0) {
            struct rt_msghdr* rtm = (struct rt_msghdr*)buffer.data();
            struct sockaddr_in* sin = (struct sockaddr_in*)(rtm + 1);
            struct sockaddr_in* gateway = (struct sockaddr_in*)((char*)sin + (sin->sin_len));
            
            if (rtm->rtm_addrs & RTA_GATEWAY) {
                char gatewayStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(gateway->sin_addr), gatewayStr, INET_ADDRSTRLEN);
                
                for (auto& adapter : adapters) {
                    if (!adapter.ip.empty()) {
                        adapter.gateway = gatewayStr;
                        break;
                    }
                }
            }
        }
    }
}

void NetworkAdapter::GetInterfaceMTU(const std::string& interfaceName, AdapterInfo& info) const {
    int mib[] = {CTL_NET, PF_ROUTE, 0, AF_LINK, NET_RT_IFLIST, 0};
    size_t len;
    
    if (sysctl(mib, 6, nullptr, &len, nullptr, 0) == 0) {
        std::vector<char> buffer(len);
        if (sysctl(mib, 6, buffer.data(), &len, nullptr, 0) == 0) {
            struct if_msghdr* ifm = (struct if_msghdr*)buffer.data();
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)(ifm + 1);
            
            if (sdl->sdl_nlen > 0) {
                std::string currentName(sdl->sdl_data, sdl->sdl_nlen);
                if (currentName == interfaceName) {
                    info.mtu = ifm->ifm_mtu;
                }
            }
        }
    }
}

void NetworkAdapter::GetInterfaceStats(const std::string& interfaceName, AdapterInfo& info) const {
    int mib[] = {CTL_NET, PF_ROUTE, 0, AF_LINK, NET_RT_IFLIST2, 0};
    size_t len;
    
    if (sysctl(mib, 6, nullptr, &len, nullptr, 0) == 0) {
        std::vector<char> buffer(len);
        if (sysctl(mib, 6, buffer.data(), &len, nullptr, 0) == 0) {
            struct if_msghdr2* ifm2 = (struct if_msghdr2*)buffer.data();
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)(ifm2 + 1);
            
            if (sdl->sdl_nlen > 0) {
                std::string currentName(sdl->sdl_data, sdl->sdl_nlen);
                if (currentName == interfaceName) {
                    info.bytesReceived = ifm2->ifm_data.ifi_ibytes;
                    info.bytesSent = ifm2->ifm_data.ifi_obytes;
                    info.packetsReceived = ifm2->ifm_data.ifi_ipackets;
                    info.packetsSent = ifm2->ifm_data.ifi_opackets;
                }
            }
        }
    }
}

#elif defined(PLATFORM_LINUX)
void NetworkAdapter::QueryLinuxNetworkAdapters() {
    std::ifstream netDev("/proc/net/dev");
    if (!netDev.is_open()) {
        Logger::Error("NetworkAdapter: Cannot open /proc/net/dev");
        return;
    }
    
    std::string line;
    // 跳过前两行标题
    std::getline(netDev, line);
    std::getline(netDev, line);
    
    while (std::getline(netDev, line)) {
        std::istringstream iss(line);
        std::string interfaceName;
        std::string stats;
        
        // 提取接口名称
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }
        
        interfaceName = line.substr(0, colonPos);
        // 去除空格
        interfaceName.erase(0, interfaceName.find_first_not_of(" \t"));
        interfaceName.erase(interfaceName.find_last_not_of(" \t") + 1);
        
        AdapterInfo info;
        info.name = interfaceName;
        info.isEnabled = true; // Linux中接口存在通常表示启用
        info.isVirtual = IsVirtualAdapter(interfaceName);
        
        // 检测无线适配器
        info.isWireless = (interfaceName.find("wl") == 0 || interfaceName.find("wlan") == 0);
        
        // 解析统计信息
        std::istringstream statsStream(line.substr(colonPos + 1));
        uint64_t rxBytes, rxPackets, rxErrs, rxDrop, rxFifo, rxFrame, rxCompressed, rxMulticast;
        uint64_t txBytes, txPackets, txErrs, txDrop, txFifo, txColls, txCarrier, txCompressed;
        
        statsStream >> rxBytes >> rxPackets >> rxErrs >> rxDrop >> rxFifo >> rxFrame >> rxCompressed >> rxMulticast
                   >> txBytes >> txPackets >> txErrs >> txDrop >> txFifo >> txColls >> txCarrier >> txCompressed;
        
        info.bytesReceived = rxBytes;
        info.bytesSent = txBytes;
        info.packetsReceived = rxPackets;
        info.packetsSent = txPackets;
        
        // 连接状态
        info.isConnected = (rxBytes > 0 || txBytes > 0);
        if (info.isConnected) {
            info.connectionStatus = "Connected";
        } else {
            info.connectionStatus = "Disconnected";
        }
        
        // 默认值
        info.mtu = 1500;
        info.speed = 0;
        info.speedString = "Unknown";
        info.adapterType = "Ethernet";
        
        adapters.push_back(info);
    }
}

void NetworkAdapter::UpdateLinuxAdapterAddresses() {
    struct ifaddrs* ifaddrsPtr = nullptr;
    
    if (getifaddrs(&ifaddrsPtr) == 0) {
        for (struct ifaddrs* ifa = ifaddrsPtr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) {
                continue;
            }
            
            std::string interfaceName(ifa->ifa_name);
            
            // 查找对应的适配器
            for (auto& adapter : adapters) {
                if (adapter.name == interfaceName) {
                    if (ifa->ifa_addr->sa_family == AF_INET) {
                        struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
                        char strBuffer[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(addr_in->sin_addr), strBuffer, INET_ADDRSTRLEN);
                        
                        // 检查是否已存在
                        bool exists = false;
                        for (const auto& ip : adapter.ipAddresses) {
                            if (ip == strBuffer) {
                                exists = true;
                                break;
                            }
                        }
                        
                        if (!exists) {
                            adapter.ipAddresses.push_back(strBuffer);
                            if (adapter.ip.empty()) {
                                adapter.ip = strBuffer;
                            }
                        }
                        
                        // 获取子网掩码
                        if (ifa->ifa_netmask != nullptr) {
                            struct sockaddr_in* mask_in = (struct sockaddr_in*)ifa->ifa_netmask;
                            char maskBuffer[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(mask_in->sin_addr), maskBuffer, INET_ADDRSTRLEN);
                            adapter.subnetMask = maskBuffer;
                        }
                    } else if (ifa->ifa_addr->sa_family == AF_INET6) {
                        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)ifa->ifa_addr;
                        char strBuffer[INET6_ADDRSTRLEN];
                        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), strBuffer, INET6_ADDRSTRLEN);
                        adapter.ipv6Address = strBuffer;
                    }
                    break;
                }
            }
        }
        
        freeifaddrs(ifaddrsPtr);
    }
    
    // 获取MTU信息
    for (auto& adapter : adapters) {
        std::string mtuPath = "/sys/class/net/" + adapter.name + "/mtu";
        std::ifstream mtuFile(mtuPath);
        if (mtuFile.is_open()) {
            mtuFile >> adapter.mtu;
        }
        
        // 获取速度信息
        std::string speedPath = "/sys/class/net/" + adapter.name + "/speed";
        std::ifstream speedFile(speedPath);
        if (speedFile.is_open()) {
            speedFile >> adapter.speed;
            if (adapter.speed > 0) {
                adapter.speedString = FormatSpeed(adapter.speed * 1000000); // Mbps to bps
            }
        }
        
        // 获取驱动信息
        std::string driverPath = "/sys/class/net/" + adapter.name + "/device/driver";
        std::ifstream driverFile(driverPath + "/module");
        if (driverFile.is_open()) {
            std::string driverName;
            driverFile >> driverName;
            adapter.driverVersion = driverName;
        }
    }
    
    // 获取网关信息
    std::ifstream routeFile("/proc/net/route");
    if (routeFile.is_open()) {
        std::string line;
        while (std::getline(routeFile, line)) {
            std::istringstream iss(line);
            std::string interface, destination, gateway;
            iss >> interface >> destination >> gateway;
            
            if (destination == "00000000") { // 默认路由
                uint32_t gatewayAddr;
                std::stringstream ss;
                ss << std::hex << gateway;
                ss >> gatewayAddr;
                
                struct in_addr addr;
                addr.s_addr = gatewayAddr;
                
                char gatewayStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr, gatewayStr, INET_ADDRSTRLEN);
                
                for (auto& adapter : adapters) {
                    if (adapter.name == interface) {
                        adapter.gateway = gatewayStr;
                        break;
                    }
                }
            }
        }
    }
}

#endif

// 通用辅助函数
#ifdef PLATFORM_WINDOWS
std::wstring NetworkAdapter::FormatMacAddress(const unsigned char* address, size_t length) const {
    std::wstringstream ss;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0) {
            ss << L":";
        }
        ss << std::hex << std::setw(2) << std::setfill(L'0') << static_cast<int>(address[i]);
    }
    return ss.str();
}

std::wstring NetworkAdapter::FormatSpeed(uint64_t bitsPerSecond) const {
    const wchar_t* units[] = {L"bps", L"Kbps", L"Mbps", L"Gbps", L"Tbps"};
    int unit = 0;
    double speed = static_cast<double>(bitsPerSecond);
    
    while (speed >= 1000.0 && unit < 4) {
        speed /= 1000.0;
        unit++;
    }
    
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(2) << speed << L" " << units[unit];
    return ss.str();
}

bool NetworkAdapter::IsVirtualAdapter(const std::wstring& name) const {
    const std::vector<std::wstring> virtualPatterns = {
        L"Virtual", L"VMware", L"Hyper-V", L"VBox", L"QEMU", L"TAP", L"TUN",
        L"Loopback", L"Microsoft KM-TEST", L"Microsoft Hosted Network"
    };
    
    for (const auto& pattern : virtualPatterns) {
        if (name.find(pattern) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

#else
std::string NetworkAdapter::FormatMacAddress(const unsigned char* address, size_t length) const {
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0) {
            ss << ":";
        }
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(address[i]);
    }
    return ss.str();
}

std::string NetworkAdapter::FormatSpeed(uint64_t bitsPerSecond) const {
    const char* units[] = {"bps", "Kbps", "Mbps", "Gbps", "Tbps"};
    int unit = 0;
    double speed = static_cast<double>(bitsPerSecond);
    
    while (speed >= 1000.0 && unit < 4) {
        speed /= 1000.0;
        unit++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << speed << " " << units[unit];
    return ss.str();
}

bool NetworkAdapter::IsVirtualAdapter(const std::string& name) const {
    const std::vector<std::string> virtualPatterns = {
        "virtual", "vmware", "hyper-v", "vbox", "qemu", "tap", "tun",
        "loopback", "docker", "virbr", "veth"
    };
    
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    for (const auto& pattern : virtualPatterns) {
        if (lowerName.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

#endif