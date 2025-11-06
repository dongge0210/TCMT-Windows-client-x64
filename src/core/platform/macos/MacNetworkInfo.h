#pragma once

#include "../../common/BaseInfo.h"
#include "../../common/PlatformDefs.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#ifdef PLATFORM_MACOS
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

class MacNetworkInfo : public INetworkInfo {
private:
    bool m_initialized;
    std::string m_lastError;
    uint64_t m_lastUpdateTime;
    bool m_dataValid;
    
    // 网络接口数据
    std::vector<NetworkInterface> m_interfaces;
    
    // 流量历史数据
    struct TrafficHistory {
        uint64_t timestamp;
        uint64_t rxBytes;
        uint64_t txBytes;
    };
    std::vector<TrafficHistory> m_trafficHistory;
    
    // 性能统计
    double m_currentDownloadSpeed;
    double m_currentUploadSpeed;
    double m_lastLatency;
    double m_lastPacketLoss;
    
    // 主要接口索引
    size_t m_primaryInterfaceIndex;
    
    // 阈值设置
    static constexpr double WARNING_LATENCY = 100.0; // ms
    static constexpr double WARNING_PACKET_LOSS = 1.0; // percentage
    static constexpr double HIGH_BANDWIDTH_USAGE = 80.0; // percentage

public:
    MacNetworkInfo();
    virtual ~MacNetworkInfo();

    // IBaseInfo 接口实现
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    virtual bool IsInitialized() const override;
    virtual bool Update() override;
    virtual bool IsDataValid() const override;
    virtual uint64_t GetLastUpdateTime() const override;
    virtual std::string GetLastError() const override;
    virtual void ClearError() override;

    // INetworkInfo 接口实现
    virtual size_t GetInterfaceCount() const override;
    virtual std::vector<NetworkInterface> GetAllInterfaces() const override;
    virtual NetworkInterface GetInterfaceByIndex(size_t index) const override;
    virtual NetworkInterface GetInterfaceByName(const std::string& name) const override;
    virtual NetworkInterface GetPrimaryInterface() const override;
    virtual uint64_t GetTotalRxBytes() const override;
    virtual uint64_t GetTotalTxBytes() const override;
    virtual double GetCurrentDownloadSpeed() const override;
    virtual double GetCurrentUploadSpeed() const override;
    virtual double GetAverageDownloadSpeed(int minutes = 5) const override;
    virtual double GetAverageUploadSpeed(int minutes = 5) const override;
    virtual bool IsConnected() const override;
    virtual bool IsInternetAvailable() const override;
    virtual std::string GetConnectionType() const override;
    virtual std::string GetConnectionStatus() const override;
    virtual std::string GetCurrentSSID() const override;
    virtual double GetSignalStrength() const override;
    virtual uint32_t GetChannel() const override;
    virtual std::string GetSecurityType() const override;
    virtual std::vector<std::string> GetAvailableNetworks() const override;
    virtual double GetLatency() const override;
    virtual double GetPacketLoss() const override;
    virtual std::string GetNetworkQuality() const override;
    virtual bool HasNetworkIssues() const override;
    virtual double GetBandwidthUsage() const override;
    virtual double GetMaxBandwidth() const override;
    virtual std::vector<std::string> GetNetworkWarnings() const override;
    virtual std::vector<std::pair<uint64_t, double>> GetDownloadHistory(int minutes = 60) const override;
    virtual std::vector<std::pair<uint64_t, double>> GetUploadHistory(int minutes = 60) const override;
    virtual uint64_t GetDataUsageToday() const override;
    virtual uint64_t GetDataUsageThisMonth() const override;

private:
    // 错误处理
    void SetError(const std::string& error);
    void ClearErrorInternal();
    
    // 网络接口发现和更新
    bool DiscoverNetworkInterfaces();
    bool UpdateInterfaceInfo(NetworkInterface& interface);
    bool GetInterfaceStatistics(const std::string& interfaceName, NetworkInterface& interface);
    bool GetWiFiInfo(NetworkInterface& interface);
    
    // 系统调用相关
    bool GetInterfaceAddresses(NetworkInterface& interface);
    bool GetInterfaceMACAddress(const std::string& interfaceName, std::string& macAddress);
    bool GetInterfaceSpeed(const std::string& interfaceName, double& speed);
    
    // 流量统计
    void UpdateTrafficStatistics();
    void AddTrafficHistoryEntry();
    void CleanupOldHistory();
    void CalculateCurrentSpeeds();
    
    // 网络质量测试
    bool TestConnectivity();
    bool MeasureLatency();
    bool MeasurePacketLoss();
    std::string CalculateNetworkQuality() const;
    
    // WiFi相关
    bool GetWiFiNetworkInfo(NetworkInterface& interface);
    bool GetCurrentWiFiInfo(std::string& ssid, double& signal, uint32_t& channel);
    std::vector<std::string> ScanWiFiNetworks() const;
    
    // 工具方法
    size_t FindPrimaryInterface() const;
    std::string GetConnectionTypeFromInterface(const NetworkInterface& interface) const;
    std::string FormatMacAddress(const std::string& rawMac) const;
    std::string FormatIPAddress(uint32_t ip) const;
    bool IsValidIPAddress(const std::string& ip) const;
    double ConvertSignalStrength(int rssi) const;
    std::vector<std::string> GenerateNetworkWarnings() const;
    
    // 命令执行
    bool RunCommand(const std::string& command, std::string& output) const;
    bool ParseNetworksetupOutput(const std::string& output, NetworkInterface& interface) const;
    bool ParseAirportOutput(const std::string& output, NetworkInterface& interface) const;
    
    // 数据使用统计
    uint64_t CalculateDataUsage(uint64_t startTime, uint64_t endTime) const;
    uint64_t GetTodayStartTime() const;
    uint64_t GetMonthStartTime() const;
};