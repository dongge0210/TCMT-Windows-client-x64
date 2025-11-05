#include "CpuInfoCompat.h"
#include "Logger.h"
#include <iostream>
#include <thread>
#include <chrono>

/*
 * CPU信息跨平台兼容性演示
 * 
 * 这个文件展示了如何使用CpuInfoCompat类来实现：
 * 1. 完全向后兼容的原有功能
 * 2. 无缝切换到跨平台实现
 * 3. 错误处理和回退机制
 */

void DemonstrateLegacyMode() {
    std::cout << "\n=== 演示原有模式 (Legacy Mode) ===" << std::endl;
    
    // 使用原有接口，完全兼容
    CpuInfoCompat cpuInfo(CpuInfoCompat::ImplementationMode::Legacy);
    
    std::cout << "CPU名称: " << cpuInfo.GetName() << std::endl;
    std::cout << "总核心数: " << cpuInfo.GetTotalCores() << std::endl;
    std::cout << "性能核心: " << cpuInfo.GetLargeCores() << std::endl;
    std::cout << "能效核心: " << cpuInfo.GetSmallCores() << std::endl;
    std::cout << "CPU使用率: " << cpuInfo.GetUsage() << "%" << std::endl;
    std::cout << "当前频率: " << cpuInfo.GetCurrentSpeed() << " MHz" << std::endl;
    std::cout << "基础频率: " << cpuInfo.GetBaseFrequencyMHz() << " MHz" << std::endl;
    std::cout << "超线程: " << (cpuInfo.IsHyperThreadingEnabled() ? "启用" : "禁用") << std::endl;
    std::cout << "虚拟化: " << (cpuInfo.IsVirtualizationEnabled() ? "支持" : "不支持") << std::endl;
    
    std::cout << "当前模式: " << (cpuInfo.GetCurrentMode() == CpuInfoCompat::ImplementationMode::Legacy ? "Legacy" : "Adapter") << std::endl;
}

void DemonstrateAdapterMode() {
    std::cout << "\n=== 演示跨平台模式 (Adapter Mode) ===" << std::endl;
    
    // 检查适配器是否可用
    CpuInfoCompat testCpu;
    if (!testCpu.IsAdapterAvailable()) {
        std::cout << "当前平台不支持适配器模式" << std::endl;
        return;
    }
    
    // 使用跨平台接口
    CpuInfoCompat cpuInfo(CpuInfoCompat::ImplementationMode::Adapter);
    
    if (!cpuInfo.GetLastError().empty()) {
        std::cout << "错误: " << cpuInfo.GetLastError() << std::endl;
        return;
    }
    
    std::cout << "CPU名称: " << cpuInfo.GetName() << std::endl;
    std::cout << "总核心数: " << cpuInfo.GetTotalCpu() << std::endl;
    std::cout << "性能核心: " << cpuInfo.GetLargeCores() << std::endl;
    std::cout << "能效核心: " << cpuInfo.GetSmallCores() << std::endl;
    std::cout << "CPU使用率: " << cpuInfo.GetUsage() << "%" << std::endl;
    std::cout << "当前频率: " << cpuInfo.GetCurrentFrequencyMHz() << " MHz" << std::endl;
    std::cout << "基础频率: " << cpuInfo.GetBaseFrequencyMHz() << " MHz" << std::endl;
    std::cout << "超线程: " << (cpuInfo.IsHyperThreadingEnabled() ? "启用" : "禁用") << std::endl;
    std::cout << "虚拟化: " << (cpuInfo.IsVirtualizationEnabled() ? "支持" : "不支持") << std::endl;
    
    std::cout << "当前模式: " << (cpuInfo.GetCurrentMode() == CpuInfoCompat::ImplementationMode::Legacy ? "Legacy" : "Adapter") << std::endl;
}

void DemonstrateRuntimeSwitching() {
    std::cout << "\n=== 演示运行时切换 ===" << std::endl;
    
    CpuInfoCompat cpuInfo(CpuInfoCompat::ImplementationMode::Legacy);
    
    std::cout << "初始模式: " << (cpuInfo.GetCurrentMode() == CpuInfoCompat::ImplementationMode::Legacy ? "Legacy" : "Adapter") << std::endl;
    std::cout << "CPU使用率: " << cpuInfo.GetUsage() << "%" << std::endl;
    
    // 尝试切换到适配器模式
    if (cpuInfo.IsAdapterAvailable()) {
        std::cout << "\n切换到适配器模式..." << std::endl;
        if (cpuInfo.SwitchToAdapter()) {
            std::cout << "切换成功!" << std::endl;
            std::cout << "CPU使用率: " << cpuInfo.GetUsage() << "%" << std::endl;
        } else {
            std::cout << "切换失败: " << cpuInfo.GetLastError() << std::endl;
        }
    } else {
        std::cout << "\n当前平台不支持适配器模式" << std::endl;
    }
    
    // 切换回原有模式
    std::cout << "\n切换回原有模式..." << std::endl;
    if (cpuInfo.SwitchToLegacy()) {
        std::cout << "切换成功!" << std::endl;
        std::cout << "CPU使用率: " << cpuInfo.GetUsage() << "%" << std::endl;
    } else {
        std::cout << "切换失败: " << cpuInfo.GetLastError() << std::endl;
    }
}

void DemonstratePerformanceMonitoring() {
    std::cout << "\n=== 演示性能监控 ===" << std::endl;
    
    CpuInfoCompat cpuInfo;
    
    // 监控CPU使用率变化
    std::cout << "监控CPU使用率5秒..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        double usage = cpuInfo.GetUsage();
        std::cout << "第" << (i + 1) << "秒: " << usage << "%" << std::endl;
    }
    
    // 显示采样间隔
    std::cout << "采样间隔: " << cpuInfo.GetLastSampleIntervalMs() << " ms" << std::endl;
}

void DemonstrateErrorHandling() {
    std::cout << "\n=== 演示错误处理 ===" << std::endl;
    
    // 创建一个无效的适配器（如果平台不支持）
    CpuInfoCompat cpuInfo(CpuInfoCompat::ImplementationMode::Adapter);
    
    if (!cpuInfo.GetLastError().empty()) {
        std::cout << "预期的错误: " << cpuInfo.GetLastError() << std::endl;
        std::cout << "自动回退到: " << (cpuInfo.GetCurrentMode() == CpuInfoCompat::ImplementationMode::Legacy ? "Legacy" : "Adapter") << std::endl;
        std::cout << "CPU使用率: " << cpuInfo.GetUsage() << "%" << std::endl;
    }
    
    // 清除错误
    cpuInfo.ClearError();
    std::cout << "错误已清除" << std::endl;
}

int main() {
    std::cout << "CPU信息跨平台兼容性演示" << std::endl;
    std::cout << "========================" << std::endl;
    
    try {
        // 演示各种使用场景
        DemonstrateLegacyMode();
        DemonstrateAdapterMode();
        DemonstrateRuntimeSwitching();
        DemonstratePerformanceMonitoring();
        DemonstrateErrorHandling();
        
        std::cout << "\n演示完成!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}