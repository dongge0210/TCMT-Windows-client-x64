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
    std::string GetArchitecture() const;
    std::string GetHostname() const;
    std::string GetDomain() const;
    std::string GetUsername() const;
    
    // 系统运行时间
    uint64_t GetUptimeSeconds() const;
    std::string GetBootTime() const;
    std::string GetBootTimeFormatted() const;
    std::string GetUptimeFormatted() const;
    std::string GetLocalTime() const;
    std::string GetUTCTime() const;
    std::string GetTimezone() const;
    
    // 系统负载
    double GetLoadAverage1Min() const;
    double GetLoadAverage5Min() const;
    double GetLoadAverage15Min() const;
    double GetCPULoadAverage() const;
    
    // 进程信息
    uint32_t GetProcessCount() const;
    uint32_t GetRunningProcessCount() const;
    uint32_t GetSleepingProcessCount() const;
    uint32_t GetThreadCount() const;
    uint32_t GetMaxProcesses() const;
    uint32_t GetTotalProcesses() const;
    uint32_t GetRunningProcesses() const;
    uint32_t GetSleepingProcesses() const;
    
    // 内存信息（从系统角度）
    uint64_t GetTotalMemory() const;
    uint64_t GetAvailableMemory() const;
    uint64_t GetUsedMemory() const;
    uint64_t GetCacheMemory() const;
    uint64_t GetSwapMemory() const;
    double GetMemoryUsagePercentage() const;
    double GetMemoryPressure() const;
    uint64_t GetTotalPhysicalMemory() const;
    uint64_t GetAvailablePhysicalMemory() const;
    uint64_t GetUsedPhysicalMemory() const;
    
    // 磁盘信息
    uint64_t GetTotalDiskSpace() const;
    uint64_t GetAvailableDiskSpace() const;
    uint64_t GetUsedDiskSpace() const;
    double GetDiskUsagePercentage() const;
    uint32_t GetDiskReadOps() const;
    uint32_t GetDiskWriteOps() const;
    uint64_t GetDiskReadBytes() const;
    uint64_t GetDiskWriteBytes() const;
    
    // 网络信息
    uint32_t GetNetworkInterfaceCount() const;
    uint64_t GetTotalBytesReceived() const;
    uint64_t GetTotalBytesSent() const;
    double GetNetworkUtilization() const;
    std::string GetPrimaryInterface() const;
    uint64_t GetNetworkBytesReceived() const;
    uint64_t GetNetworkBytesSent() const;
    uint64_t GetNetworkPacketsReceived() const;
    uint64_t GetNetworkPacketsSent() const;
    
    // 系统状态
    bool IsSystemHealthy() const;
    std::string GetSystemStatus() const;
    std::vector<std::string> GetSystemWarnings() const;
    std::vector<std::string> GetSystemErrors() const;
    double GetSystemHealthScore() const;
    
    // 安全信息
    bool IsSecureBootEnabled() const;
    bool IsFirewallEnabled() const;
    bool IsAntivirusRunning() const;
    std::string GetSecurityStatus() const;
    
    // 硬件信息
    std::string GetMotherboardModel() const;
    std::string GetBIOSVersion() const;
    std::string GetFirmwareVersion() const;
    std::string GetSerialNumber() const;
    
    // 虚拟化信息
    bool IsVirtualMachine() const;
    std::string GetVirtualizationPlatform() const;
    uint32_t GetVirtualCPUCount() const;
    uint64_t GetVirtualMemory() const;
    
    // 环境变量
    std::vector<std::string> GetEnvironmentVariables() const;
    std::string GetEnvironmentVariable(const std::string& name) const;
    
    // 系统更新
    std::string GetLastSystemUpdateTime() const;
    bool UpdatesAvailable() const;
    std::vector<std::string> GetPendingUpdates() const;
    
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
    mutable std::string m_architecture;
    mutable std::string m_hostname;
    mutable std::string m_domain;
    mutable std::string m_username;
    mutable uint64_t m_uptimeSeconds;
    mutable std::chrono::system_clock::time_point m_bootTime;
    mutable double m_loadAverage[3];
    mutable uint32_t m_processCount;
    mutable uint32_t m_runningProcessCount;
    mutable uint32_t m_sleepingProcessCount;
    mutable uint32_t m_threadCount;
    mutable uint32_t m_maxProcesses;
    mutable uint32_t m_totalProcesses;
    mutable uint32_t m_runningProcesses;
    mutable uint32_t m_sleepingProcesses;
    mutable uint64_t m_totalMemory;
    mutable uint64_t m_availableMemory;
    mutable uint64_t m_usedMemory;
    mutable uint64_t m_cacheMemory;
    mutable uint64_t m_swapMemory;
    mutable double m_memoryPressure;
    mutable uint64_t m_totalPhysicalMemory;
    mutable uint64_t m_availablePhysicalMemory;
    mutable uint64_t m_usedPhysicalMemory;
    mutable uint64_t m_totalSwapMemory;
    mutable uint64_t m_availableSwapMemory;
    mutable uint64_t m_usedSwapMemory;
    mutable uint64_t m_totalDiskSpace;
    mutable uint64_t m_availableDiskSpace;
    mutable uint64_t m_usedDiskSpace;
    mutable uint32_t m_diskReadOps;
    mutable uint32_t m_diskWriteOps;
    mutable uint64_t m_diskReadBytes;
    mutable uint64_t m_diskWriteBytes;
    mutable uint32_t m_networkInterfaceCount;
    mutable uint64_t m_totalBytesReceived;
    mutable uint64_t m_totalBytesSent;
    mutable double m_networkUtilization;
    mutable uint64_t m_networkBytesReceived;
    mutable uint64_t m_networkBytesSent;
    mutable uint64_t m_networkPacketsReceived;
    mutable uint64_t m_networkPacketsSent;
    mutable std::string m_primaryInterface;
    mutable std::vector<std::string> m_systemWarnings;
    mutable std::vector<std::string> m_systemErrors;
    mutable double m_systemHealthScore;
    mutable std::string m_motherboardModel;
    mutable std::string m_biosVersion;
    mutable std::string m_firmwareVersion;
    mutable std::string m_serialNumber;
    mutable bool m_isVirtualMachine;
    mutable std::string m_virtualizationPlatform;
    mutable uint32_t m_virtualCPUCount;
    mutable uint64_t m_virtualMemory;
    
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