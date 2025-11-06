#include <iostream>
#include <iomanip>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

// 电池信息测试
void testBatteryInfo() {
    std::cout << "\n=== 电池信息测试 ===" << std::endl;
    
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
        
        // 检查是否为电池
        CFBooleanRef isBattery = (CFBooleanRef)CFDictionaryGetValue(description, CFSTR(kIOPSIsPresentKey));
        if (isBattery && CFBooleanGetValue(isBattery)) {
            std::cout << "类型: 电池" << std::endl;
            
            // 获取电池电量
            CFNumberRef capacity = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSCurrentCapacityKey));
            if (capacity) {
                int currentCapacity = 0;
                CFNumberGetValue(capacity, kCFNumberIntType, &currentCapacity);
                std::cout << "当前容量: " << currentCapacity << " (单位可能是mAh或百分比)" << std::endl;
                
                // 获取最大容量
                CFNumberRef maxCapacity = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSMaxCapacityKey));
                int maxCapValue = 0;
                int designCapValue = 0;
                if (maxCapacity) {
                    CFNumberGetValue(maxCapacity, kCFNumberIntType, &maxCapValue);
                    std::cout << "最大容量: " << maxCapValue << " (单位可能是mAh或参考值)" << std::endl;
                    
                    if (maxCapValue > 0) {
                        double percentage = (double)currentCapacity / maxCapValue * 100;
                        std::cout << "电量百分比: " << std::fixed << std::setprecision(1) 
                                  << percentage << "%" << std::endl;
                    }
                }
                
                // 获取设计容量
                CFNumberRef designCapacity = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSDesignCapacityKey));
                if (designCapacity) {
                    CFNumberGetValue(designCapacity, kCFNumberIntType, &designCapValue);
                    std::cout << "设计容量: " << designCapValue << " mAh" << std::endl;
                    
                    if (designCapValue > 0 && maxCapValue > 0) {
                        double health = (double)maxCapValue / designCapValue * 100;
                        std::cout << "电池健康度: " << std::fixed << std::setprecision(1) 
                                  << health << "%" << std::endl;
                        
                        // 健康状态评估
                        std::string healthStatus;
                        if (health >= 80.0) {
                            healthStatus = "优秀";
                        } else if (health >= 60.0) {
                            healthStatus = "良好";
                        } else if (health >= 40.0) {
                            healthStatus = "一般";
                        } else {
                            healthStatus = "需要更换";
                        }
                        std::cout << "健康状态: " << healthStatus << std::endl;
                    }
                }
            }
            
            // 获取充电状态
            CFBooleanRef isCharging = (CFBooleanRef)CFDictionaryGetValue(description, CFSTR(kIOPSIsChargingKey));
            if (isCharging) {
                std::cout << "充电状态: " << (CFBooleanGetValue(isCharging) ? "充电中" : "未充电") << std::endl;
            }
            
            // 获取电源状态
            CFStringRef powerState = (CFStringRef)CFDictionaryGetValue(description, CFSTR(kIOPSPowerSourceStateKey));
            if (powerState) {
                char state[256];
                CFStringGetCString(powerState, state, sizeof(state), kCFStringEncodingUTF8);
                std::cout << "电源状态: " << state << std::endl;
            }
            
            // 获取时间到充满
            CFNumberRef timeToFull = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSTimeToFullChargeKey));
            if (timeToFull) {
                int minutes = 0;
                CFNumberGetValue(timeToFull, kCFNumberIntType, &minutes);
                if (minutes > 0) {
                    std::cout << "充满时间: " << (minutes / 60) << " 小时 " << (minutes % 60) << " 分钟" << std::endl;
                }
            }
            
            // 获取时间到空
            CFNumberRef timeToEmpty = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSTimeToEmptyKey));
            if (timeToEmpty) {
                int minutes = 0;
                CFNumberGetValue(timeToEmpty, kCFNumberIntType, &minutes);
                if (minutes > 0 && minutes != 999) { // 999 表示正在充电
                    std::cout << "剩余时间: " << (minutes / 60) << " 小时 " << (minutes % 60) << " 分钟" << std::endl;
                }
            }
            
            // 获取循环次数
            CFNumberRef cycleCount = (CFNumberRef)CFDictionaryGetValue(description, CFSTR("CycleCount"));
            if (cycleCount) {
                int cycles = 0;
                CFNumberGetValue(cycleCount, kCFNumberIntType, &cycles);
                std::cout << "循环次数: " << cycles << std::endl;
            }
            
            // 获取温度
            CFNumberRef temperature = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSTemperatureKey));
            if (temperature) {
                float temp = 0;
                CFNumberGetValue(temperature, kCFNumberFloatType, &temp);
                std::cout << "电池温度: " << std::fixed << std::setprecision(1) << temp << "°C" << std::endl;
            }
            
            // 获取电压
            CFNumberRef voltage = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSVoltageKey));
            if (voltage) {
                float volt = 0;
                CFNumberGetValue(voltage, kCFNumberFloatType, &volt);
                std::cout << "电压: " << std::fixed << std::setprecision(2) << volt << " V" << std::endl;
            }
            
            // 获取电流
            CFNumberRef current = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSCurrentKey));
            if (current) {
                int cur = 0;
                CFNumberGetValue(current, kCFNumberIntType, &cur);
                std::cout << "电流: " << cur << " mA" << std::endl;
                
                // 计算功率 (电压 × 电流 / 1000)
                CFNumberRef voltage = (CFNumberRef)CFDictionaryGetValue(description, CFSTR(kIOPSVoltageKey));
                if (voltage) {
                    float volt = 0;
                    CFNumberGetValue(voltage, kCFNumberFloatType, &volt);
                    double power = (volt * std::abs(cur)) / 1000.0; // V * mA = mW, 转换为W
                    std::cout << "功率: " << std::fixed << std::setprecision(2) << power << " W" << std::endl;
                }
            }
        } else {
            std::cout << "类型: 非电池电源 (可能是电源适配器)" << std::endl;
        }
    }
    
    CFRelease(powerSources);
    CFRelease(powerSourceInfo);
    std::cout << "✓ 电池信息测试完成" << std::endl;
}

int main() {
    std::cout << "=== macOS 电池信息检测测试 ===" << std::endl;
    std::cout << "测试电池状态和充电信息获取功能" << std::endl;
    
    try {
        testBatteryInfo();
        std::cout << "\n=== 所有测试完成 ===" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}