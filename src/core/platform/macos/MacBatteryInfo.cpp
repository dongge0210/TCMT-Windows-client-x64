#include "MacBatteryInfo.h"
#include "../../Utils/Logger.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include <CoreFoundation/CoreFoundation.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#ifdef PLATFORM_MACOS

MacBatteryInfo::MacBatteryInfo() {
    m_initialized = false;
    m_lastUpdateTime = 0;
    m_batteryPresent = false;
    m_charging = false;
    m_acPowered = false;
    m_currentCapacity = 0;
    m_maxCapacity = 0;
    m_designCapacity = 0;
    m_nominalCapacity = 0;
    m_voltage = 0.0;
    m_amperage = 0.0;
    m_wattage = 0.0;
    m_timeToFullCharge = 0;
    m_timeToEmpty = 0;
    m_timeRemaining = 0;
    m_cycleCount = 0;
    m_cycleCountLimit = 1000; // 默认限制
    m_temperature = 0.0;
    m_powerSavingMode = false;
    m_optimizedBatteryCharging = false;
    
    Initialize();
}

MacBatteryInfo::~MacBatteryInfo() {
    Cleanup();
}

bool MacBatteryInfo::Initialize() {
    try {
        ClearErrorInternal();
        
        // 尝试从IOKit获取电池信息
        if (!GetBatteryInfoFromIOKit()) {
            Logger::Debug("IOKit battery info not available, trying alternative methods");
            
            // 尝试从系统分析器获取信息
            if (!GetBatteryInfoFromSystemProfiler()) {
                SetError("Failed to get battery information from any source");
                return false;
            }
        }
        
        m_initialized = true;
        Logger::Info("MacBatteryInfo initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        SetError("MacBatteryInfo initialization failed: " + std::string(e.what()));
        return false;
    }
}

void MacBatteryInfo::Cleanup() {
    m_initialized = false;
    ClearErrorInternal();
}

bool MacBatteryInfo::IsInitialized() const {
    return m_initialized;
}

bool MacBatteryInfo::Update() {
    if (!m_initialized) {
        SetError("MacBatteryInfo not initialized");
        return false;
    }
    
    bool success = true;
    
    // 更新电池信息
    if (!GetBatteryInfoFromIOKit()) {
        // 如果IOKit失败，尝试备用方法
        if (!GetBatteryInfoFromSystemProfiler()) {
            success = false;
        }
    }
    
    // 更新电池健康状态
    UpdateBatteryHealth();
    
    // 更新电源管理状态
    UpdatePowerManagement();
    
    // 检查电池警告
    CheckBatteryWarnings();
    
    m_lastUpdateTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    return success;
}

bool MacBatteryInfo::IsDataValid() const {
    return m_initialized && (m_lastUpdateTime > 0);
}

bool MacBatteryInfo::IsBatteryPresent() const {
    return m_batteryPresent;
}

bool MacBatteryInfo::IsCharging() const {
    return m_charging;
}

bool MacBatteryInfo::IsACPowered() const {
    return m_acPowered;
}

std::string MacBatteryInfo::GetBatteryType() const {
    return m_batteryType;
}

std::string MacBatteryInfo::GetBatteryModel() const {
    return m_batteryModel;
}

std::string MacBatteryInfo::GetBatteryManufacturer() const {
    return m_batteryManufacturer;
}

std::string MacBatteryInfo::GetBatterySerialNumber() const {
    return m_batterySerialNumber;
}

uint32_t MacBatteryInfo::GetCurrentCapacity() const {
    return m_currentCapacity;
}

uint32_t MacBatteryInfo::GetMaxCapacity() const {
    return m_maxCapacity;
}

uint32_t MacBatteryInfo::GetDesignCapacity() const {
    return m_designCapacity;
}

uint32_t MacBatteryInfo::GetNominalCapacity() const {
    return m_nominalCapacity;
}

double MacBatteryInfo::GetChargePercentage() const {
    if (m_maxCapacity == 0) return 0.0;
    return (double)m_currentCapacity / m_maxCapacity * 100.0;
}

double MacBatteryInfo::GetHealthPercentage() const {
    if (m_designCapacity == 0) return 100.0;
    return (double)m_maxCapacity / m_designCapacity * 100.0;
}

double MacBatteryInfo::GetDesignHealthPercentage() const {
    if (m_nominalCapacity == 0) return 100.0;
    return (double)m_maxCapacity / m_nominalCapacity * 100.0;
}

double MacBatteryInfo::GetVoltage() const {
    return m_voltage;
}

double MacBatteryInfo::GetAmperage() const {
    return m_amperage;
}

double MacBatteryInfo::GetWattage() const {
    return m_wattage;
}

uint32_t MacBatteryInfo::GetTimeToFullCharge() const {
    return m_timeToFullCharge;
}

uint32_t MacBatteryInfo::GetTimeToEmpty() const {
    return m_timeToEmpty;
}

uint32_t MacBatteryInfo::GetTimeRemaining() const {
    return m_timeRemaining;
}

uint32_t MacBatteryInfo::GetCycleCount() const {
    return m_cycleCount;
}

uint32_t MacBatteryInfo::GetCycleCountLimit() const {
    return m_cycleCountLimit;
}

double MacBatteryInfo::GetCycleCountPercentage() const {
    if (m_cycleCountLimit == 0) return 0.0;
    return (double)m_cycleCount / m_cycleCountLimit * 100.0;
}

double MacBatteryInfo::GetTemperature() const {
    return m_temperature;
}

std::vector<BatteryCell> MacBatteryInfo::GetCellInfo() const {
    return m_cellInfo;
}

std::string MacBatteryInfo::GetPowerSourceState() const {
    if (m_acPowered) {
        return m_charging ? "AC Power - Charging" : "AC Power - Charged";
    } else {
        return "Battery Power";
    }
}

bool MacBatteryInfo::IsPowerSavingMode() const {
    return m_powerSavingMode;
}

bool MacBatteryInfo::IsOptimizedBatteryCharging() const {
    return m_optimizedBatteryCharging;
}

std::string MacBatteryInfo::GetChargingState() const {
    if (!m_batteryPresent) {
        return "No Battery";
    }
    
    if (m_acPowered) {
        if (m_charging) {
            return "Charging";
        } else {
            return "Charged";
        }
    } else {
        return "Discharging";
    }
}

bool MacBatteryInfo::IsBatteryHealthy() const {
    double health = GetHealthPercentage();
    double cyclePercent = GetCycleCountPercentage();
    
    // 健康度低于80%或循环次数超过80%认为不健康
    return (health >= 80.0) && (cyclePercent <= 80.0);
}

std::string MacBatteryInfo::GetBatteryHealthStatus() const {
    double health = GetHealthPercentage();
    
    if (health >= 95.0) {
        return "Excellent";
    } else if (health >= 85.0) {
        return "Good";
    } else if (health >= 70.0) {
        return "Fair";
    } else if (health >= 50.0) {
        return "Poor";
    } else {
        return "Very Poor";
    }
}

std::vector<std::string> MacBatteryInfo::GetWarnings() const {
    return m_warnings;
}

std::vector<std::string> MacBatteryInfo::GetErrors() const {
    return m_errors;
}

double MacBatteryInfo::GetEstimatedRuntime() const {
    if (m_acPowered || m_timeToEmpty == 0) {
        return 0.0;
    }
    return m_timeToEmpty / 60.0; // 转换为小时
}

double MacBatteryInfo::GetEstimatedChargingTime() const {
    if (!m_charging || m_timeToFullCharge == 0) {
        return 0.0;
    }
    return m_timeToFullCharge / 60.0; // 转换为小时
}

std::string MacBatteryInfo::GetLastError() const {
    return m_lastError;
}

void MacBatteryInfo::ClearError() {
    ClearErrorInternal();
}

// 私有方法实现
bool MacBatteryInfo::GetBatteryInfoFromIOKit() {
    // 获取电源源信息
    CFTypeRef powerSourceInfo = IOPSCopyPowerSourcesInfo();
    if (!powerSourceInfo) {
        SetError("Failed to get power sources info");
        return false;
    }
    
    CFArrayRef powerSources = IOPSCopyPowerSourcesList(powerSourceInfo);
    if (!powerSources) {
        CFRelease(powerSourceInfo);
        SetError("Failed to get power sources list");
        return false;
    }
    
    CFIndex count = CFArrayGetCount(powerSources);
    bool foundBattery = false;
    
    for (CFIndex i = 0; i < count; ++i) {
        CFTypeRef powerSource = CFArrayGetValueAtIndex(powerSources, i);
        CFDictionaryRef description = IOPSGetPowerSourceDescription(powerSources, powerSource);
        
        if (!description) continue;
        
        // 检查是否是内置电池
        CFStringRef transportType = (CFStringRef)CFDictionaryGetValue(description, CFSTR(kIOPSTransportTypeKey));
        if (transportType && CFStringCompare(transportType, CFSTR(kIOPSInternalType), 0) == kCFCompareEqualTo) {
            foundBattery = true;
            
            // 解析电池信息
            if (!ParseBatteryData(description)) {
                CFRelease(powerSourceInfo);
                CFRelease(powerSources);
                SetError("Failed to parse battery data");
                return false;
            }
            break;
        }
    }
    
    CFRelease(powerSourceInfo);
    CFRelease(powerSources);
    
    if (!foundBattery) {
        m_batteryPresent = false;
        Logger::Debug("No internal battery found");
    } else {
        m_batteryPresent = true;
    }
    
    return true;
}

bool MacBatteryInfo::GetBatteryInfoFromSystemProfiler() {
    // 使用系统分析器命令获取电池信息
    FILE* fp = popen("system_profiler SPPowerDataType -json 2>/dev/null", "r");
    if (!fp) {
        SetError("Failed to execute system_profiler command");
        return false;
    }
    
    std::string jsonOutput;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        jsonOutput += buffer;
    }
    pclose(fp);
    
    if (jsonOutput.empty()) {
        SetError("No output from system_profiler");
        return false;
    }
    
    return ParseBatteryData(jsonOutput);
}

