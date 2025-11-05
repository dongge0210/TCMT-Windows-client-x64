#pragma once
#include "../../common/BaseInfo.h"
#include "../../common/PlatformDefs.h"
#include "MacGpuInfo.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#ifdef PLATFORM_MACOS

class MacGpuAdapter {
public:
    MacGpuAdapter();
    virtual ~MacGpuAdapter();
    
    // 兼容原始GpuInfo接口
    struct GpuData {
        std::string name;
        std::string vendor;
        uint64_t dedicatedMemory;
        uint64_t sharedMemory;
        double usage;
        double temperature;
        double powerUsage;
        double fanSpeed;
    };
    
    const std::vector<GpuData>& GetGpuData() const;
    
    // 扩展方法，提供更详细的信息
    double GetGpuUsage() const;
    double GetMemoryUsage() const;
    double GetVideoDecoderUsage() const;
    double GetVideoEncoderUsage() const;
    double GetCurrentFrequency() const;
    double GetBaseFrequency() const;
    double GetMaxFrequency() const;
    double GetTemperature() const;
    double GetHotspotTemperature() const;
    double GetMemoryTemperature() const;
    double GetPowerUsage() const;
    double GetBoardPower() const;
    double GetMaxPowerLimit() const;
    double GetFanSpeed() const;
    double GetFanSpeedPercent() const;
    uint64_t GetComputeUnits() const;
    std::string GetArchitecture() const;
    double GetPerformanceRating() const;
    
    // 内存信息
    uint64_t GetDedicatedMemory() const;
    uint64_t GetSharedMemory() const;
    uint64_t GetTotalMemory() const;
    
    // 初始化和更新
    bool Initialize();
    void Cleanup();
    bool Update();
    
    // 错误处理
    std::string GetLastError() const;

private:
    std::unique_ptr<MacGpuInfo> m_macGpuInfo;
    mutable std::vector<GpuData> m_gpuData;
    mutable std::string m_lastError;
    
    void UpdateGpuData();
    void SetError(const std::string& error) const;
};

#endif // PLATFORM_MACOS