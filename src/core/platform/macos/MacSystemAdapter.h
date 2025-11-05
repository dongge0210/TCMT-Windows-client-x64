#pragma once
#include "../../common/PlatformDefs.h"
#include <memory>
#include <string>

#ifdef PLATFORM_MACOS

// 前向声明
class MacSystemInfo;

class MacSystemAdapter {
public:
    MacSystemAdapter();
    virtual ~MacSystemAdapter();

    // 初始化和清理
    bool Initialize();
    void Cleanup();
    bool IsInitialized() const;
    
    // 更新数据
    bool Update();
    bool IsDataValid() const;
    
    // 系统基本信息
    std::string GetOSName() const;
    std::string GetOSVersion() const;
    std::string GetOSBuild() const;
    std::string GetHostname() const;
    std::string GetUsername() const;
    
    // 系统运行时间
    uint64_t GetUptimeSeconds() const;
    std::string GetBootTime() const;
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
    
    // 内存信息
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
    std::unique_ptr<MacSystemInfo> m_macSystemInfo;
};

#endif // PLATFORM_MACOS