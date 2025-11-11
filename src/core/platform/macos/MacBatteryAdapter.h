#pragma once
#include "../../common/PlatformDefs.h"
#include "../../common/BaseInfo.h"
#include <memory>
#include <string>
#include <vector>

#ifdef PLATFORM_MACOS

// 前向声明
class MacBatteryInfo;

class MacBatteryAdapter {
public:
    MacBatteryAdapter();
    virtual ~MacBatteryAdapter();

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
    uint32_t GetCurrentCapacity() const;
    uint32_t GetMaxCapacity() const;
    uint32_t GetDesignCapacity() const;
    uint32_t GetNominalCapacity() const;
    
    double GetChargePercentage() const;
    double GetHealthPercentage() const;
    double GetDesignHealthPercentage() const;
    
    // 电压和电流信息
    double GetVoltage() const;
    double GetAmperage() const;
    double GetWattage() const;
    
    // 时间信息
    uint32_t GetTimeToFullCharge() const;
    uint32_t GetTimeToEmpty() const;
    uint32_t GetTimeRemaining() const;
    
    // 循环计数
    uint32_t GetCycleCount() const;
    uint32_t GetCycleCountLimit() const;
    double GetCycleCountPercentage() const;
    
    // 温度信息
    double GetTemperature() const;
    std::vector<BatteryCell> GetCellInfo() const;
    
    // 电源管理信息
    std::string GetPowerSourceState() const;
    bool IsPowerSavingMode() const;
    bool IsOptimizedBatteryCharging() const;
    std::string GetChargingState() const;
    
    // 电池状态
    bool IsBatteryHealthy() const;
    std::string GetBatteryHealthStatus() const;
    std::vector<std::string> GetWarnings() const;
    std::vector<std::string> GetErrors() const;
    
    // 估算信息
    double GetEstimatedRuntime() const;
    double GetEstimatedChargingTime() const;
    
    // 错误处理
    std::string GetLastError() const;
    void ClearError();

private:
    std::unique_ptr<MacBatteryInfo> m_macBatteryInfo;
};

#endif // PLATFORM_MACOS