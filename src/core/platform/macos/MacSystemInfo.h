#pragma once
#include "../../common/PlatformDefs.h"
#include "../../common/BaseInfo.h"
#include <string>
#include <chrono>

#ifdef PLATFORM_MACOS

class MacSystemInfo : public ISystemInfo {
public:
    MacSystemInfo();
    virtual ~MacSystemInfo();

    // 初始化和清理
    bool Initialize();
    void Cleanup();
    bool IsInitialized() const;
    
    // 更新数据
    bool Update();
    bool IsDataValid() const;
    uint64_t GetLastUpdateTime() const;
    
    // 系统基本信息
    std::string GetOSName() const;
    std::string GetOSVersion() const;
    std::string GetOSBuild() const;
    std::string GetHostname() const;
    std::string GetUsername() const;
    
    // 系统运行时间
    uint64_t GetUptimeSeconds() const;
    std::string GetBootTime() const;
    std::string GetBootTimeFormatted() const;
    std::string GetUptimeFormatted() const;
    
    // 系统负载
    double GetLoadAverage1Min() const;
    double GetLoadAverage5Min() const;
    double GetLoadAverage15Min() const;
    
    // 进程信息
    uint32_t GetTotalProcesses() const;
    uint32_t GetRunningProcesses() const;
    uint32_t GetSleepingProcesses() const;
    uint32_t GetThreadCount() const;
    
    // 内存信息（从系统角度）
    uint64_t GetTotalPhysicalMemory() const;
    uint64_t GetAvailablePhysicalMemory() const;
    uint64_t GetUsedPhysicalMemory() const;
    double GetMemoryUsagePercentage() const;
    
    // 磁盘信息
    uint64_t GetTotalDiskSpace() const;
    uint64_t GetAvailableDiskSpace() const;
    uint64_t GetUsedDiskSpace() const;
    double GetDiskUsagePercentage() const;
    
    // 网络信息
    std::string GetPrimaryInterface() const;
    uint64_t GetNetworkBytesReceived() const;
    uint64_t GetNetworkBytesSent() const;
    uint64_t GetNetworkPacketsReceived() const;
    uint64_t GetNetworkPacketsSent() const;
    
    // 系统状态
    bool IsSystemHealthy() const;
    std::string GetSystemStatus() const;
    
    // 错误处理
    std::string GetLastError() const;
    void ClearError();

private:
    bool m_initialized;
    bool m_dataValid;
    mutable std::string m_lastError;
    mutable uint64_t m_lastUpdateTime;
    
    // 系统信息缓存
    mutable std::string m_osName;
    mutable std::string m_osVersion;
    mutable std::string m_osBuild;
    mutable std::string m_hostname;
    mutable std::string m_username;
    mutable uint64_t m_uptimeSeconds;
    mutable std::chrono::system_clock::time_point m_bootTime;
    mutable double m_loadAverage[3];
    mutable uint32_t m_totalProcesses;
    mutable uint32_t m_runningProcesses;
    mutable uint32_t m_sleepingProcesses;
    mutable uint32_t m_threadCount;
    mutable uint64_t m_totalPhysicalMemory;
    mutable uint64_t m_availablePhysicalMemory;
    mutable uint64_t m_usedPhysicalMemory;
    mutable uint64_t m_totalSwapMemory;
    mutable uint64_t m_availableSwapMemory;
    mutable uint64_t m_usedSwapMemory;
    mutable uint64_t m_totalDiskSpace;
    mutable uint64_t m_availableDiskSpace;
    mutable uint64_t m_usedDiskSpace;
    mutable uint64_t m_networkBytesReceived;
    mutable uint64_t m_networkBytesSent;
    mutable uint64_t m_networkPacketsReceived;
    mutable uint64_t m_networkPacketsSent;
    mutable std::string m_primaryInterface;
    
    // 私有更新方法
    bool UpdateOSInfo() const;
    bool UpdateHostname() const;
    bool UpdateUsername() const;
    bool UpdateUptime() const;
    bool GetLoadAverages() const;
    bool GetProcessInfo() const;
    bool GetMemoryInfo() const;
    bool GetDiskInfo() const;
    bool GetNetworkInfo() const;
    
    void ClearErrorInternal() const;
    void SetError(const std::string& error) const;
};

#endif // PLATFORM_MACOS