#pragma once
#include "../../common/BaseInfo.h"
#include "../../common/PlatformDefs.h"
#include "MacMemoryInfo.h"
#include <memory>
#include <string>
#include <cstdint>

#ifdef PLATFORM_MACOS

class MacMemoryAdapter {
public:
    MacMemoryAdapter();
    virtual ~MacMemoryAdapter();
    
    // 兼容原始MemoryInfo接口
    uint64_t GetTotalPhysical() const;
    uint64_t GetAvailablePhysical() const;
    uint64_t GetTotalVirtual() const;
    uint64_t GetAvailableVirtual() const;
    
    // 扩展方法，提供更详细的信息
    double GetPhysicalUsagePercentage() const;
    double GetVirtualUsagePercentage() const;
    uint64_t GetUsedPhysical() const;
    uint64_t GetUsedVirtual() const;
    
    // 交换文件信息
    uint64_t GetTotalSwap() const;
    uint64_t GetAvailableSwap() const;
    uint64_t GetUsedSwap() const;
    double GetSwapUsagePercentage() const;
    
    // 内存详情
    double GetMemorySpeed() const;
    std::string GetMemoryType() const;
    uint32_t GetMemoryChannels() const;
    uint64_t GetCachedMemory() const;
    uint64_t GetBufferedMemory() const;
    uint64_t GetSharedMemory() const;
    
    // 内存压力状态
    double GetMemoryPressure() const;
    bool IsMemoryLow() const;
    bool IsMemoryCritical() const;
    
    // 初始化和更新
    bool Initialize();
    void Cleanup();
    bool Update();
    
    // 错误处理
    std::string GetLastError() const;

private:
    std::unique_ptr<MacMemoryInfo> m_macMemoryInfo;
    mutable std::string m_lastError;
    
    void SetError(const std::string& error) const;
};

#endif // PLATFORM_MACOS