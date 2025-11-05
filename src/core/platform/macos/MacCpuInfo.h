#pragma once
#include "../../common/BaseInfo.h"
#include "../../common/PlatformDefs.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#ifdef PLATFORM_MACOS
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <mach/host_info.h>
#include <mach/mach_time.h>
#include <CoreFoundation/CoreFoundation.h>
// #include "MacErrorHandler.h"  // 暂时禁用以解决编译问题
#endif

class MacCpuInfo : public ICpuInfo {
public:
    MacCpuInfo();
    virtual ~MacCpuInfo();

    // IBaseInfo 接口实现
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    virtual bool IsInitialized() const override;
    virtual bool Update() override;
    virtual bool IsDataValid() const override;
    virtual uint64_t GetLastUpdateTime() const override;
    virtual std::string GetLastError() const override;
    virtual void ClearError() override;

    // ICpuInfo 接口实现
    virtual std::string GetName() const override;
    virtual std::string GetVendor() const override;
    virtual std::string GetArchitecture() const override;
    virtual uint32_t GetTotalCores() const override;
    virtual uint32_t GetLogicalCores() const override;
    virtual uint32_t GetPhysicalCores() const override;
    virtual uint32_t GetPerformanceCores() const override;
    virtual uint32_t GetEfficiencyCores() const override;
    virtual bool HasHybridArchitecture() const override;
    virtual double GetTotalUsage() const override;
    virtual std::vector<double> GetPerCoreUsage() const override;
    virtual double GetPerformanceCoreUsage() const override;
    virtual double GetEfficiencyCoreUsage() const override;
    virtual double GetCurrentFrequency() const override;
    virtual double GetBaseFrequency() const override;
    virtual double GetMaxFrequency() const override;
    virtual std::vector<double> GetPerCoreFrequencies() const override;
    virtual bool IsHyperThreadingEnabled() const override;
    virtual bool IsVirtualizationEnabled() const override;
    virtual bool SupportsAVX() const override;
    virtual bool SupportsAVX2() const override;
    virtual bool SupportsAVX512() const override;
    virtual double GetTemperature() const override;
    virtual std::vector<double> GetPerCoreTemperatures() const override;
    virtual double GetPowerUsage() const override;
    virtual double GetPackagePower() const override;
    virtual double GetCorePower() const override;

private:
    // 初始化相关
    bool DetectCores();
    void DetectPerformanceEfficiencyCores();
    void DetectFrequencies();
    void DetectFeatures();
    std::string GetCpuName();
    bool GetSysctlInt(const char* name, int& value);
    bool GetSysctlString(const char* name, std::string& value);
    bool GetHostStatistics(host_cpu_load_info_data_t& data) const;
    bool UpdateCurrentFrequency() const;
    bool UpdateTemperature() const;
    bool UpdatePowerUsage() const;
    
    // 基本状态
    bool m_initialized;
    mutable std::string m_lastError;
    uint64_t m_lastUpdateTime;
    
    // 错误处理
    // mutable MacErrorHandler m_errorHandler;  // 暂时禁用
    
    // CPU基本信息
    std::string m_cpuName;
    std::string m_cpuVendor;
    std::string m_cpuArchitecture;
    uint32_t m_totalCores;
    uint32_t m_logicalCores;
    uint32_t m_physicalCores;
    uint32_t m_performanceCores;
    uint32_t m_efficiencyCores;
    bool m_hasHybridArchitecture;
    
    // CPU使用率
    mutable double m_cpuUsage;
    mutable std::vector<double> m_perCoreUsage;
    mutable double m_performanceCoreUsage;
    mutable double m_efficiencyCoreUsage;
    
    // 频率信息 (MHz)
    mutable double m_currentFrequency;
    double m_baseFrequency;
    double m_maxFrequency;
    mutable std::vector<double> m_perCoreFrequencies;
    
    // 特性支持
    bool m_hyperThreadingEnabled;
    bool m_virtualizationEnabled;
    bool m_supportsAVX;
    bool m_supportsAVX2;
    bool m_supportsAVX512;
    
    // 温度信息
    mutable double m_temperature;
    mutable std::vector<double> m_perCoreTemperatures;
    
    // 功耗信息
    mutable double m_powerUsage;
    mutable double m_packagePower;
    mutable double m_corePower;
    
    // 采样相关
    mutable std::chrono::steady_clock::time_point m_lastSampleTime;
    mutable uint64_t m_prevTotalTicks;
    mutable uint64_t m_prevIdleTicks;
    
    // 错误处理
    void SetError(const std::string& error) const;
    void ClearErrorInternal() const;
    bool AttemptErrorRecovery() const;
};