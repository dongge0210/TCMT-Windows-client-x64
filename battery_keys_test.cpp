#include <iostream>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

// 探测可用的电池数据键
void exploreBatteryKeys() {
    std::cout << "\n=== 探测电池数据键 ===" << std::endl;
    
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
        
        // 获取一些常见的键
        const char* commonKeys[] = {
            "Name", "Type", "CurrentCapacity", "MaxCapacity", "DesignCapacity",
            "CycleCount", "IsCharging", "IsPresent", "Voltage", "Current",
            "TimeToFullCharge", "TimeToEmpty", "Temperature", "PowerSourceState",
            "BatteryHealth", "InstantAmperage", "Voltage", NULL
        };
        
        for (int k = 0; commonKeys[k] != NULL; k++) {
            CFStringRef keyName = CFStringCreateWithCString(kCFAllocatorDefault, commonKeys[k], kCFStringEncodingUTF8);
            CFTypeRef value = CFDictionaryGetValue(description, keyName);
            
            if (value) {
                std::cout << "键: " << commonKeys[k] << " - ";
                
                // 根据值类型显示信息
                CFTypeID typeID = CFGetTypeID(value);
                if (typeID == CFStringGetTypeID()) {
                    CFStringRef valueStr = (CFStringRef)value;
                    char valueContent[256];
                    if (CFStringGetCString(valueStr, valueContent, sizeof(valueContent), kCFStringEncodingUTF8)) {
                        std::cout << "字符串值: " << valueContent;
                    }
                } else if (typeID == CFNumberGetTypeID()) {
                    CFNumberRef number = (CFNumberRef)value;
                    if (CFNumberIsFloatType(number)) {
                        float floatVal;
                        CFNumberGetValue(number, kCFNumberFloatType, &floatVal);
                        std::cout << "浮点数: " << floatVal;
                    } else {
                        int intVal;
                        CFNumberGetValue(number, kCFNumberIntType, &intVal);
                        std::cout << "整数: " << intVal;
                    }
                } else if (typeID == CFBooleanGetTypeID()) {
                    CFBooleanRef boolVal = (CFBooleanRef)value;
                    std::cout << "布尔值: " << (CFBooleanGetValue(boolVal) ? "true" : "false");
                } else {
                    std::cout << "其他类型: " << typeID;
                }
                std::cout << std::endl;
            }
            CFRelease(keyName);
        }
    }
    
    CFRelease(powerSources);
    CFRelease(powerSourceInfo);
    std::cout << "✓ 电池键探测完成" << std::endl;
}

int main() {
    std::cout << "=== macOS 电池数据键探测 ===" << std::endl;
    std::cout << "探测实际可用的电池数据键" << std::endl;
    
    try {
        exploreBatteryKeys();
        std::cout << "\n=== 探测完成 ===" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "探测过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}