bool MacBatteryInfo::ParseBatteryData(CFDictionaryRef data) {
    // 简化解析，实际应该使用JSON解析器
    // 这里使用字符串搜索来提取关键信息
    
    // 检查电池是否存在
    m_batteryPresent = (data.find("\"sppower_battery_built_in\"") != std::string::npos);
    
    if (!m_batteryPresent) {
        return true; // 没有电池是正常情况
    }
    
    // 解析容量信息
    size_t pos = data.find("\"sppower_battery_current_capacity\"");
    if (pos != std::string::npos) {
        size_t start = data.find("\"", pos + 30);
        size_t end = data.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string capacityStr = data.substr(start + 1, end - start - 1);
            m_currentCapacity = std::stoi(capacityStr);
        }
    }
    
    pos = data.find("\"sppower_battery_max_capacity\"");
    if (pos != std::string::npos) {
        size_t start = data.find("\"", pos + 28);
        size_t end = data.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string capacityStr = data.substr(start + 1, end - start - 1);
            m_maxCapacity = std::stoi(capacityStr);
        }
    }
    
    pos = data.find("\"sppower_battery_design_capacity\"");
    if (pos != std::string::npos) {
        size_t start = data.find("\"", pos + 31);
        size_t end = data.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string capacityStr = data.substr(start + 1, end - start - 1);
            m_designCapacity = std::stoi(capacityStr);
        }
    }
    
    // 解析充电状态
    pos = data.find("\"sppower_battery_is_charging\"");
    if (pos != std::string::npos) {
        size_t start = data.find("\"", pos + 29);
        size_t end = data.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string chargingStr = data.substr(start + 1, end - start - 1);
            m_charging = (chargingStr == "true");
        }
    }
    
    // 解析AC电源状态
    pos = data.find("\"sppower_ac_charger_connected\"");
    if (pos != std::string::npos) {
        size_t start = data.find("\"", pos + 30);
        size_t end = data.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string acStr = data.substr(start + 1, end - start - 1);
            m_acPowered = (acStr == "true");
        }
    }
    
    // 解析循环次数
    pos = data.find("\"sppower_battery_cycle_count\"");
    if (pos != std::string::npos) {
        size_t start = data.find("\"", pos + 30);
        size_t end = data.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string cycleStr = data.substr(start + 1, end - start - 1);
            m_cycleCount = std::stoi(cycleStr);
        }
    }
    
    // 计算功率
    if (m_voltage > 0 && m_amperage != 0) {
        m_wattage = m_voltage * m_amperage;
    }
    
    // 估算时间
    if (m_charging && m_maxCapacity > m_currentCapacity) {
        uint32_t remainingCapacity = m_maxCapacity - m_currentCapacity;
        if (m_amperage > 0) {
            m_timeToFullCharge = remainingCapacity / (uint32_t)(m_amperage * 1000); // 简化计算
        }
    } else if (!m_charging && m_currentCapacity > 0) {
        if (m_amperage < 0) {
            m_timeToEmpty = m_currentCapacity / (uint32_t)(-m_amperage * 1000); // 简化计算
        }
    }
    
    return true;
}

