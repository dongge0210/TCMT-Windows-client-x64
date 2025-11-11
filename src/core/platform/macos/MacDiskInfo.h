#pragma once

#include "../../common/BaseInfo.h"
#include "../../common/PlatformDefs.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#ifdef PLATFORM_MACOS
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/sysctl.h>
#include <diskarbitration/diskarbitration.h>
#endif

class MacDiskInfo : public IDiskInfo {
private:
    bool m_initialized;
    std::string m_lastError;
    uint64_t m_lastUpdateTime;
    bool m_dataValid;
    std::string m_diskName;
    
    // 磁盘数据
    std::vector<DiskInfo> m_disks;
    
    // I/O历史数据
    struct IOHistory {
        uint64_t timestamp;
        uint64_t readBytes;
        uint64_t writeBytes;
    };
    std::vector<IOHistory> m_ioHistory;
    
    // 性能统计
    double m_lastReadSpeed;
    double m_lastWriteSpeed;
    
    // 阈值设置
    static constexpr uint64_t WARNING_FREE_SPACE = 10LL * 1024 * 1024 * 1024; // 10GB
    static constexpr double WARNING_USAGE_PERCENTAGE = 90.0;
    static constexpr double CRITICAL_USAGE_PERCENTAGE = 95.0;

public:
    MacDiskInfo(const std::string& diskName = "");
    virtual ~MacDiskInfo();

    // IBaseInfo 接口实现
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    virtual bool IsInitialized() const override;
    virtual bool Update() override;
    virtual bool IsDataValid() const override;
    virtual uint64_t GetLastUpdateTime() const override;
    virtual std::string GetLastError() const override;
    virtual void ClearError() override;
    
    // IDiskInfo 接口实现
    virtual std::string GetName() const override;
    virtual std::string GetModel() const override;
    virtual std::string GetSerialNumber() const override;
    virtual uint64_t GetTotalSize() const override;
    virtual uint64_t GetAvailableSpace() const override;
    virtual uint64_t GetUsedSpace() const override;
    virtual double GetUsagePercentage() const override;
    virtual std::string GetFileSystem() const override;
    virtual double GetReadSpeed() const override;
    virtual double GetWriteSpeed() const override;
    virtual uint64_t GetTotalReadBytes() const override;
    virtual uint64_t GetTotalWriteBytes() const override;
    virtual bool IsHealthy() const override;
    virtual int GetHealthPercentage() const override;
    virtual std::string GetHealthStatus() const override;
    virtual uint64_t GetPowerOnHours() const override;
    virtual std::vector<std::pair<std::string, std::string>> GetSmartAttributes() const override;
    virtual int GetReallocatedSectorCount() const override;
    virtual int GetPendingSectorCount() const override;
    virtual int GetUncorrectableSectorCount() const override;
    virtual bool IsSSD() const override;
    virtual bool IsHDD() const override;
    virtual bool IsNVMe() const override;
    virtual std::string GetInterfaceType() const override;
    virtual std::vector<DiskInfo> GetAllDisks() const;

private:
    // 错误处理
    void SetError(const std::string& error);
    void ClearErrorInternal();
    
    // 磁盘发现和更新
    bool DiscoverDisks();
    bool UpdateDiskInfo(DiskInfo& disk);
    bool GetDiskSpaceInfo(const std::string& path, uint64_t& total, uint64_t& free);
    bool GetDiskPerformanceInfo(DiskInfo& disk);
    bool GetSmartInfo(DiskInfo& disk);
    
    // IOKit相关方法
    bool GetDiskFromIOKit(const std::string& bsdName, DiskInfo& disk);
    bool GetDiskProperties(io_registry_entry_t entry, DiskInfo& disk);
    bool GetDiskPerformanceFromIOKit(DiskInfo& disk);
    
    // 性能监控
    void UpdateIOStatistics();
    void AddIOHistoryEntry();
    void CleanupOldHistory();
    void CalculateCurrentSpeeds();
    
    // 健康检查
    void CalculateHealthScores();
    std::vector<std::string> GenerateDiskWarnings() const;
    bool CheckDiskSpaceWarning(const DiskInfo& disk) const;
    bool CheckPerformanceWarning(const DiskInfo& disk) const;
    
    // 工具方法
    std::string FormatBytes(uint64_t bytes) const;
    std::string GetFileSystemType(const std::string& path) const;
    bool IsSSD(const std::string& model) const;
    bool IsRemovableDisk(const std::string& bsdName) const;
    std::string GetDiskInterface(io_registry_entry_t entry) const;
    double GetDiskTemperatureFromIOKit(const std::string& bsdName) const;
    bool IsEncryptedVolume(const std::string& path) const;
    
    // 系统调用辅助
    bool RunCommand(const std::string& command, std::string& output) const;
    bool ParseDiskutilOutput(const std::string& output, DiskInfo& disk) const;
    bool ParseSmartctlOutput(const std::string& output, DiskInfo& disk) const;
};