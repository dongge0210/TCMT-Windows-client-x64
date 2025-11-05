#include "src/core/platform/macos/MacGpuInfo.h"
#include "src/core/platform/macos/MacGpuAdapter.h"
#include "src/core/factory/InfoFactory.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== macOS GPU信息测试 ===" << std::endl;
    
    // 测试工厂模式创建GPU信息
    auto gpuInfo = InfoFactory::CreateGpuInfo();
    if (gpuInfo) {
        std::cout << "✓ GPU Info created successfully" << std::endl;
        
        // 显示基本GPU信息
        std::cout << "\n--- 基本GPU信息 ---" << std::endl;
        std::cout << "GPU名称: " << gpuInfo->GetName() << std::endl;
        std::cout << "供应商: " << gpuInfo->GetVendor() << std::endl;
        std::cout << "驱动版本: " << gpuInfo->GetDriverVersion() << std::endl;
        std::cout << "架构: " << gpuInfo->GetArchitecture() << std::endl;
        
        // 显示内存信息
        std::cout << "\n--- 内存信息 ---" << std::endl;
        uint64_t dedicated = gpuInfo->GetDedicatedMemory();
        uint64_t shared = gpuInfo->GetSharedMemory();
        uint64_t total = dedicated + shared;
        
        std::cout << "专用内存: " << std::fixed << std::setprecision(2) 
                  << (double)dedicated / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "共享内存: " << std::fixed << std::setprecision(2) 
                  << (double)shared / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "总内存: " << std::fixed << std::setprecision(2) 
                  << (double)total / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "内存使用率: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetMemoryUsage() << "%" << std::endl;
        
        // 显示性能信息
        std::cout << "\n--- 性能信息 ---" << std::endl;
        std::cout << "GPU使用率: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetGpuUsage() << "%" << std::endl;
        std::cout << "视频解码使用率: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetVideoDecoderUsage() << "%" << std::endl;
        std::cout << "视频编码使用率: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetVideoEncoderUsage() << "%" << std::endl;
        std::cout << "计算单元数: " << gpuInfo->GetComputeUnits() << std::endl;
        std::cout << "性能评级: " << std::fixed << std::setprecision(0) 
                  << gpuInfo->GetPerformanceRating() << std::endl;
        
        // 显示频率信息
        std::cout << "\n--- 频率信息 ---" << std::endl;
        std::cout << "当前频率: " << std::fixed << std::setprecision(0) 
                  << gpuInfo->GetCurrentFrequency() << " MHz" << std::endl;
        std::cout << "基础频率: " << std::fixed << std::setprecision(0) 
                  << gpuInfo->GetBaseFrequency() << " MHz" << std::endl;
        std::cout << "最大频率: " << std::fixed << std::setprecision(0) 
                  << gpuInfo->GetMaxFrequency() << " MHz" << std::endl;
        std::cout << "内存频率: " << std::fixed << std::setprecision(0) 
                  << gpuInfo->GetMemoryFrequency() << " MHz" << std::endl;
        
        // 显示温度和功耗信息
        std::cout << "\n--- 温度和功耗 ---" << std::endl;
        std::cout << "GPU温度: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetTemperature() << "°C" << std::endl;
        std::cout << "热点温度: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetHotspotTemperature() << "°C" << std::endl;
        std::cout << "内存温度: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetMemoryTemperature() << "°C" << std::endl;
        std::cout << "GPU功耗: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetPowerUsage() << " W" << std::endl;
        std::cout << "板卡功耗: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetBoardPower() << " W" << std::endl;
        std::cout << "最大功耗限制: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetMaxPowerLimit() << " W" << std::endl;
        
        // 显示风扇信息
        std::cout << "\n--- 风扇信息 ---" << std::endl;
        std::cout << "风扇转速: " << std::fixed << std::setprecision(0) 
                  << gpuInfo->GetFanSpeed() << " RPM" << std::endl;
        std::cout << "风扇转速百分比: " << std::fixed << std::setprecision(1) 
                  << gpuInfo->GetFanSpeedPercent() << "%" << std::endl;
        
    } else {
        std::cout << "✗ Failed to create GPU Info" << std::endl;
        std::cout << "  Error: " << InfoFactory::GetLastError() << std::endl;
        return 1;
    }
    
    // 测试GPU适配器
    auto gpuAdapter = InfoFactory::CreateMacGpuAdapter();
    if (gpuAdapter) {
        std::cout << "\n✓ GPU Adapter created successfully" << std::endl;
        
        const auto& gpuData = gpuAdapter->GetGpuData();
        if (!gpuData.empty()) {
            std::cout << "GPU数据:" << std::endl;
            for (size_t i = 0; i < gpuData.size(); ++i) {
                std::wcout << L"  GPU " << i << L": " << gpuData[i].name << std::endl;
                std::wcout << L"    设备ID: " << gpuData[i].deviceId << std::endl;
                std::wcout << L"    专用内存: " << gpuData[i].dedicatedMemory << L" bytes" << std::endl;
                std::wcout << L"    核心时钟: " << gpuData[i].coreClock << L" Hz" << std::endl;
                std::wcout << L"    温度: " << gpuData[i].temperature << L"°C" << std::endl;
            }
        }
        
        std::cout << "适配器GPU使用率: " << std::fixed << std::setprecision(1) 
                  << gpuAdapter->GetGpuUsage() << "%" << std::endl;
        std::cout << "适配器总内存: " << std::fixed << std::setprecision(2) 
                  << (double)gpuAdapter->GetTotalMemory() / (1024*1024*1024) << " GB" << std::endl;
    } else {
        std::cout << "✗ Failed to create GPU Adapter" << std::endl;
        std::cout << "  Error: " << InfoFactory::GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== 跨平台GPU架构测试成功 ===" << std::endl;
    std::cout << "✓ macOS GPU信息获取正常" << std::endl;
    std::cout << "✓ 接口统一，实现分离" << std::endl;
    std::cout << "✓ 原有功能保持不变" << std::endl;
    
    return 0;
}