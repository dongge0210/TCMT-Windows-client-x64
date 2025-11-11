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

uint64_t MacBatteryInfo::GetLastUpdateTime() const {
    return m_lastUpdateTime;
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
    // 使用IOPowerSources API获取电池信息
    CFTypeRef info = IOPSCopyPowerSourcesInfo();
    if (!info) {
        SetError("Failed to get power sources info");
        return false;
    }
    
    CFArrayRef list = IOPSCopyPowerSourcesList(info);
    if (!list || !CFArrayGetCount(list)) {
        CFRelease(info);
        SetError("No power sources found");
        return false;
    }
    
    CFDictionaryRef battery = (CFDictionaryRef)IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(list, 0));
    if (!battery) {
        CFRelease(list);
        CFRelease(info);
        SetError("Failed to get battery description");
        return false;
    }
    
    bool success = ParseBatteryDataFromCFDictionary(battery);
    
    CFRelease(battery);
    CFRelease(list);
    CFRelease(info);
    
    return success;
}

bool MacBatteryInfo::ParseBatteryDataFromCFDictionary(CFDictionaryRef data) {
    // 从CFDictionary中提取电池信息
    if (!data) {
        SetError("Invalid battery data dictionary");
        return false;
    }
    
    // 检查电池是否存在
    CFBooleanRef isPresent = (CFBooleanRef)CFDictionaryGetValue(data, CFSTR("Is Present"));
    m_batteryPresent = isPresent && CFBooleanGetValue(isPresent);
    
    if (!m_batteryPresent) {
        return true; // 没有电池是正常情况
    }
    
    // 获取当前容量
    CFNumberRef currentCap = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("Current Capacity"));
    if (currentCap) {
        SInt32 currentValue;
        if (CFNumberGetValue(currentCap, kCFNumberSInt32Type, &currentValue)) {
            m_currentCapacity = currentValue;
        }
    }
    
    // 获取最大容量
    CFNumberRef maxCap = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("Max Capacity"));
    if (maxCap) {
        SInt32 maxValue;
        if (CFNumberGetValue(maxCap, kCFNumberSInt32Type, &maxValue)) {
            m_maxCapacity = maxValue;
        }
    }
    
    // 获取设计容量 - 使用 DesignCycleCount 作为近似值
    CFNumberRef designCap = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("DesignCycleCount"));
    if (designCap) {
        SInt32 designValue;
        if (CFNumberGetValue(designCap, kCFNumberSInt32Type, &designValue)) {
            m_designCapacity = designValue;
        }
    }
    
    // 获取充电状态
    CFBooleanRef isCharging = (CFBooleanRef)CFDictionaryGetValue(data, CFSTR(kIOPSIsChargingKey));
    m_charging = isCharging && CFBooleanGetValue(isCharging);
    
    // 获取AC电源状态 - 从 Power Source State 判断
    CFStringRef powerState = (CFStringRef)CFDictionaryGetValue(data, CFSTR("Power Source State"));
    if (powerState) {
        const char* stateStr = CFStringGetCStringPtr(powerState, kCFStringEncodingUTF8);
        if (stateStr && strcmp(stateStr, "AC Power") == 0) {
            m_acPowered = true;
        } else {
            m_acPowered = false;
        }
    }
    
    // 获取电压
    CFNumberRef voltage = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("Voltage"));
    if (voltage) {
        SInt32 voltageValue;
        if (CFNumberGetValue(voltage, kCFNumberSInt32Type, &voltageValue)) {
            m_voltage = voltageValue / 1000.0; // 转换为伏特
        }
    }
    
    // 获取电流
    CFNumberRef amperage = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("Current"));
    if (amperage) {
        SInt32 amperageValue;
        if (CFNumberGetValue(amperage, kCFNumberSInt32Type, &amperageValue)) {
            m_amperage = amperageValue / 1000.0; // 转换为安培
        }
    }
    
    // 获取时间信息
    CFNumberRef timeToEmpty = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("Time to Empty"));
    if (timeToEmpty) {
        SInt32 timeValue;
        if (CFNumberGetValue(timeToEmpty, kCFNumberSInt32Type, &timeValue)) {
            m_timeToEmpty = timeValue;
        }
    }
    
    CFNumberRef timeToFull = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("Time to Full Charge"));
    if (timeToFull) {
        SInt32 timeValue;
        if (CFNumberGetValue(timeToFull, kCFNumberSInt32Type, &timeValue)) {
            m_timeToFullCharge = timeValue;
        }
    }
    
    // 获取循环次数
    CFNumberRef cycleCount = (CFNumberRef)CFDictionaryGetValue(data, CFSTR("DesignCycleCount"));
    if (cycleCount) {
        SInt32 cycleValue;
        if (CFNumberGetValue(cycleCount, kCFNumberSInt32Type, &cycleValue)) {
            m_cycleCount = cycleValue;
        }
    }
    
    // 计算百分比
    if (m_maxCapacity > 0) {
        m_chargePercentage = (double)m_currentCapacity / m_maxCapacity * 100.0;
        Logger::Debug("Battery: " + std::to_string(m_currentCapacity) + "/" + std::to_string(m_maxCapacity) + " = " + std::to_string(m_chargePercentage) + "%");
    } else {
        Logger::Debug("Battery: max capacity is 0");
    }
    
    // 计算健康百分比
    if (m_designCapacity > 0) {
        m_healthPercentage = (double)m_maxCapacity / m_designCapacity * 100.0;
        Logger::Debug("Battery Health: " + std::to_string(m_maxCapacity) + "/" + std::to_string(m_designCapacity) + " = " + std::to_string(m_healthPercentage) + "%");
    }
    
    return true;
}

