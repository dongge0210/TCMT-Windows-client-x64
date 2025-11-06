#include "MacNetworkInfo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#ifdef PLATFORM_MACOS

MacNetworkInfo::MacNetworkInfo() 
    : m_initialized(false)
    , m_lastError("")
    , m_lastUpdateTime(0)
    , m_dataValid(false)
    , m_currentDownloadSpeed(0.0)
    , m_currentUploadSpeed(0.0)
    , m_lastLatency(0.0)
    , m_lastPacketLoss(0.0)
    , m_primaryInterfaceIndex(0) {
}

MacNetworkInfo::~MacNetworkInfo() {
    Cleanup();
}

bool MacNetworkInfo::Initialize() {
    ClearErrorInternal();
    
    try {
        // 发现所有网络接口
        if (!DiscoverNetworkInterfaces()) {
            SetError("Failed to discover network interfaces");
            return false;
        }
        
        // 找到主要接口
        m_primaryInterfaceIndex = FindPrimaryInterface();
        
        // 获取初始数据
        if (!Update()) {
            SetError("Failed to get initial network data");
            return false;
        }
        
        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        SetError("Initialization failed: " + std::string(e.what()));
        return false;
    }
}

void MacNetworkInfo::Cleanup() {
    m_initialized = false;
    m_dataValid = false;
    m_interfaces.clear();
    m_trafficHistory.clear();
    ClearErrorInternal();
}

bool MacNetworkInfo::IsInitialized() const {
    return m_initialized;
}

bool MacNetworkInfo::Update() {
    if (!m_initialized) {
        SetError("Not initialized");
        return false;
    }
    
    ClearErrorInternal();
    m_dataValid = false;
    
    try {
        // 更新流量统计
        UpdateTrafficStatistics();
        
        // 更新每个接口的信息
        bool success = true;
        for (auto& interface : m_interfaces) {
            success &= UpdateInterfaceInfo(interface);
        }
        
        // 更新主要接口索引
        m_primaryInterfaceIndex = FindPrimaryInterface();
        
        // 测试网络质量
        TestConnectivity();
        MeasureLatency();
        MeasurePacketLoss();
        
        if (success) {
            m_lastUpdateTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            m_dataValid = true;
        }
        
        return success;
    } catch (const std::exception& e) {
        SetError("Update failed: " + std::string(e.what()));
        return false;
    }
}

bool MacNetworkInfo::IsDataValid() const {
    return m_initialized && m_dataValid;
}

uint64_t MacNetworkInfo::GetLastUpdateTime() const {
    return m_lastUpdateTime;
}

std::string MacNetworkInfo::GetLastError() const {
    return m_lastError;
}

void MacNetworkInfo::ClearError() {
    ClearErrorInternal();
}

// INetworkInfo 接口实现
size_t MacNetworkInfo::GetInterfaceCount() const {
    return m_interfaces.size();
}

std::vector<NetworkInterface> MacNetworkInfo::GetAllInterfaces() const {
    return m_interfaces;
}

NetworkInterface MacNetworkInfo::GetInterfaceByIndex(size_t index) const {
    if (index < m_interfaces.size()) {
        return m_interfaces[index];
    }
    return NetworkInterface(); // 返回空对象
}

NetworkInterface MacNetworkInfo::GetInterfaceByName(const std::string& name) const {
    for (const auto& interface : m_interfaces) {
        if (interface.name == name) {
            return interface;
        }
    }
    return NetworkInterface(); // 返回空对象
}

NetworkInterface MacNetworkInfo::GetPrimaryInterface() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        return m_interfaces[m_primaryInterfaceIndex];
    }
    return NetworkInterface(); // 返回空对象
}

uint64_t MacNetworkInfo::GetTotalRxBytes() const {
    uint64_t total = 0;
    for (const auto& interface : m_interfaces) {
        total += interface.totalRxBytes;
    }
    return total;
}

uint64_t MacNetworkInfo::GetTotalTxBytes() const {
    uint64_t total = 0;
    for (const auto& interface : m_interfaces) {
        total += interface.totalTxBytes;
    }
    return total;
}

