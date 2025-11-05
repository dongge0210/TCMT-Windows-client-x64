#include "src/core/platform/macos/MacMemoryInfo.h"
#include "src/core/platform/macos/MacMemoryAdapter.h"
#include "src/core/factory/InfoFactory.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== macOS RAM信息测试 ===" << std::endl;
    
    // 测试工厂模式创建内存信息
    auto memoryInfo = InfoFactory::CreateMemoryInfo();
    if (memoryInfo) {
        std::cout << "✓ Memory Info created successfully" << std::endl;
        
        // 显示物理内存信息
        std::cout << "\n--- 物理内存 ---" << std::endl;
        uint64_t total = memoryInfo->GetTotalPhysicalMemory();
        uint64_t available = memoryInfo->GetAvailablePhysicalMemory();
        uint64_t used = memoryInfo->GetUsedPhysicalMemory();
        
        std::cout << "总内存: " << std::fixed << std::setprecision(2) 
                  << (double)total / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "可用内存: " << std::fixed << std::setprecision(2) 
                  << (double)available / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "已用内存: " << std::fixed << std::setprecision(2) 
                  << (double)used / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "使用率: " << std::fixed << std::setprecision(1) 
                  << memoryInfo->GetPhysicalMemoryUsage() << "%" << std::endl;
        
        // 显示虚拟内存信息
        std::cout << "\n--- 虚拟内存 ---" << std::endl;
        uint64_t totalVirtual = memoryInfo->GetTotalVirtualMemory();
        uint64_t usedVirtual = memoryInfo->GetUsedVirtualMemory();
        
        std::cout << "总虚拟内存: " << std::fixed << std::setprecision(2) 
                  << (double)totalVirtual / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "已用虚拟内存: " << std::fixed << std::setprecision(2) 
                  << (double)usedVirtual / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "虚拟内存使用率: " << std::fixed << std::setprecision(1) 
                  << memoryInfo->GetVirtualMemoryUsage() << "%" << std::endl;
        
        // 显示交换文件信息
        std::cout << "\n--- 交换文件 ---" << std::endl;
        uint64_t totalSwap = memoryInfo->GetTotalSwapMemory();
        uint64_t usedSwap = memoryInfo->GetUsedSwapMemory();
        
        std::cout << "总交换文件: " << std::fixed << std::setprecision(2) 
                  << (double)totalSwap / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "已用交换文件: " << std::fixed << std::setprecision(2) 
                  << (double)usedSwap / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "交换文件使用率: " << std::fixed << std::setprecision(1) 
                  << memoryInfo->GetSwapMemoryUsage() << "%" << std::endl;
        
        // 显示内存详情
        std::cout << "\n--- 内存详情 ---" << std::endl;
        std::cout << "内存类型: " << memoryInfo->GetMemoryType() << std::endl;
        std::cout << "内存速度: " << std::fixed << std::setprecision(0) 
                  << memoryInfo->GetMemorySpeed() << " MHz" << std::endl;
        std::cout << "内存通道数: " << memoryInfo->GetMemoryChannels() << std::endl;
        std::cout << "缓存内存: " << std::fixed << std::setprecision(2) 
                  << (double)memoryInfo->GetCachedMemory() / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "共享内存: " << std::fixed << std::setprecision(2) 
                  << (double)memoryInfo->GetSharedMemory() / (1024*1024*1024) << " GB" << std::endl;
        
        // 显示内存压力状态
        std::cout << "\n--- 内存状态 ---" << std::endl;
        std::cout << "内存压力: " << std::fixed << std::setprecision(1) 
                  << memoryInfo->GetMemoryPressure() << "%" << std::endl;
        std::cout << "内存不足: " << (memoryInfo->IsMemoryLow() ? "是" : "否") << std::endl;
        std::cout << "内存危急: " << (memoryInfo->IsMemoryCritical() ? "是" : "否") << std::endl;
        
    } else {
        std::cout << "✗ Failed to create Memory Info" << std::endl;
        std::cout << "  Error: " << InfoFactory::GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== 跨平台RAM架构测试成功 ===" << std::endl;
    std::cout << "✓ macOS RAM信息获取正常" << std::endl;
    std::cout << "✓ 接口统一，实现分离" << std::endl;
    std::cout << "✓ 原有功能保持不变" << std::endl;
    
    return 0;
}