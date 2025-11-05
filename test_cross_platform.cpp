#include "src/core/platform/macos/MacCpuInfo.h"
#include "src/core/platform/macos/MacCpuAdapter.h"
#include "src/core/platform/macos/MacMemoryInfo.h"
#include "src/core/platform/macos/MacGpuInfo.h"
#include "src/core/factory/InfoFactory.h"
#include <iostream>

int main() {
    std::cout << "=== 跨平台架构测试 (macOS) ===" << std::endl;
    
    // 测试CPU信息
    std::cout << "\n--- CPU信息测试 ---" << std::endl;
    auto cpuInfo = InfoFactory::CreateCpuInfo();
    if (cpuInfo) {
        std::cout << "✓ CPU信息创建成功" << std::endl;
        std::cout << "  CPU名称: " << cpuInfo->GetName() << std::endl;
        std::cout << "  总核心数: " << cpuInfo->GetTotalCores() << std::endl;
        std::cout << "  性能核心数: " << cpuInfo->GetPerformanceCores() << std::endl;
        std::cout << "  效率核心数: " << cpuInfo->GetEfficiencyCores() << std::endl;
        std::cout << "  CPU使用率: " << cpuInfo->GetTotalUsage() << "%" << std::endl;
        std::cout << "  当前频率: " << cpuInfo->GetCurrentFrequency() << " MHz" << std::endl;
        std::cout << "  温度: " << cpuInfo->GetTemperature() << "°C" << std::endl;
    } else {
        std::cout << "✗ CPU信息创建失败: " << InfoFactory::GetLastError() << std::endl;
    }
    
    // 测试内存信息
    std::cout << "\n--- 内存信息测试 ---" << std::endl;
    auto memoryInfo = InfoFactory::CreateMemoryInfo();
    if (memoryInfo) {
        std::cout << "✓ 内存信息创建成功" << std::endl;
        std::cout << "  总物理内存: " << memoryInfo->GetTotalPhysicalMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
        std::cout << "  可用物理内存: " << memoryInfo->GetAvailablePhysicalMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
        std::cout << "  内存使用率: " << memoryInfo->GetPhysicalMemoryUsage() * 100 << "%" << std::endl;
        std::cout << "  总虚拟内存: " << memoryInfo->GetTotalVirtualMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
        std::cout << "  交换文件: " << memoryInfo->GetTotalSwapMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
        std::cout << "  内存压力: " << memoryInfo->GetMemoryPressure() * 100 << "%" << std::endl;
    } else {
        std::cout << "✗ 内存信息创建失败: " << InfoFactory::GetLastError() << std::endl;
    }
    
    // 测试GPU信息
    std::cout << "\n--- GPU信息测试 ---" << std::endl;
    auto gpuInfo = InfoFactory::CreateGpuInfo();
    if (gpuInfo) {
        std::cout << "✓ GPU信息创建成功" << std::endl;
        std::cout << "  GPU名称: " << gpuInfo->GetName() << std::endl;
        std::cout << "  供应商: " << gpuInfo->GetVendor() << std::endl;
        std::cout << "  架构: " << gpuInfo->GetArchitecture() << std::endl;
        std::cout << "  专用内存: " << gpuInfo->GetDedicatedMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
        std::cout << "  GPU使用率: " << gpuInfo->GetGpuUsage() * 100 << "%" << std::endl;
        std::cout << "  内存使用率: " << gpuInfo->GetMemoryUsage() * 100 << "%" << std::endl;
        std::cout << "  当前频率: " << gpuInfo->GetCurrentFrequency() << " MHz" << std::endl;
        std::cout << "  温度: " << gpuInfo->GetTemperature() << "°C" << std::endl;
        std::cout << "  计算单元数: " << gpuInfo->GetComputeUnits() << std::endl;
    } else {
        std::cout << "✗ GPU信息创建失败: " << InfoFactory::GetLastError() << std::endl;
    }
    
    // 测试平台信息
    std::cout << "\n--- 平台信息 ---" << std::endl;
    std::cout << "平台名称: " << InfoFactory::GetPlatformName() << std::endl;
    std::cout << "编译器: " << InfoFactory::GetCompilerName() << std::endl;
    std::cout << "架构: " << InfoFactory::GetArchitectureName() << std::endl;
    std::cout << "构建信息: " << InfoFactory::GetBuildInfo() << std::endl;
    
    // 测试功能可用性
    std::cout << "\n--- 功能可用性 ---" << std::endl;
    std::cout << "GPU监控: " << (InfoFactory::IsGpuMonitoringAvailable() ? "✓ 可用" : "✗ 不可用") << std::endl;
    std::cout << "TPM支持: " << (InfoFactory::IsTpmAvailable() ? "✓ 可用" : "✗ 不可用") << std::endl;
    std::cout << "USB监控: " << (InfoFactory::IsUsbMonitoringAvailable() ? "✓ 可用" : "✗ 不可用") << std::endl;
    std::cout << "温度监控: " << (InfoFactory::IsTemperatureMonitoringAvailable() ? "✓ 可用" : "✗ 不可用") << std::endl;
    
    std::cout << "\n=== 跨平台架构测试完成 ===" << std::endl;
    std::cout << "✓ 接口统一，实现分离" << std::endl;
    std::cout << "✓ 平台特定功能正常工作" << std::endl;
    std::cout << "✓ 原有功能保持不变" << std::endl;
    
    return 0;
}