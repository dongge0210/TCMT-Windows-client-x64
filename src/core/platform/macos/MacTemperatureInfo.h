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
#endif

class MacTemperatureInfo : public ITemperatureInfo {
private:
    bool m_initialized;
    std::string m_lastError;
    uint64_t m_lastUpdateTime;
    bool m_dataValid;
    
    // 温度数据
    double m_cpuTemp;
    double m_gpuTemp;
    double m_systemTemp;
    double m_ambientTemp;
    double m_ssdTemp;
    
    // 温度极值
    double m_cpuMaxTemp;
    double m_cpuMinTemp;
    double m_gpuMaxTemp;
    
    // 传感器数据
    std::vector<TemperatureSensor> m_sensors;
    
    // 历史数据
    std::vector<std::pair<uint64_t, double>> m_tempHistory;
    
    // 阈值设置
    static constexpr double CPU_WARNING_TEMP = 80.0;
    static constexpr double CPU_CRITICAL_TEMP = 90.0;
    static constexpr double GPU_WARNING_TEMP = 85.0;
    static constexpr double SYSTEM_WARNING_TEMP = 70.0;

public:
    MacTemperatureInfo();
    virtual ~MacTemperatureInfo();

    // IBaseInfo 接口实现
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    virtual bool IsInitialized() const override;
    virtual bool Update() override;
    virtual bool IsDataValid() const override;
    virtual uint64_t GetLastUpdateTime() const override;
    virtual std::string GetLastError() const override;
    virtual void ClearError() override;

    // ITemperatureInfo 接口实现
    virtual double GetCPUTemperature() const override;
    virtual double GetCpuMaxTemperature() const override;
    virtual double GetCpuMinTemperature() const override;
    virtual double GetGPUTemperature() const override;
    virtual double GetGpuMaxTemperature() const override;
    virtual double GetSystemTemperature() const override;
    virtual double GetAmbientTemperature() const override;
    virtual double GetSSDTemperature() const override;
    virtual std::vector<std::string> GetHDDTemperatures() const override;
    virtual std::vector<TemperatureSensor> GetAllSensors() const override;
    virtual size_t GetSensorCount() const override;
    virtual bool IsOverheating() const override;
    virtual bool IsThermalThrottling() const override;
    virtual double GetThermalPressure() const override;
    virtual std::vector<double> GetTemperatureHistory(int minutes = 60) const override;
    virtual double GetAverageTemperature(int minutes = 10) const override;
    virtual std::vector<std::string> GetTemperatureWarnings() const override;
    virtual bool HasHighTemperatureAlert() const override;

private:
    // 错误处理
    void SetError(const std::string& error);
    void ClearErrorInternal();
    
    // 温度获取方法
    bool GetCPUTemperatureFromIOKit();
    bool GetGPUTemperatureFromIOKit();
    bool GetSystemTemperatureFromSysctl();
    bool GetSSDTemperatureFromIOKit();
    bool DiscoverTemperatureSensors();
    
    // IOKit辅助方法
    bool GetIOKitTemperature(const char* serviceName, const char* key, double& temperature);
    bool EnumerateThermalSensors();
    void ProcessSensorData(io_registry_entry_t entry, const std::string& name);
    
    // 历史数据管理
    void AddTemperatureToHistory(double temperature);
    void CleanupOldHistory();
    
    // 温度状态评估
    double CalculateThermalPressure() const;
    std::vector<std::string> GenerateTemperatureWarnings() const;
    
    // 工具方法
    double KelvinToCelsius(double kelvin) const;
    std::string GetSensorLocation(const std::string& sensorName) const;
    bool IsValidTemperature(double temp) const;
};