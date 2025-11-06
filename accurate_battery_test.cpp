#include <iostream>
#include <iomanip>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

// 电池信息测试 - 基于实际MacBook Air M2可用的键
void testAccurateBatteryInfo() {
    std::cout << "\n=== MacBook Air M2 电池信息测试 ===" << std::endl;
    
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
        CFStringRef psType = (CFStringRef)CFDictionaryGetValue(description, CFSTR("Type"));
        if (psType) {
            char typeStr[256];
            CFStringGetCString(psType, typeStr, sizeof(typeStr), kCFStringEncodingUTF8);
            std::cout << "类型: " << typeStr << std::endl;
            
            if (std::string(typeStr) == "InternalBattery") {
                std::cout << "✓ 检测到电池设备" << std::endl;
                
                // 获取电池名称
                CFStringRef name = (CFStringRef)CFDictionaryGetValue(description, CFSTR("Name"));
                if (name) {
                    char nameStr[256];
                    CFStringGetCString(name, nameStr, sizeof(nameStr), kCFStringEncodingUTF8);
                    std::cout << "电池名称: " << nameStr << std::endl;
                }
                
                // 获取当前容量百分比
                CFNumberRef currentCap = (CFNumberRef)CFDictionaryGetValue(description, CFSTR("Current"));
                if (currentCap) {
                    int currentValue = 0;
                    CFNumberGetValue(currentCap, kCFNumberIntType, &currentValue);
                    std::cout << "当前容量: " << currentValue << "%" << std::endl;
                }
                
                // 获取电池健康度
                CFStringRef health = (CFStringRef)CFDictionaryGetValue(description, CFSTR("BatteryHealth"));
                if (health) {
                    char healthStr[256];
                    CFStringGetCString(health, healthStr, sizeof(healthStr), kCFStringEncodingUTF8);
                    std::cout << "电池健康: " << healthStr << std::endl;
                    
                    // 尝试解析健康百分比
                    std::string healthStrStr(healthStr);
                    if (healthStrStr.find("Good") != std::string::npos) {
                        std::cout << "健康状态: 良好 (约85-100%)" << std::endl;
                    } else if (healthStrStr.find("Fair") != std::string::npos) {
                        std::cout << "健康状态: 一般 (约60-84%)" << std::endl;
                    } else if (healthStrStr.find("Poor") != std::string::npos) {
                        std::cout << "健康状态: 较差 (低于60%)" << std::endl;
                    }
                }
                
                // 尝试获取其他可能的键
                CFStringRef keys[] = {
                    CFSTR("Voltage"),
                    CFSTR("Current"),
                    CFSTR("Power"),
                    CFSTR("Temperature"),
                    CFSTR("CycleCount"),
                    CFSTR("IsCharging"),
                    CFSTR("TimeRemaining"),
                    CFSTR("Max Capacity"),
                    CFSTR("DesignCapacity"),
                    CFSTR("InstantAmperage"),
                    CFSTR("Current Capacity"),
                    NULL
                };
                
                const char* keyNames[] = {
                    "电压",
                    "电流",
                    "功率",
                    "温度",
                    "循环次数",
                    "充电状态",
                    "剩余时间",
                    "最大容量",
                    "设计容量",
                    "瞬时电流",
                    "当前容量",
                    NULL
                };
                
                for (int j = 0; keys[j] != NULL; ++j) {
                    CFTypeRef value = CFDictionaryGetValue(description, keys[j]);
                    if (value) {
                        if (CFGetTypeID(value) == CFNumberGetTypeID()) {
                            double numValue = 0;
                            CFNumberGetValue((CFNumberRef)value, kCFNumberDoubleType, &numValue);
                            
                            if (j == 0) { // 电压
                                std::cout << keyNames[j] << ": " << std::fixed << std::setprecision(2) 
                                          << numValue << " V" << std::endl;
                            } else if (j == 1) { // 电流
                                std::cout << keyNames[j] << ": " << std::fixed << std::setprecision(0) 
                                          << numValue << " mA" << std::endl;
                            } else if (j == 2) { // 功率
                                std::cout << keyNames[j] << ": " << std::fixed << std::setprecision(2) 
                                          << numValue << " W" << std::endl;
                            } else if (j == 3) { // 温度
                                std::cout << keyNames[j] << ": " << std::fixed << std::setprecision(1) 
                                          << numValue << "°C" << std::endl;
                            } else if (j == 7) { // 最大容量
                                std::cout << keyNames[j] << ": " << (int)numValue << " mAh" << std::endl;
                            } else if (j == 8) { // 设计容量
                                std::cout << keyNames[j] << ": " << (int)numValue << " mAh" << std::endl;
                            } else if (j == 9) { // 瞬时电流
                                std::cout << keyNames[j] << ": " << std::fixed << std::setprecision(0) 
                                          << numValue << " mA" << std::endl;
                            } else {
                                std::cout << keyNames[j] << ": " << (int)numValue << std::endl;
                            }
                        } else if (CFGetTypeID(value) == CFStringGetTypeID()) {
                            char strValue[256];
                            CFStringGetCString((CFStringRef)value, strValue, sizeof(strValue), kCFStringEncodingUTF8);
                            std::cout << keyNames[j] << ": " << strValue << std::endl;
                        } else if (CFGetTypeID(value) == CFBooleanGetTypeID()) {
                            bool boolValue = CFBooleanGetValue((CFBooleanRef)value);
                            std::cout << keyNames[j] << ": " << (boolValue ? "是" : "否") << std::endl;
                        }
                    }
                }
                
                // 计算功率（如果有电压和电流）
                CFNumberRef voltage = (CFNumberRef)CFDictionaryGetValue(description, CFSTR("Voltage"));
                CFNumberRef currentVal = (CFNumberRef)CFDictionaryGetValue(description, CFSTR("InstantAmperage"));
                
                if (voltage && currentVal) {
                    double volt = 0, curr = 0;
                    CFNumberGetValue(voltage, kCFNumberDoubleType, &volt);
                    CFNumberGetValue(currentVal, kCFNumberDoubleType, &curr);
                    
                    double power = (volt * std::abs(curr)) / 1000.0; // V * mA = mW, 转换为W
                    std::cout << "计算功率: " << std::fixed << std::setprecision(3) 
                              << power << " W" << std::endl;
                              
                    // 显示功率状态
                    if (curr < 0) {
                        std::cout << "状态: 放电中 (消耗功率)" << std::endl;
                    } else if (curr > 0) {
                        std::cout << "状态: 充电中 (输入功率)" << std::endl;
                    } else {
                        std::cout << "状态: 满电/未使用" << std::endl;
                    }
                }
                
                // 电池健康度估算（如果有设计容量和最大容量）
                CFNumberRef maxCap = (CFNumberRef)CFDictionaryGetValue(description, CFSTR("Max Capacity"));
                CFNumberRef designCap = (CFNumberRef)CFDictionaryGetValue(description, CFSTR("DesignCapacity"));
                
                if (maxCap && designCap) {
                    double maxVal = 0, designVal = 0;
                    CFNumberGetValue(maxCap, kCFNumberDoubleType, &maxVal);
                    CFNumberGetValue(designCap, kCFNumberDoubleType, &designVal);
                    
                    double healthPercent = (maxVal / designVal) * 100.0;
                    std::cout << "电池健康度: " << std::fixed << std::setprecision(1) 
                              << healthPercent << "%" << std::endl;
                              
                    if (healthPercent >= 80.0) {
                        std::cout << "健康评级: 优秀" << std::endl;
                    } else if (healthPercent >= 60.0) {
                        std::cout << "健康评级: 良好" << std::endl;
                    } else if (healthPercent >= 40.0) {
                        std::cout << "健康评级: 一般" << std::endl;
                    } else {
                        std::cout << "健康评级: 较差 (建议更换)" << std::endl;
                    }
                }
            } else {
                std::cout << "非电池电源" << std::endl;
            }
        }
        
        // 显示所有可用的键（调试用）
        std::cout << "\n--- 可用键列表 ---" << std::endl;
        CFIndex keyCount = CFDictionaryGetCount(description);
        if (keyCount > 0) {
            std::vector<CFStringRef> keys(keyCount);
            std::vector<CFTypeRef> values(keyCount);
            CFDictionaryGetKeysAndValues(description, (const void**)keys.data(), (const void**)values.data());
            
            for (CFIndex j = 0; j < keyCount; ++j) {
                char keyStr[256];
                CFStringGetCString(keys[j], keyStr, sizeof(keyStr), kCFStringEncodingUTF8);
                std::cout << "键: " << keyStr;
                
                // 显示值类型
                CFTypeRef value = values[j];
                if (value) {
                    if (CFGetTypeID(value) == CFNumberGetTypeID()) {
                        double numValue = 0;
                        CFNumberGetValue((CFNumberRef)value, kCFNumberDoubleType, &numValue);
                        std::cout << " (数值: " << numValue << ")";
                    } else if (CFGetTypeID(value) == CFStringGetTypeID()) {
                        char strValue[256];
                        CFStringGetCString((CFStringRef)value, strValue, sizeof(strValue), kCFStringEncodingUTF8);
                        std::cout << " (字符串: " << strValue << ")";
                    } else if (CFGetTypeID(value) == CFBooleanGetTypeID()) {
                        bool boolValue = CFBooleanGetValue((CFBooleanRef)value);
                        std::cout << " (布尔: " << (boolValue ? "true" : "false") << ")";
                    } else {
                        std::cout << " (其他类型)";
                    }
                }
                std::cout << std::endl;
            }
        }
    }
    
    CFRelease(powerSources);
    CFRelease(powerSourceInfo);
    std::cout << "✓ 准确电池信息测试完成" << std::endl;
}

int main() {
    std::cout << "=== Macbook Air M2 准确电池信息测试 ===" << std::endl;
    std::cout << "基于实际可用的IOKit键进行精确检测" << std::endl;
    
    try {
        testAccurateBatteryInfo();
        std::cout << "\n=== 测试完成 ===" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}