double MacNetworkInfo::GetCurrentDownloadSpeed() const {
    return m_currentDownloadSpeed;
}

double MacNetworkInfo::GetCurrentUploadSpeed() const {
    return m_currentUploadSpeed;
}

double MacNetworkInfo::GetAverageDownloadSpeed(int minutes) const {
    if (m_trafficHistory.size() < 2) return 0.0;
    
    uint64_t cutoffTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - (minutes * 60 * 1000);
    
    uint64_t totalRx = 0;
    uint64_t firstTime = 0;
    bool first = true;
    
    for (const auto& entry : m_trafficHistory) {
        if (entry.timestamp >= cutoffTime) {
            totalRx += entry.rxBytes;
            if (first) {
                firstTime = entry.timestamp;
                first = false;
            }
        }
    }
    
    if (firstTime == 0 || totalRx == 0) return 0.0;
    
    uint64_t timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - firstTime;
    
    return (double)totalRx / timeDiff * 1000.0 * 8 / (1024 * 1024); // Mbps
}

double MacNetworkInfo::GetAverageUploadSpeed(int minutes) const {
    if (m_trafficHistory.size() < 2) return 0.0;
    
    uint64_t cutoffTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - (minutes * 60 * 1000);
    
    uint64_t totalTx = 0;
    uint64_t firstTime = 0;
    bool first = true;
    
    for (const auto& entry : m_trafficHistory) {
        if (entry.timestamp >= cutoffTime) {
            totalTx += entry.txBytes;
            if (first) {
                firstTime = entry.timestamp;
                first = false;
            }
        }
    }
    
    if (firstTime == 0 || totalTx == 0) return 0.0;
    
    uint64_t timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - firstTime;
    
    return (double)totalTx / timeDiff * 1000.0 * 8 / (1024 * 1024); // Mbps
}

bool MacNetworkInfo::IsConnected() const {
    for (const auto& interface : m_interfaces) {
        if (interface.isActive && !interface.isLoopback) {
            return true;
        }
    }
    return false;
}

bool MacNetworkInfo::IsInternetAvailable() const {
    // 简单的连通性测试
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    int ret = getaddrinfo("8.8.8.8", "53", &hints, &result);
    if (ret != 0) return false;
    
    bool connected = false;
    int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock != -1) {
        if (connect(sock, result->ai_addr, result->ai_addrlen) == 0) {
            connected = true;
        }
        close(sock);
    }
    
    freeaddrinfo(result);
    return connected;
}

std::string MacNetworkInfo::GetConnectionType() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        return GetConnectionTypeFromInterface(m_interfaces[m_primaryInterfaceIndex]);
    }
    return "Unknown";
}

std::string MacNetworkInfo::GetConnectionStatus() const {
    if (IsConnected()) {
        return "Connected";
    } else {
        return "Disconnected";
    }
}

std::string MacNetworkInfo::GetCurrentSSID() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        const auto& interface = m_interfaces[m_primaryInterfaceIndex];
        if (interface.isWireless) {
            return interface.ssid;
        }
    }
    return "";
}

double MacNetworkInfo::GetSignalStrength() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        const auto& interface = m_interfaces[m_primaryInterfaceIndex];
        if (interface.isWireless) {
            return interface.signalStrength;
        }
    }
    return 0.0;
}

uint32_t MacNetworkInfo::GetChannel() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        const auto& interface = m_interfaces[m_primaryInterfaceIndex];
        if (interface.isWireless) {
            return interface.channel;
        }
    }
    return 0;
}

std::string MacNetworkInfo::GetSecurityType() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        const auto& interface = m_interfaces[m_primaryInterfaceIndex];
        if (interface.isWireless) {
            return interface.securityType;
        }
    }
    return "Unknown";
}

std::vector<std::string> MacNetworkInfo::GetAvailableNetworks() const {
    return ScanWiFiNetworks();
}

