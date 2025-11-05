#pragma once
#include "../../common/BaseInfo.h"
#include "../../common/PlatformDefs.h"
#include <memory>
#include <string>
#include <vector>

#ifdef PLATFORM_MACOS

class MacGpuInfo : public IGpuInfo {
public:
    MacGpuInfo();
    virtual ~MacGpuInfo();

    // IBaseInfo 接口实现
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    virtual bool IsInitialized() const override;
    virtual bool Update() override;
    virtual bool IsDataValid() const override;
    virtual uint64_t GetLastUpdateTime() const override;
    virtual std::string GetLastError() const override;
    virtual void ClearError() override;

    // IGpuInfo 接口实现
    virtual std::string GetName() const override;
    virtual std::string GetVendor() const override;
    virtual std::string GetDriverVersion() const override;
    virtual uint64_t GetDedicatedMemory() const override;
    virtual uint64_t GetSharedMemory() const override;
    
    virtual double GetGpuUsage() const override;
    virtual double GetMemoryUsage() const override;
    virtual double GetVideoDecoderUsage() const override;
    virtual double GetVideoEncoderUsage() const override;
    
    virtual double GetCurrentFrequency() const override;
    virtual double GetBaseFrequency() const override;
    virtual double GetMaxFrequency() const override;
    virtual double GetMemoryFrequency() const override;
    
    virtual double GetTemperature() const override;
    virtual double GetHotspotTemperature() const override;
    virtual double GetMemoryTemperature() const override;
    
    virtual double GetPowerUsage() const override;
    virtual double GetBoardPower() const override;
    virtual double GetMaxPowerLimit() const override;
    
    virtual double GetFanSpeed() const override;
    virtual double GetFanSpeedPercent() const override;
    
    virtual uint64_t GetComputeUnits() const override;
    virtual std::string GetArchitecture() const override;
    virtual double GetPerformanceRating() const override;

private:
    bool GetGpuInfo() const;
    bool GetGpuPerformance() const;
    bool GetGpuTemperature() const;
    bool GetGpuPowerInfo() const;
    void SetError(const std::string& error) const;

    mutable bool m_initialized;
    mutable bool m_dataValid;
    mutable uint64_t m_lastUpdateTime;
    mutable std::string m_lastError;
    
    // GPU基本信息
    mutable std::string m_name;
    mutable std::string m_vendor;
    mutable std::string m_driverVersion;
    mutable std::string m_architecture;
    mutable uint64_t m_dedicatedMemory;
    mutable uint64_t m_sharedMemory;
    mutable uint64_t m_computeUnits;
    
    // 性能信息
    mutable double m_gpuUsage;
    mutable double m_memoryUsage;
    mutable double m_videoDecoderUsage;
    mutable double m_videoEncoderUsage;
    mutable double m_currentFrequency;
    mutable double m_baseFrequency;
    mutable double m_maxFrequency;
    mutable double m_memoryFrequency;
    mutable double m_performanceRating;
    
    // 温度和功耗
    mutable double m_temperature;
    mutable double m_hotspotTemperature;
    mutable double m_memoryTemperature;
    mutable double m_powerUsage;
    mutable double m_boardPower;
    mutable double m_maxPowerLimit;
    
    // 风扇信息
    mutable double m_fanSpeed;
    mutable double m_fanSpeedPercent;
};

#endif // PLATFORM_MACOS