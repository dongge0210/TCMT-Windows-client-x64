#pragma once
#include "../../common/BaseInfo.h"
#include <memory>
#include <string>

#ifdef PLATFORM_MACOS

class MacMemoryInfo : public IMemoryInfo {
public:
    MacMemoryInfo();
    virtual ~MacMemoryInfo();

    // IMemoryInfo 接口实现
    virtual uint64_t GetTotalPhysicalMemory() const override;
    virtual uint64_t GetAvailablePhysicalMemory() const override;
    virtual uint64_t GetUsedPhysicalMemory() const override;
    virtual double GetPhysicalMemoryUsage() const override;
    virtual uint64_t GetTotalVirtualMemory() const override;
    virtual uint64_t GetAvailableVirtualMemory() const override;
    virtual uint64_t GetUsedVirtualMemory() const override;
    virtual double GetVirtualMemoryUsage() const override;
    
    // 交换文件/页面文件信息
    virtual uint64_t GetTotalSwapMemory() const override;
    virtual uint64_t GetAvailableSwapMemory() const override;
    virtual uint64_t GetUsedSwapMemory() const override;
    virtual double GetSwapMemoryUsage() const override;
    
    // 内存速度信息
    virtual double GetMemorySpeed() const override;
    virtual std::string GetMemoryType() const override;
    virtual uint32_t GetMemoryChannels() const override;
    
    // 内存使用详情
    virtual uint64_t GetCachedMemory() const override;
    virtual uint64_t GetBufferedMemory() const override;
    virtual uint64_t GetSharedMemory() const override;
    
    // 内存压力
    virtual double GetMemoryPressure() const override;
    virtual bool IsMemoryLow() const override;
    virtual bool IsMemoryCritical() const override;
    
    // IBaseInfo 接口实现
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    virtual bool IsInitialized() const override;
    virtual bool IsDataValid() const override;
    virtual bool Update() override;
    virtual uint64_t GetLastUpdateTime() const override;
    virtual std::string GetLastError() const override;
    virtual void ClearError() override;

    // 高级内存分析方法
    virtual bool AnalyzeMemoryHealth() const;
    virtual std::string GetMemoryStatusDescription() const;
    virtual double GetMemoryEfficiency() const;

private:
    mutable uint64_t m_totalPhysicalMemory;
    mutable uint64_t m_availablePhysicalMemory;
    mutable uint64_t m_totalVirtualMemory;
    mutable uint64_t m_availableVirtualMemory;
    mutable uint64_t m_totalSwapMemory;
    mutable uint64_t m_availableSwapMemory;
    mutable uint64_t m_cachedMemory;
    mutable uint64_t m_bufferedMemory;
    mutable uint64_t m_sharedMemory;
    mutable double m_memoryPressure;
    mutable std::string m_memoryType;
    mutable uint32_t m_memoryChannels;
    mutable double m_memorySpeed;
    mutable bool m_initialized;
    mutable bool m_dataValid;
    mutable uint64_t m_lastUpdateTime;
    mutable std::string m_lastError;
    
    bool GetMemoryInfo() const;
    bool GetSwapInfo() const;
    bool GetMemoryDetails() const;
};

#endif // PLATFORM_MACOS