double MacNetworkInfo::GetLatency() const {
    return m_lastLatency;
}

double MacNetworkInfo::GetPacketLoss() const {
    return m_lastPacketLoss;
}

std::string MacNetworkInfo::GetNetworkQuality() const {
    return CalculateNetworkQuality();
}

bool MacNetworkInfo::HasNetworkIssues() const {
    return m_lastLatency > WARNING_LATENCY || 
           m_lastPacketLoss > WARNING_PACKET_LOSS ||
           !IsInternetAvailable();
}

double MacNetworkInfo::GetBandwidthUsage() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        const auto& interface = m_interfaces[m_primaryInterfaceIndex];
        if (interface.linkSpeed > 0) {
            double currentSpeed = interface.currentSpeed;
            return (currentSpeed / interface.linkSpeed) * 100.0;
        }
    }
    return 0.0;
}

double MacNetworkInfo::GetMaxBandwidth() const {
    if (m_primaryInterfaceIndex < m_interfaces.size()) {
        return m_interfaces[m_primaryInterfaceIndex].linkSpeed;
    }
    return 0.0;
}

std::vector<std::string> MacNetworkInfo::GetNetworkWarnings() const {
    return GenerateNetworkWarnings();
}

std::vector<std::pair<uint64_t, double>> MacNetworkInfo::GetDownloadHistory(int minutes) const {
    std::vector<std::pair<uint64_t, double>> history;
    uint64_t cutoffTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - (minutes * 60 * 1000);
    
    for (const auto& entry : m_trafficHistory) {
        if (entry.timestamp >= cutoffTime) {
            double speed = (double)entry.rxBytes / 1024 / 1024; // MB/s
            history.emplace_back(entry.timestamp, speed);
        }
    }
    
    return history;
}

std::vector<std::pair<uint64_t, double>> MacNetworkInfo::GetUploadHistory(int minutes) const {
    std::vector<std::pair<uint64_t, double>> history;
    uint64_t cutoffTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - (minutes * 60 * 1000);
    
    for (const auto& entry : m_trafficHistory) {
        if (entry.timestamp >= cutoffTime) {
            double speed = (double)entry.txBytes / 1024 / 1024; // MB/s
            history.emplace_back(entry.timestamp, speed);
        }
    }
    
    return history;
}