bool MacBatteryInfo::ParseBatteryData(const std::string& data) {
    // 简化解析，使用字符串搜索来提取关键信息
    
    // 检查电池是否存在
    m_batteryPresent = (data.find("\"spbattery_information\"") != std::string::npos);
    
    if (!m_batteryPresent) {
        return true; // 没有电池是正常情况
    }
    
    // 解析当前电量百分比
    size_t pos = data.find("\"sppower_battery_state_of_charge\"");
    if (pos != std::string::npos) {
        size_t start = data.find(":", pos + 35);
        size_t end = data.find(",", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string chargeStr = data.substr(start + 1, end - start - 1);
            // 去除可能的空白字符
            chargeStr.erase(0, chargeStr.find_first_not_of(" \t\n\r"));
            chargeStr.erase(chargeStr.find_last_not_of(" \t\n\r") + 1);
            if (!chargeStr.empty()) {
                m_chargePercentage = std::stoi(chargeStr);
                m_currentCapacity = m_chargePercentage; // 临时存储
                m_maxCapacity = 100; // 临时设置为100%
            }
        }
    }
    
    // 解析充电状态
    pos = data.find("\"sppower_battery_is_charging\"");
    if (pos != std::string::npos) {
        size_t start = data.find(":", pos + 29);
        size_t end = data.find(",", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string chargingStr = data.substr(start + 1, end - start - 1);
            chargingStr.erase(0, chargingStr.find_first_not_of(" \t\n\r"));
            chargingStr.erase(chargingStr.find_last_not_of(" \t\n\r") + 1);
            m_charging = (chargingStr == "true" || chargingStr == "TRUE");
        }
    }
    
    // 解析循环次数
    pos = data.find("\"sppower_battery_cycle_count\"");
    if (pos != std::string::npos) {
        size_t start = data.find(":", pos + 30);
        size_t end = data.find(",", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string cycleStr = data.substr(start + 1, end - start - 1);
            cycleStr.erase(0, cycleStr.find_first_not_of(" \t\n\r"));
            cycleStr.erase(cycleStr.find_last_not_of(" \t\n\r") + 1);
            if (!cycleStr.empty()) {
                m_cycleCount = std::stoi(cycleStr);
            }
        }
    }
    
    // 解析健康百分比
    pos = data.find("\"sppower_battery_health_maximum_capacity\"");
    if (pos != std::string::npos) {
        size_t start = data.find(":", pos + 42);
        size_t end = data.find(",", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            std::string healthStr = data.substr(start + 1, end - start - 1);
            healthStr.erase(0, healthStr.find_first_not_of(" \t\n\r"));
            healthStr.erase(healthStr.find_last_not_of(" \t\n\r") + 1);
            // 移除百分号
            if (!healthStr.empty() && healthStr.back() == '%') {
                healthStr.pop_back();
                m_healthPercentage = std::stod(healthStr);
            }
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

// 添加缺少的方法实现
BatteryInfo MacBatteryInfo::GetDetailedBatteryInfo() const {
    BatteryInfo info;
    info.currentCapacity = m_currentCapacity;
    info.maxCapacity = m_maxCapacity;
    info.designCapacity = m_designCapacity;
    info.cycleCount = m_cycleCount;
    info.isCharging = m_charging;
    info.isPresent = m_batteryPresent;
    info.voltage = m_voltage;
    info.current = m_amperage;
    info.temperature = m_temperature;
    info.healthStatus = GetBatteryHealthStatus();
    info.timeToEmpty = m_timeToEmpty;
    info.timeToFullCharge = m_timeToFullCharge;
    info.powerSourceState = GetPowerSourceState();
    info.powerConsumption = m_wattage;
    info.healthPercentage = GetHealthPercentage();
    info.batterySerial = m_batterySerialNumber;
    info.batteryWarnings.clear();
    auto warnings = GetWarnings();
    for (const auto& warning : warnings) {
        info.batteryWarnings.push_back(warning);
    }
    return info;
}

std::vector<std::string> MacBatteryInfo::GetBatteryWarnings() const {
    return m_warnings;
}

double MacBatteryInfo::GetBatteryWearLevel() const {
    if (m_designCapacity == 0) return 0.0;
    return (1.0 - (double)m_maxCapacity / m_designCapacity) * 100.0;
}

std::string MacBatteryInfo::GetBatterySerial() const {
    return m_batterySerialNumber;
}

std::string MacBatteryInfo::GetManufacturingDate() const {
    // macOS中通常不容易获取制造日期，返回空字符串
    return "";
}

uint32_t MacBatteryInfo::GetPowerOnTime() const {
    // 简化实现，返回0表示未知
    return 0;
}

bool MacBatteryInfo::IsBatteryCalibrated() const {
    // 简化实现，假设电池已校准
    return true;
}



#endif // PLATFORM_MACOS