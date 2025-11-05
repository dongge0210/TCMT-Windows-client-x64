#pragma once
#include "../common/PlatformDefs.h"
#include <string>
#include <chrono>
#include <vector>

#ifdef PLATFORM_MACOS

struct BatteryCell {
    uint32_t cellIndex;
    double voltage;        // 电压 (V)
    double temperature;    // 温度 (°C)
    double capacity;       // 容量 (mAh)
    bool isHealthy;        // 健康状态
};

class MacBatteryInfo {
public:
    MacBatteryInfo();
    virtual ~MacBatteryInfo();

    // 初始化和清理
    bool Initialize();
    void Cleanup();
    bool IsInitialized() const;
    
    // 更新数据
    bool Update();
    bool IsDataValid() const;
    
    // 基本电池信息
    bool IsBatteryPresent() const;
    bool IsCharging() const;
    bool IsACPowered() const;
    std::string GetBatteryType() const;
    std::string GetBatteryModel() const;
    std::string GetBatteryManufacturer() const;
    std::string GetBatterySerialNumber() const;
    
    // 容量和电量信息
    uint32_t GetCurrentCapacity() const;      // 当前容量 (mAh)
    uint32_t GetMaxCapacity() const;          // 最大容量 (mAh)
    uint32_t GetDesignCapacity() const;        // 设计容量 (mAh)
    uint32_t GetNominalCapacity() const;       // 标称容量 (mAh)
    
    double GetChargePercentage() const;        // 充电百分比 (0-100)
    double GetHealthPercentage() const;         // 健康百分比 (0-100)
    double GetDesignHealthPercentage() const;   // 设计健康百分比
    
    // 电压和电流信息
    double GetVoltage() const;                 // 当前电压 (V)
    double GetAmperage() const;                 // 当前电流 (A)
    double GetWattage() const;                  // 当前功率 (W)
    
    // 时间信息
    uint32_t GetTimeToFullCharge() const;        // 充满所需时间 (分钟)
    uint32_t GetTimeToEmpty() const;            // 用完时间 (分钟)
    uint32_t GetTimeRemaining() const;           // 剩余时间 (分钟)
    
    // 循环计数
    uint32_t GetCycleCount() const;             // 充电循环次数
    uint32_t GetCycleCountLimit() const;         // 循环次数限制
    double GetCycleCountPercentage() const;      // 循环次数百分比
    
    // 温度信息
    double GetTemperature() const;               // 电池温度 (°C)
    std::vector<BatteryCell> GetCellInfo() const; // 单元格信息
    
    // 电源管理信息
    std::string GetPowerSourceState() const;     // 电源状态描述
    bool IsPowerSavingMode() const;              // 是否省电模式
    bool IsOptimizedBatteryCharging() const;     // 是否优化电池充电
    std::string GetChargingState() const;        // 充电状态描述
    
    // 电池状态
    bool IsBatteryHealthy() const;               // 电池是否健康
    std::string GetBatteryHealthStatus() const;  // 电池健康状态
    std::vector<std::string> GetWarnings() const; // 警告信息
    std::vector<std::string> GetErrors() const;   // 错误信息
    
    // 估算信息
    double GetEstimatedRuntime() const;          // 估算运行时间 (小时)
    double GetEstimatedChargingTime() const;     // 估算充电时间 (小时)
    
    // 错误处理
    std::string GetLastError() const;
    void ClearError();

private:
    bool m_initialized;
    uint64_t m_lastUpdateTime;
    std::string m_lastError;
    
    // 基本状态
    bool m_batteryPresent;
    bool m_charging;
    bool m_acPowered;
    std::string m_batteryType;
    std::string m_batteryModel;
    std::string m_batteryManufacturer;
    std::string m_batterySerialNumber;
    
    // 容量信息
    uint32_t m_currentCapacity;
    uint32_t m_maxCapacity;
    uint32_t m_designCapacity;
    uint32_t m_nominalCapacity;
    
    // 电气信息
    double m_voltage;
    double m_amperage;
    double m_wattage;
    
    // 时间信息
    uint32_t m_timeToFullCharge;
    uint32_t m_timeToEmpty;
    uint32_t m_timeRemaining;
    
    // 循环信息
    uint32_t m_cycleCount;
    uint32_t m_cycleCountLimit;
    
    // 温度信息
    double m_temperature;
    std::vector<BatteryCell> m_cellInfo;
    
    // 电源管理
    bool m_powerSavingMode;
    bool m_optimizedBatteryCharging;
    
    // 状态和警告
    std::vector<std::string> m_warnings;
    std::vector<std::string> m_errors;
    
    // 辅助方法
    bool GetBatteryInfoFromIOKit();
    bool GetBatteryInfoFromSystemProfiler();
    bool ParseBatteryData(CFDictionaryRef data);
    bool GetCellInfo();
    void UpdateBatteryHealth();
    void UpdatePowerManagement();
    void CheckBatteryWarnings();
    void SetError(const std::string& error) const;
    void ClearErrorInternal() const;
    
    // IOKit相关方法
    bool GetIOKitBatteryInfo();
    bool GetIOKitPowerInfo();
    
    // 系统分析器相关方法
    bool GetSystemProfilerInfo();
    
    // 数据解析方法
    bool ParseCapacityData(const std::string& data);
    bool ParseVoltageData(const std::string& data);
    bool ParseCurrentData(const std::string& data);
    bool ParseTemperatureData(const std::string& data);
    bool ParseCycleData(const std::string& data);
};

#endif // PLATFORM_MACOS