uint64_t MacNetworkInfo::GetDataUsageToday() const {
    uint64_t todayStart = GetTodayStartTime();
    return CalculateDataUsage(todayStart, 
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}

uint64_t MacNetworkInfo::GetDataUsageThisMonth() const {
    uint64_t monthStart = GetMonthStartTime();
    return CalculateDataUsage(monthStart,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}

// 私有方法实现
void MacNetworkInfo::SetError(const std::string& error) {
    m_lastError = error;
    m_dataValid = false;
}

void MacNetworkInfo::ClearErrorInternal() {
    m_lastError.clear();
}

bool MacNetworkInfo::DiscoverNetworkInterfaces() {
    m_interfaces.clear();
    
    struct ifaddrs *ifap, *ifa;
    
    if (getifaddrs(&ifap) != 0) {
        SetError("Failed to get interface addresses");
        return false;
    }
    
    for (ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        
        NetworkInterface interface;
        interface.name = ifa->ifa_name;
        interface.isActive = (ifa->ifa_flags & IFF_UP) != 0;
        interface.isLoopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;
        
        // 获取MAC地址
        GetInterfaceMACAddress(interface.name, interface.macAddress);
        
        // 获取IP地址
        GetInterfaceAddresses(interface);
        
        // 获取接口速度
        GetInterfaceSpeed(interface.name, interface.linkSpeed);
        
        // 如果是WiFi接口，获取WiFi信息
        if (interface.name.find("en") == 0 || interface.name.find("wifi") == 0) {
            interface.isWireless = true;
            GetWiFiInfo(interface);
        }
        
        // 初始化统计数据
        interface.totalRxBytes = 0;
        interface.totalTxBytes = 0;
        interface.currentRxBytes = 0;
        interface.currentTxBytes = 0;
        interface.currentSpeed = 0.0;
        
        m_interfaces.push_back(interface);
    }
    
    freeifaddrs(ifap);
    return !m_interfaces.empty();
}

bool MacNetworkInfo::UpdateInterfaceInfo(NetworkInterface& interface) {
    return GetInterfaceStatistics(interface.name, interface);
}

bool MacNetworkInfo::GetInterfaceStatistics(const std::string& interfaceName, NetworkInterface& interface) {
    // 模拟网络统计数据获取
    // 实际实现可能需要从系统统计文件获取
    
    static uint64_t baseRx = 1000000;
    static uint64_t baseTx = 500000;
    
    interface.totalRxBytes = baseRx + (rand() % 10000000);
    interface.totalTxBytes = baseTx + (rand() % 5000000);
    
    // 更新当前速度
    interface.currentSpeed = 50.0 + (rand() % 100); // Mbps
    
    baseRx = interface.totalRxBytes;
    baseTx = interface.totalTxBytes;
    
    return true;
}

bool MacNetworkInfo::GetWiFiInfo(NetworkInterface& interface) {
    return GetWiFiNetworkInfo(interface);
}

bool MacNetworkInfo::GetInterfaceAddresses(NetworkInterface& interface) {
    struct ifaddrs *ifap, *ifa;
    
    if (getifaddrs(&ifap) != 0) return false;
    
    for (ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (std::string(ifa->ifa_name) != interface.name) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN);
            interface.ipAddress = ip_str;
            
            // 获取子网掩码
            if (ifa->ifa_netmask && ifa->ifa_netmask->sa_family == AF_INET) {
                struct sockaddr_in* mask_in = (struct sockaddr_in*)ifa->ifa_netmask;
                char mask_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(mask_in->sin_addr), mask_str, INET_ADDRSTRLEN);
                interface.subnetMask = mask_str;
            }
        }
    }
    
    freeifaddrs(ifap);
    return true;
}

bool MacNetworkInfo::GetInterfaceMACAddress(const std::string& interfaceName, std::string& macAddress) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return false;
    
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
        unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        macAddress = mac_str;
    }
    
    close(fd);
    return !macAddress.empty();
}

bool MacNetworkInfo::GetInterfaceSpeed(const std::string& interfaceName, double& speed) {
    // 尝试从系统获取接口速度
    std::string command = "ifconfig " + interfaceName + " | grep 'media:'";
    std::string output;
    
    if (RunCommand(command, output)) {
        // 解析ifconfig输出获取速度
        if (output.find("1000baseT") != std::string::npos) {
            speed = 1000.0; // 1Gbps
        } else if (output.find("100baseTX") != std::string::npos) {
            speed = 100.0;  // 100Mbps
        } else if (output.find("10baseT") != std::string::npos) {
            speed = 10.0;   // 10Mbps
        } else {
            speed = 100.0; // 默认值
        }
        return true;
    }
    
    // 默认值
    speed = 100.0;
    return true;
}

void MacNetworkInfo::UpdateTrafficStatistics() {
    AddTrafficHistoryEntry();
    CleanupOldHistory();
    CalculateCurrentSpeeds();
}

void MacNetworkInfo::AddTrafficHistoryEntry() {
    TrafficHistory entry;
    entry.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    uint64_t totalRx = 0, totalTx = 0;
    for (const auto& interface : m_interfaces) {
        totalRx += interface.totalRxBytes;
        totalTx += interface.totalTxBytes;
    }
    
    entry.rxBytes = totalRx;
    entry.txBytes = totalTx;
    
    m_trafficHistory.push_back(entry);
}

void MacNetworkInfo::CleanupOldHistory() {
    // 保留最近24小时的数据
    uint64_t cutoffTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - (24 * 60 * 60 * 1000);
    
    m_trafficHistory.erase(
        std::remove_if(m_trafficHistory.begin(), m_trafficHistory.end(),
            [cutoffTime](const auto& entry) { return entry.timestamp < cutoffTime; }),
        m_trafficHistory.end()
    );
}

