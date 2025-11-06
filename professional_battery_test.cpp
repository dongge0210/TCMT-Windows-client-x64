#include <iostream>
#include <iomanip>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

// 专业的电池信息测试
void testProfessionalBatteryInfo() {
    std::cout << "\n=== 专业电池信息测试 ===" << std::endl;
    
    // 获取电源信息
    CFTypeRef powerSourceInfo = IOPSCopyPowerSourcesInfo();
    if (!powerSourceInfo) {
        std::cout << "✗ 无法获取电源信息" << std::endl;
        return;
    }
    
    CFArrayRef powerSources = IOPSCopyPowerSourcesList(powerSourceInfo);
    if (!powerSources) {
        std::cout << "✗ 无法获取电源列表" << std::endl;
        CFRelease(powerSourceInfo);
        return;
    }
    
    CFIndex count = CFArrayGetCount(powerSources);
    std::cout << "找到 " << count << " 个电源" << std::endl;
    
    for (CFIndex i = 0; i < count; i++) {
        CFTypeRef powerSource = CFArrayGetValueAtIndex(powerSources, i);
        CFDictionaryRef description = IOPSGetPowerSourceDescription(powerSources, powerSource);
        
        if (!description) {
            std::cout << "✗ 无法获取电源描述 " << i << std::endl;
            continue;
        }
        
        std::cout << "\n--- 电源 " << (i + 1) << " ---" << std::endl;
        
        // 检查是否为内置电池
        CFBooleanRef isInternal = (CFBooleanRef)CFDictionaryGetValue(description, CFSTR("Internal"));
        CFBooleanRef isPresent = (CFBooleanRef)CFDictionaryGetValue(description, CFSTR("Present"));
        
        if (isInternal && CFBooleanGetValue(isInternal) && isPresent && CFBooleanGetValue(isPresent)) {
            std::cout << "类型: 内置电池" << std::endl;
            
            // 获取电池基本信息
            CFStringRef name = (CFStringRef)CFDictionaryGetValue(description, CFSTR(kIOPSNameKey));
            if (name) {
                char nameStr[256];
                CFStringGetCString(name, nameStr, sizeof(nameStr), kCFStringEncodingUTF8);
                std::cout << "电池名称: " << nameStr << std::endl;
            }
            
            // 获取电池容量信息
            CFNumberRef currentCapacity = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSCurrentCapacityKey));
            CFNumberRef maxCapacity = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSMaxCapacityKey));
            CFNumberRef designCapacity = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSDesignCapacityKey));
            
            int currentCap = 0, maxCap = 0, designCap = 0;
            if (currentCapacity) CFNumberGetValue(currentCapacity, kCFNumberIntType, &currentCap);
            if (maxCapacity) CFNumberGetValue(maxCapacity, kCFNumberIntType, &maxCap);
            if (designCapacity) CFNumberGetValue(designCapacity, kCFNumberIntType, &designCap);
            
            if (currentCap > 0 && maxCap > 0) {
                double percentage = (double)currentCap / maxCap * 100;
                std::cout << "当前电量: " << std::fixed << std::setprecision(1) << percentage << "%" << std::endl;
                std::cout << "当前容量: " << currentCap << " mAh" << std::endl;
                std::cout << "最大容量: " << maxCap << " mAh" << std::endl;
            }
            
            if (designCap > 0) {
                std::cout << "设计容量: " << designCap << " mAh" << std::endl;
                
                if (maxCap > 0) {
                    double health = (double)maxCap / designCap * 100;
                    std::cout << "电池健康度: " << std::fixed << std::setprecision(1) << health << "%" << std::endl;
                    
                    // 健康状态评估
                    std::string healthStatus;
                    if (health >= 85.0) {
                        healthStatus = "优秀";
                    } else if (health >= 70.0) {
                        healthStatus = "良好";
                    } else if (health >= 50.0) {
                        healthStatus = "一般";
                    } else {
                        healthStatus = "需要更换";
                    }
                    std::cout << "健康状态: " << healthStatus << std::endl;
                    
                    // 容量损耗
                    double degradation = (1.0 - (double)maxCap / designCap) * 100;
                    std::cout << "容量损耗: " << std::fixed << std::setprecision(1) << degradation << "%" << std::endl;
                }
            }
            
            // 获取循环次数
            CFNumberRef cycleCount = (CFNumberRef)CFDictionaryGetValue(description, CFSTR("CycleCount"));
            if (cycleCount) {
                int cycles = 0;
                CFNumberGetValue(cycleCount, kCFNumberIntType, &cycles);
                std::cout << "充放电循环: " << cycles << " 次" << std::endl;
                
                if (cycles > 0) {
                    std::string cycleStatus;
                    if (cycles < 300) {
                        cycleStatus = "极新";
                    } else if (cycles < 800) {
                        cycleStatus = "正常";
                    } else if (cycles < 1500) {
                        cycleStatus = "较多";
                    } else {
                        cycleStatus = "很多";
                    }
                    std::cout << "循环状态: " << cycleStatus << std::endl;
                }
            }
            
            // 获取充电状态
            CFBooleanRef isCharging = (CFBooleanRef)CFDictionaryGetValue(description, CFSTR(kIOPSIsChargingKey));
            CFStringRef powerState = (CFStringRef)CFDictionaryGetValue(description, CFSTR(kIOPSPowerSourceStateKey));
            
            std::cout << "充电状态: ";
            if (isCharging) {
                std::cout << (CFBooleanGetValue(isCharging) ? "充电中" : "未充电") << std::endl;
            }
            
            if (powerState) {
                char stateStr[256];
                CFStringGetCString(powerState, stateStr, sizeof(stateStr), kCFStringEncodingUTF8);
                std::cout << "电源状态: " << stateStr << std::endl;
            }
            
            // 获取时间信息
            CFNumberRef timeToFull = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSTimeToFullChargeKey));
            CFNumberRef timeToEmpty = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSTimeToEmptyKey));
            
            if (timeToFull) {
                int minutes = 0;
                CFNumberGetValue(timeToFull, kCFNumberIntType, &minutes);
                if (minutes > 0 && minutes != -1) {
                    std::cout << "充满时间: " << (minutes / 60) << "小时 " << (minutes % 60) << "分钟" << std::endl;
                }
            }
            
            if (timeToEmpty) {
                int minutes = 0;
                CFNumberGetValue(timeToEmpty, kCFNumberIntType, &minutes);
                if (minutes > 0 && minutes != -1) {
                    std::cout << "剩余时间: " << (minutes / 60) << "小时 " << (minutes % 60) << "分钟" << std::endl;
                }
            }
            
            // 获取电气参数
            CFNumberRef voltage = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSVoltageKey));
            CFNumberRef current = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSCurrentKey));
            
            float volt = 0;
            int cur = 0;
            
            if (voltage) {
                CFNumberGetValue(voltage, kCFNumberFloatType, &volt);
                std::cout << "电压: " << std::fixed << std::setprecision(2) << volt << " V" << std::endl;
            }
            
            if (current) {
                CFNumberGetValue(current, kCFNumberIntType, &cur);
                std::cout << "电流: " << cur << " mA ";
                
                if (cur > 0) {
                    std::cout << "(充电)" << std::endl;
                } else if (cur < 0) {
                    std::cout << "(放电)" << std::endl;
                } else {
                    std::cout << "(待机)" << std::endl;
                }
                
                // 计算功率
                if (voltage && cur != 0) {
                    double power = (volt * std::abs(cur)) / 1000.0; // V * mA = mW, 转换为W
                    std::cout << "功率: " << std::fixed << std::setprecision(2) << power << " W" << std::endl;
                }
            }
            
            // 获取温度信息
            CFNumberRef temperature = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSTemperatureKey));
            if (temperature) {
                float temp = 0;
                CFNumberGetValue(temperature, kCFNumberFloatType, &temp);
                double celsius = temp - 273.15; // 从开尔文转换到摄氏度
                std::cout << "电池温度: " << std::fixed << std::setprecision(1) << celsius << "°C" << std::endl;
                
                // 温度状态评估
                std::string tempStatus;
                if (celsius < 15.0) {
                    tempStatus = "偏低";
                } else if (celsius < 35.0) {
                    tempStatus = "正常";
                } else if (celsius < 45.0) {
                    tempStatus = "偏高";
                } else {
                    tempStatus = "过高";
                }
                std::cout << "温度状态: " << tempStatus << std::endl;
            }
            
        } else {
            std::cout << "类型: 外部电源或非内置电池" << std::endl;
        }
    }
    
    CFRelease(powerSources);
    CFRelease(powerSourceInfo);
    std::cout << "✓ 专业电池信息测试完成" << std::endl;
}

int main() {
    std::cout << "=== macOS 专业电池信息检测测试 ===" << std::endl;
    std::cout << "测试详细的电池状态和健康信息" << std::endl;
    
    try {
        testProfessionalBatteryInfo();
        std::cout << "\n=== 所有测试完成 ===" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}