#pragma once
#include "core/common/BaseInfo.h"
#include "core/common/PlatformDefs.h"
#include <windows.h>
#include <pdh.h>
#include <string>
#include <vector>
#include <memory>

class WinCpuInfo : public ICpuInfo {
public:
    WinCpuInfo();
    virtual ~WinCpuInfo();

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
    bool InitializeCpuInfo();
    bool InitializePdhCounters();
    bool InitializeFrequencyCounters();
    bool InitializeTemperatureMonitoring();
    void CleanupPdhCounters();
    void CleanupFrequencyCounters();
    void CleanupTemperatureMonitoring();

    // 数据更新
    bool UpdateCpuUsage();
    bool UpdateCpuFrequency();
    bool UpdateCpuTemperature();
    bool UpdateCpuPower();

    // CPU信息获取
    std::string GetNameFromRegistry();
    std::string GetVendorFromCpuid();
    std::string GetArchitectureFromSystem();
    bool DetectCores();
    bool DetectHybridArchitecture();
    bool DetectInstructionSets();
    bool DetectVirtualizationSupport();

    // PDH计数器相关
    PDH_HQUERY m_queryHandle;
    PDH_HCOUNTER m_totalCounter;
    Vector<PDH_HCOUNTER> m_perCoreCounters;
    bool m_pdhInitialized;

    // 频率计数器相关
    PDH_HQUERY m_freqQueryHandle;
    PDH_HCOUNTER m_freqCounter;
    Vector<PDH_HCOUNTER> m_perCoreFreqCounters;
    bool m_freqInitialized;

    // 温度监控相关
    bool m_tempInitialized;
    std::string m_tempSensorPath;

    // 功耗监控相关
    bool m_powerInitialized;
    std::string m_powerSensorPath;

    // 缓存的数据
    std::string m_name;
    std::string m_vendor;
    std::string m_architecture;
    uint32_t m_totalCores;
    uint32_t m_logicalCores;
    uint32_t m_physicalCores;
    uint32_t m_performanceCores;
    uint32_t m_efficiencyCores;
    bool m_hasHybridArchitecture;
    double m_totalUsage;
    std::vector<double> m_perCoreUsage;
    double m_performanceCoreUsage;
    double m_efficiencyCoreUsage;
    double m_currentFrequency;
    double m_baseFrequency;
    double m_maxFrequency;
    std::vector<double> m_perCoreFrequencies;
    bool m_isHyperThreadingEnabled;
    bool m_isVirtualizationEnabled;
    bool m_supportsAVX;
    bool m_supportsAVX2;
    bool m_supportsAVX512;
    double m_temperature;
    std::vector<double> m_perCoreTemperatures;
    double m_powerUsage;
    double m_packagePower;
    double m_corePower;

    // 状态管理
    bool m_initialized;
    bool m_dataValid;
    uint64_t m_lastUpdateTime;
    std::string m_lastError;

    // 辅助函数
    void SetError(const std::string& error);
    bool ReadRegistryValue(const std::string& keyPath, const std::string& valueName, std::string& result);
    bool ReadRegistryDword(const std::string& keyPath, const std::string& valueName, uint32_t& result);
    bool GetCpuidInfo(uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx, uint32_t leaf);
    bool DetectHyperThreading();
    bool DetectVirtualization();
    bool DetectInstructionSet(uint32_t featureBit);
    double GetCounterValue(PDH_HCOUNTER counter);
    bool UpdateCounterValue(PDH_HCOUNTER counter, double& value);
    bool GetTemperatureFromWmi(double& temperature);
    bool GetPowerFromWmi(double& power, double& packagePower, double& corePower);
    bool GetFrequencyFromRegistry(double& frequency);
    std::string GetCpuBrandString();
};