void MacNetworkInfo::CalculateCurrentSpeeds() {
    if (m_trafficHistory.size() < 2) {
        m_currentDownloadSpeed = 0.0;
        m_currentUploadSpeed = 0.0;
        return;
    }
    
    const auto& current = m_trafficHistory.back();
    const auto& previous = m_trafficHistory[m_trafficHistory.size() - 2];
    
    uint64_t timeDiff = current.timestamp - previous.timestamp;
    if (timeDiff == 0) return;
    
    uint64_t rxDiff = current.rxBytes - previous.rxBytes;
    uint64_t txDiff = current.txBytes - previous.txBytes;
    
    m_currentDownloadSpeed = (double)rxDiff / timeDiff * 1000.0 * 8 / (1024 * 1024); // Mbps
    m_currentUploadSpeed = (double)txDiff / timeDiff * 1000.0 * 8 / (1024 * 1024); // Mbps
}

bool MacNetworkInfo::TestConnectivity() {
    return IsInternetAvailable();
}

bool MacNetworkInfo::MeasureLatency() {
    struct timeval start, end;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);
    
    gettimeofday(&start, nullptr);
    
    int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    gettimeofday(&end, nullptr);
    close(sock);
    
    if (result == 0) {
        double latency = (end.tv_sec - start.tv_sec) * 1000.0 + 
                       (end.tv_usec - start.tv_usec) / 1000.0;
        m_lastLatency = latency;
        return true;
    }
    
    m_lastLatency = -1.0;
    return false;
}

bool MacNetworkInfo::MeasurePacketLoss() {
    // 简化的丢包率测量
    m_lastPacketLoss = 0.0; // 默认无丢包
    return true;
}

std::string MacNetworkInfo::CalculateNetworkQuality() const {
    if (!IsConnected()) {
        return "No Connection";
    }
    
    if (m_lastLatency < 50 && m_lastPacketLoss < 0.1) {
        return "Excellent";
    } else if (m_lastLatency < 100 && m_lastPacketLoss < 0.5) {
        return "Good";
    } else if (m_lastLatency < 200 && m_lastPacketLoss < 1.0) {
        return "Fair";
    } else {
        return "Poor";
    }
}

bool MacNetworkInfo::GetWiFiNetworkInfo(NetworkInterface& interface) {
    return GetCurrentWiFiInfo(interface.ssid, interface.signalStrength, interface.channel);
}

bool MacNetworkInfo::GetCurrentWiFiInfo(std::string& ssid, double& signal, uint32_t& channel) {
    std::string output;
    if (RunCommand("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -I", output)) {
        // 解析airport输出
        ParseAirportOutput(output, m_interfaces[0]);
        return true;
    }
    
    // 默认值
    ssid = "Unknown";
    signal = -50.0; // dBm
    channel = 6;
    return false;
}

std::vector<std::string> MacNetworkInfo::ScanWiFiNetworks() const {
    std::vector<std::string> networks;
    std::string output;
    
    if (RunCommand("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -s", output)) {
        // 解析扫描结果
        std::istringstream iss(output);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("SSID") != std::string::npos) {
                size_t pos = line.find("SSID");
                if (pos != std::string::npos) {
                    std::string ssid = line.substr(pos + 5);
                    // 去除引号和空格
                    ssid.erase(std::remove(ssid.begin(), ssid.end(), '"'), ssid.end());
                    ssid.erase(0, ssid.find_first_not_of(" \t"));
                    if (!ssid.empty()) {
                        networks.push_back(ssid);
                    }
                }
            }
        }
    }
    
    return networks;
}

size_t MacNetworkInfo::FindPrimaryInterface() const {
    for (size_t i = 0; i < m_interfaces.size(); i++) {
        const auto& interface = m_interfaces[i];
        if (interface.isActive && !interface.isLoopback) {
            // 优先选择WiFi接口
            if (interface.isWireless) {
                return i;
            }
        }
    }
    
    // 如果没有WiFi，选择第一个活跃的有线接口
    for (size_t i = 0; i < m_interfaces.size(); i++) {
        const auto& interface = m_interfaces[i];
        if (interface.isActive && !interface.isLoopback) {
            return i;
        }
    }
    
    return 0; // 默认第一个
}

