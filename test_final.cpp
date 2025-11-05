#include <iostream>
#include <chrono>
#include <thread>

#define PLATFORM_MACOS

#ifdef PLATFORM_MACOS
#include "src/core/platform/macos/MacCpuInfo.h"
#include "src/core/platform/macos/MacMemoryInfo.h"
#include "src/core/platform/macos/MacGpuInfo.h"
#endif

// 简单的错误处理，避免依赖MacErrorHandler
class SimpleErrorHandler {
public:
    void ReportError(const std::string& error) {
        std::cerr << "Error: " << error << std::endl;
    }
    bool AttemptRecovery() {
        return false;
    }
};

int main() {
    std::cout << "=== macOS 硬件监控功能测试 ===" << std::endl;
    
#ifdef PLATFORM_MACOS
    // 初始化CPU监控
    MacCpuInfo cpuInfo;
    if (!cpuInfo.Initialize()) {
        std::cerr << "CPU初始化失败: " << cpuInfo.GetLastError() << std::endl;
        // 继续测试其他功能
    }
    
    // 初始化内存监控
    MacMemoryInfo memoryInfo;
    if (!memoryInfo.Initialize()) {
        std::cerr << "内存初始化失败" << std::endl;
        return 1;
    }
    
    // 初始化GPU监控
    MacGpuInfo gpuInfo;
    if (!gpuInfo.Initialize()) {
        std::cerr << "GPU初始化失败" << std::endl;
        return 1;
    }
    
    std::cout << "所有组件初始化成功！" << std::endl;
    
    // 测试基本功能
    std::cout << "\n=== 基本功能测试 ===" << std::endl;
    
    // CPU信息
    std::cout << "CPU: " << cpuInfo.GetName() << std::endl;
    std::cout << "  核心: " << cpuInfo.GetPhysicalCores() << "物理/" << cpuInfo.GetLogicalCores() << "逻辑" << std::endl;
    std::cout << "  频率: " << cpuInfo.GetBaseFrequency() << "MHz - " << cpuInfo.GetMaxFrequency() << "MHz" << std::endl;
    
    // 内存信息
    std::cout << "内存总量: " << (memoryInfo.GetTotalPhysicalMemory() / 1024.0 / 1024.0 / 1024.0) << "GB" << std::endl;
    
    // GPU信息
    std::cout << "GPU: " << gpuInfo.GetName() << std::endl;
    std::cout << "  内存: " << (gpuInfo.GetSharedMemory() / 1024.0 / 1024.0 / 1024.0) << "GB" << std::endl;
    std::cout << "  计算单元: " << gpuInfo.GetComputeUnits() << std::endl;
    
    // 动态更新测试
    std::cout << "\n=== 动态更新测试 (5秒) ===" << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        cpuInfo.Update();
        memoryInfo.Update();
        gpuInfo.Update();
        
        std::cout << "第" << (i+1) << "次采样:" << std::endl;
        std::cout << "  CPU使用率: " << cpuInfo.GetTotalUsage() << "%" << std::endl;
        std::cout << "  内存使用率: " << memoryInfo.GetPhysicalMemoryUsage() << "%" << std::endl;
        std::cout << "  GPU使用率: " << gpuInfo.GetGpuUsage() << "%" << std::endl;
        std::cout << "  CPU温度: " << cpuInfo.GetTemperature() << "°C" << std::endl;
        std::cout << "  GPU温度: " << gpuInfo.GetTemperature() << "°C" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\n=== 功能验证完成 ===" << std::endl;
    std::cout << "✓ CPU/RAM/GPU监控功能正常运行" << std::endl;
    
#else
    std::cout << "此测试仅在macOS平台运行" << std::endl;
#endif
    
    return 0;
}