bool MacBatteryInfo::GetCellInfo() {
    // 对于多电池系统，获取每个电池单元的信息
    // 这里简化处理，返回模拟数据
    m_cellInfo.clear();
    
    if (m_batteryPresent) {
        BatteryCell cell;
        cell.cellIndex = 0;
        cell.voltage = m_voltage;
        cell.temperature = m_temperature;
        cell.capacity = m_currentCapacity;
        cell.isHealthy = IsBatteryHealthy();
        m_cellInfo.push_back(cell);
    }
    
    return true;
}

void MacBatteryInfo::UpdateBatteryHealth() {
    // 更新电池健康状态评估
    if (!m_batteryPresent) return;
    
    double health = GetHealthPercentage();
    
    // 检查是否需要添加健康警告
    if (health < 80.0) {
        bool found = false;
        for (const auto& warning : m_warnings) {
            if (warning.find("Battery health") != std::string::npos) {
                found = true;
                break;
            }
        }
        if (!found) {
            m_warnings.push_back("Battery health below 80%");
        }
    }
}

void MacBatteryInfo::UpdatePowerManagement() {
    // 更新电源管理状态
    // 这里可以检查系统省电模式设置
    m_powerSavingMode = (GetChargePercentage() < 20.0);
    
    // 检查优化电池充电（macOS 10.15.5+功能）
    // 这里简化处理
    m_optimizedBatteryCharging = false;
}

void MacBatteryInfo::CheckBatteryWarnings() {
    m_warnings.clear();
    m_errors.clear();
    
    if (!m_batteryPresent) return;
    
    // 检查电池健康度
    double health = GetHealthPercentage();
    if (health < 50.0) {
        m_errors.push_back("Battery health critically low");
    } else if (health < 80.0) {
        m_warnings.push_back("Battery health degraded");
    }
    
    // 检查循环次数
    double cyclePercent = GetCycleCountPercentage();
    if (cyclePercent > 90.0) {
        m_warnings.push_back("Battery cycle count high");
    }
    
    // 检查温度
    if (m_temperature > 45.0) {
        m_warnings.push_back("Battery temperature high");
    }
    
    // 检查电压
    if (m_voltage < 11.0 || m_voltage > 12.6) {
        m_errors.push_back("Battery voltage abnormal");
    }
}

void MacBatteryInfo::SetError(const std::string& error) const {
    m_lastError = error;
    Logger::Error("MacBatteryInfo error: " + error);
}

void MacBatteryInfo::ClearErrorInternal() const {
    m_lastError.clear();
}

#endif // PLATFORM_MACOS