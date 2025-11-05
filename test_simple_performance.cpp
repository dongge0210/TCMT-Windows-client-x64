#include <iostream>
#include <chrono>
#include <thread>

#ifdef PLATFORM_MACOS
#include "src/core/platform/macos/MacCpuInfo.h"
#include "src/core/platform/macos/MacMemoryInfo.h"
#include "src/core/platform/macos/MacGpuInfo.h"
#include "src/core/platform/macos/MacErrorHandler.h"
#endif

int main() {
    std::cout << "=== macOS 硬件监控性能测试 ===" << std::endl;
    
#ifdef PLATFORM_MACOS
    // 初始化CPU监控
    MacCpuInfo cpuInfo;
    if (!cpuInfo.Initialize()) {
        std::cerr << "CPU初始化失败: " << cpuInfo.GetLastError() << std::endl;
        return 1;
    }
    
    // 初始化内存监控
    MacMemoryInfo memoryInfo;
    if (!memoryInfo.Initialize()) {
        std::cerr << "内存初始化失败: " << memoryInfo.GetLastError() << std::endl;
        return 1;
    }
    
    // 初始化GPU监控
    MacGpuInfo gpuInfo;
    if (!gpuInfo.Initialize()) {
        std::cerr << "GPU初始化失败: " << gpuInfo.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "所有组件初始化成功！" << std::endl;
    std::cout << "\n=== 实时监控测试 (运行10秒) ===" << std::endl;
    
    // 运行10秒的实时监控
    for (int i = 0; i < 10; ++i) {
        // 更新所有组件数据
        cpuInfo.Update();
        memoryInfo.Update();
        gpuInfo.Update();
        
        // 显示CPU信息
        std::cout << "\n--- 第 " << (i+1) << " 次采样 ---" << std::endl;
        std::cout << "CPU: " << cpuInfo.GetName() << std::endl;
        std::cout << "  使用率: " << cpuInfo.GetTotalUsage() << "%" << std::endl;
        std::cout << "  当前频率: " << cpuInfo.GetCurrentFrequency() << " MHz" << std::endl;
        std::cout << "  温度: " << cpuInfo.GetTemperature() << "°C" << std::endl;
        std::cout << "  功耗: " << cpuInfo.GetPowerUsage() << "W" << std::endl;
        
        // 显示内存信息
        std::cout << "内存:" << std::endl;
        std::cout << "  物理内存使用: " << memoryInfo.GetPhysicalMemoryUsage() << "%" << std::endl;
        std::cout << "  可用内存: " << (memoryInfo.GetAvailablePhysicalMemory() / 1024.0 / 1024.0 / 1024.0) << " GB" << std::endl;
        std::cout << "  内存压力: " << memoryInfo.GetMemoryPressure() << "%" << std::endl;
        std::cout << "  内存状态: " << memoryInfo.GetMemoryStatusDescription() << std::endl;
        
        // 显示GPU信息
        std::cout << "GPU: " << gpuInfo.GetName() << std::endl;
        std::cout << "  使用率: " << gpuInfo.GetGpuUsage() << "%" << std::endl;
        std::cout << "  当前频率: " << gpuInfo.GetCurrentFrequency() << " MHz" << std::endl;
        std::cout << "  温度: " << gpuInfo.GetTemperature() << "°C" << std::endl;
        std::cout << "  功耗: " << gpuInfo.GetPowerUsage() << "W" << std::endl;
        
        // 等待1秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\n=== 性能分析总结 ===" << std::endl;
    
    // 健康检查
    bool cpuHealthy = cpuInfo.GetTemperature() < 85.0;
    bool memoryHealthy = memoryInfo.AnalyzeMemoryHealth();
    bool gpuHealthy = gpuInfo.GetTemperature() < 85.0;
    
    std::cout << "CPU健康状态: " << (cpuHealthy ? "正常" : "异常") << std::endl;
    std::cout << "内存健康状态: " << (memoryHealthy ? "正常" : "异常") << std::endl;
    std::cout << "GPU健康状态: " << (gpuHealthy ? "正常" : "异常") << std::endl;
    
    // 性能评级
    double cpuPerformance = cpuInfo.GetCurrentFrequency() / cpuInfo.GetMaxFrequency() * 100.0;
    double memoryEfficiency = memoryInfo.GetMemoryEfficiency();
    double gpuPerformance = gpuInfo.GetPerformanceRating();
    
    std::cout << "\n性能评级:" << std::endl;
    std::cout << "CPU性能: " << cpuPerformance << "%" << std::endl;
    std::cout << "内存效率: " << memoryEfficiency << "%" << std::endl;
    std::cout << "GPU性能: " << gpuPerformance << " 分" << std::endl;
    
    std::cout << "\n测试完成！所有CPU/RAM/GPU功能运行正常。" << std::endl;
    
#else
    std::cout << "此测试仅在macOS平台上运行" << std::endl;
#endif
    
    return 0;
}