std::string MacNetworkInfo::GetConnectionTypeFromInterface(const NetworkInterface& interface) const {
    if (interface.isWireless) {
        return "WiFi";
    } else if (interface.name.find("en") == 0) {
        return "Ethernet";
    } else if (interface.name.find("ppp") == 0) {
        return "PPP";
    } else {
        return "Unknown";
    }
}

bool MacNetworkInfo::RunCommand(const std::string& command, std::string& output) const {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return false;
    
    char buffer[128];
    output.clear();
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    
    int result = pclose(pipe);
    return result == 0;
}

bool MacNetworkInfo::ParseAirportOutput(const std::string& output, NetworkInterface& interface) const {
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.find("SSID") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                interface.ssid = line.substr(pos + 1);
                interface.ssid.erase(0, interface.ssid.find_first_not_of(" \t"));
                interface.ssid.erase(interface.ssid.find_last_not_of(" \t") + 1);
            }
        } else if (line.find("lastTxRate") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string rate = line.substr(pos + 1);
                rate.erase(0, rate.find_first_not_of(" \t"));
                interface.linkSpeed = std::stod(rate);
            }
        } else if (line.find("agrCtlRSSI") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string rssi = line.substr(pos + 1);
                rssi.erase(0, rssi.find_first_not_of(" \t"));
                interface.signalStrength = ConvertSignalStrength(std::stoi(rssi));
            }
        } else if (line.find("channel") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string ch = line.substr(pos + 1);
                ch.erase(0, ch.find_first_not_of(" \t"));
                interface.channel = std::stoul(ch);
            }
        }
    }
    
    return true;
}

std::vector<std::string> MacNetworkInfo::GenerateNetworkWarnings() const {
    std::vector<std::string> warnings;
    
    if (!IsConnected()) {
        warnings.push_back("⚠️ 无网络连接");
    }
    
    if (!IsInternetAvailable()) {
        warnings.push_back("🌐 无法访问互联网");
    }
    
    if (m_lastLatency > WARNING_LATENCY) {
        warnings.push_back("🐌 网络延迟过高");
    }
    
    if (m_lastPacketLoss > WARNING_PACKET_LOSS) {
        warnings.push_back("📉 网络丢包严重");
    }
    
    double bandwidthUsage = GetBandwidthUsage();
    if (bandwidthUsage > HIGH_BANDWIDTH_USAGE) {
        warnings.push_back("📊 带宽使用率过高");
    }
    
    return warnings;
}

uint64_t MacNetworkInfo::CalculateDataUsage(uint64_t startTime, uint64_t endTime) const {
    uint64_t usage = 0;
    
    for (const auto& entry : m_trafficHistory) {
        if (entry.timestamp >= startTime && entry.timestamp <= endTime) {
            usage += entry.rxBytes + entry.txBytes;
        }
    }
    
    return usage;
}

uint64_t MacNetworkInfo::GetTodayStartTime() const {
    auto now = std::chrono::system_clock::now();
    auto today = std::chrono::system_clock::time_point(
        std::chrono::floor<std::chrono::days>(now.time_since_epoch()));
    return std::chrono::duration_cast<std::chrono::milliseconds>(today.time_since_epoch()).count();
}

uint64_t MacNetworkInfo::GetMonthStartTime() const {
    auto now = std::chrono::system_clock::now();
    auto month = std::chrono::system_clock::time_point(
        std::chrono::floor<std::chrono::months>(now.time_since_epoch()));
    return std::chrono::duration_cast<std::chrono::milliseconds>(month.time_since_epoch()).count();
}

double MacNetworkInfo::ConvertSignalStrength(int rssi) const {
    // 将RSSI转换为dBm
    return (double)rssi;
}

#endif // PLATFORM_MACOS