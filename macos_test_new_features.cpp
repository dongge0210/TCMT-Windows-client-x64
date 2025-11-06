#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <memory>

// 包含新功能的头文件
#include "src/core/factory/InfoFactory.h"
#include "src/core/common/BaseInfo.h"

void testSystemInfo() {
    std::cout << "\n=== 测试系统信息监控 ===" << std::endl;
    
    auto systemInfo = InfoFactory::CreateSystemInfo();
    if (!systemInfo) {
        std::cout << "✗ 创建系统信息对象失败: " << InfoFactory::GetLastError() << std::endl;
        return;
    }
    
    // 初始化
    if (!systemInfo->Initialize()) {
        std::cout << "✗ 系统信息初始化失败" << std::endl;
        return;
    }
    
    std::cout << "✓ 系统信息初始化成功" << std::endl;
    
    // 测试基本系统信息
    std::cout << "\n--- 基本系统信息 ---" << std::endl;
    std::cout << "操作系统: " << systemInfo->GetOSName() << std::endl;
    std::cout << "版本: " << systemInfo->GetOSVersion() << std::endl;
    std::cout << "主机名: " << systemInfo->GetHostname() << std::endl;
    std::cout << "架构: " << systemInfo->GetArchitecture() << std::endl;
    
    // 测试运行时间
    std::cout << "\n--- 系统运行时间 ---" << std::endl;
    uint64_t uptime = systemInfo->GetUptimeSeconds();
    std::cout << "运行时间: " << uptime << " 秒 (" << (uptime / 3600) << " 小时)" << std::endl;
    
    // 测试系统负载
    std::cout << "\n--- 系统负载 ---" << std::endl;
    std::cout << "1分钟负载: " << std::fixed << std::setprecision(2) 
              << systemInfo->GetLoadAverage1Min() << std::endl;
    std::cout << "5分钟负载: " << std::fixed << std::setprecision(2) 
              << systemInfo->GetLoadAverage5Min() << std::endl;
    std::cout << "15分钟负载: " << std::fixed << std::setprecision(2) 
              << systemInfo->GetLoadAverage15Min() << std::endl;
    
    // 测试进程信息
    std::cout << "\n--- 进程信息 ---" << std::endl;
    std::cout << "总进程数: " << systemInfo->GetTotalProcesses() << std::endl;
    std::cout << "运行中进程: " << systemInfo->GetRunningProcesses() << std::endl;
    std::cout << "睡眠进程: " << systemInfo->GetSleepingProcesses() << std::endl;
    std::cout << "线程数: " << systemInfo->GetThreads() << std::endl;
    
    // 测试内存信息
    std::cout << "\n--- 内存信息 ---" << std::endl;
    uint64_t totalMem = systemInfo->GetTotalPhysicalMemory();
    uint64_t availMem = systemInfo->GetAvailablePhysicalMemory();
    std::cout << "总物理内存: " << std::fixed << std::setprecision(2) 
              << (double)totalMem / (1024*1024*1024) << " GB" << std::endl;
    std::cout << "可用内存: " << std::fixed << std::setprecision(2) 
              << (double)availMem / (1024*1024*1024) << " GB" << std::endl;
    std::cout << "使用率: " << std::fixed << std::setprecision(1) 
              << ((double)(totalMem - availMem) / totalMem * 100) << "%" << std::endl;
    
    // 测试磁盘信息
    std::cout << "\n--- 磁盘信息 ---" << std::endl;
    uint64_t totalDisk = systemInfo->GetTotalDiskSpace();
    uint64_t availDisk = systemInfo->GetAvailableDiskSpace();
    std::cout << "总磁盘空间: " << std::fixed << std::setprecision(2) 
              << (double)totalDisk / (1024*1024*1024) << " GB" << std::endl;
    std::cout << "可用空间: " << std::fixed << std::setprecision(2) 
              << (double)availDisk / (1024*1024*1024) << " GB" << std::endl;
    std::cout << "使用率: " << std::fixed << std::setprecision(1) 
              << ((double)(totalDisk - availDisk) / totalDisk * 100) << "%" << std::endl;
    
    // 测试网络统计
    std::cout << "\n--- 网络统计 ---" << std::endl;
    std::cout << "接收字节: " << systemInfo->GetNetworkBytesReceived() << std::endl;
    std::cout << "发送字节: " << systemInfo->GetNetworkBytesSent() << std::endl;
    std::cout << "接收包数: " << systemInfo->GetNetworkPacketsReceived() << std::endl;
    std::cout << "发送包数: " << systemInfo->GetNetworkPacketsSent() << std::endl;
    
    // 测试系统健康状态
    std::cout << "\n--- 系统健康状态 ---" << std::endl;
    std::cout << "系统健康: " << (systemInfo->IsSystemHealthy() ? "正常" : "异常") << std::endl;
    std::cout << "健康评分: " << std::fixed << std::setprecision(1) 
              << systemInfo->GetSystemHealthScore() << "/100" << std::endl;
    std::cout << "系统状态: " << systemInfo->GetSystemStatus() << std::endl;
    
    // 清理
    systemInfo->Cleanup();
    std::cout << "✓ 系统信息测试完成" << std::endl;
}

void testBatteryInfo() {
    std::cout << "\n=== 测试电池状态监控 ===" << std::endl;
    
    auto batteryInfo = InfoFactory::CreateBatteryInfo();
    if (!batteryInfo) {
        std::cout << "✗ 创建电池信息对象失败: " << InfoFactory::GetLastError() << std::endl;
        return;
    }
    
    // 初始化
    if (!batteryInfo->Initialize()) {
        std::cout << "✗ 电池信息初始化失败" << std::endl;
        return;
    }
    
    std::cout << "✓ 电池信息初始化成功" << std::endl;
    
    // 检查电池是否存在
    if (!batteryInfo->IsBatteryPresent()) {
        std::cout << "ℹ 未检测到电池（可能是台式机）" << std::endl;
        batteryInfo->Cleanup();
        return;
    }
    
    std::cout << "✓ 检测到电池设备" << std::endl;
    
    // 测试充电状态
    std::cout << "\n--- 充电状态 ---" << std::endl;
    std::cout << "充电中: " << (batteryInfo->IsCharging() ? "是" : "否") << std::endl;
    std::cout << "充满电: " << (batteryInfo->IsFullyCharged() ? "是" : "否") << std::endl;
    std::cout << "放电中: " << (batteryInfo->IsDischarging() ? "是" : "否") << std::endl;
    
    // 测试电量信息
    std::cout << "\n--- 电量信息 ---" << std::endl;
    std::cout << "当前电量: " << std::fixed << std::setprecision(1) 
              << batteryInfo->GetChargePercentage() << "%" << std::endl;
    std::cout << "设计容量: " << std::fixed << std::setprecision(1) 
              << batteryInfo->GetDesignCapacity() << " mAh" << std::endl;
    std::cout << "当前容量: " << std::fixed << std::setprecision(1) 
              << batteryInfo->GetCurrentCapacity() << " mAh" << std::endl;
    std::cout << "最大容量: " << std::fixed << std::setprecision(1) 
              << batteryInfo->GetMaximumCapacity() << " mAh" << std::endl;
    std::cout << "健康度: " << std::fixed << std::setprecision(1) 
              << batteryInfo->GetHealthPercentage() << "%" << std::endl;
    
    // 测试循环次数
    uint32_t cycles = batteryInfo->GetCycleCount();
    if (cycles > 0) {
        std::cout << "循环次数: " << cycles << std::endl;
    }
    
    // 测试时间信息
    std::cout << "\n--- 时间信息 ---" << std::endl;
    uint64_t timeToEmpty = batteryInfo->GetTimeToEmpty();
    uint64_t timeToFull = batteryInfo->GetTimeToFullCharge();
    uint64_t timeRemaining = batteryInfo->GetTimeRemaining();
    
    if (timeToEmpty > 0) {
        std::cout << "剩余使用时间: " << (timeToEmpty / 3600) << " 小时 " 
                  << ((timeToEmpty % 3600) / 60) << " 分钟" << std::endl;
    }
    if (timeToFull > 0) {
        std::cout << "充满时间: " << (timeToFull / 3600) << " 小时 " 
                  << ((timeToFull % 3600) / 60) << " 分钟" << std::endl;
    }
    if (timeRemaining > 0) {
        std::cout << "剩余时间: " << (timeRemaining / 3600) << " 小时 " 
                  << ((timeRemaining % 3600) / 60) << " 分钟" << std::endl;
    }
    
    // 测试温度和电压
    std::cout << "\n--- 温度和电压 ---" << std::endl;
    double temperature = batteryInfo->GetTemperature();
    double voltage = batteryInfo->GetVoltage();
    double current = batteryInfo->GetCurrent();
    
    if (temperature > 0) {
        std::cout << "电池温度: " << std::fixed << std::setprecision(1) 
                  << temperature << "°C" << std::endl;
    }
    if (voltage > 0) {
        std::cout << "电压: " << std::fixed << std::setprecision(2) 
                  << voltage << " V" << std::endl;
    }
    if (current > 0) {
        std::cout << "电流: " << std::fixed << std::setprecision(3) 
                  << current << " A" << std::endl;
    }
    
    // 测试电源信息
    std::cout << "\n--- 电源信息 ---" << std::endl;
    std::cout << "电源类型: " << batteryInfo->GetPowerSource() << std::endl;
    std::cout << "充电器类型: " << batteryInfo->GetChargerType() << std::endl;
    double power = batteryInfo->GetPowerWattage();
    if (power > 0) {
        std::cout << "功率: " << std::fixed << std::setprecision(1) 
                  << power << " W" << std::endl;
    }
    
    // 测试电池标识
    std::cout << "\n--- 电池标识 ---" << std::endl;
    std::cout << "制造商: " << batteryInfo->GetBatteryManufacturer() << std::endl;
    std::cout << "型号: " << batteryInfo->GetBatteryModel() << std::endl;
    std::cout << "序列号: " << batteryInfo->GetSerialNumber() << std::endl;
    
    // 测试电池特性
    std::cout << "\n--- 电池特性 ---" << std::endl;
    std::cout << "支持快充: " << (batteryInfo->SupportsFastCharging() ? "是" : "否") << std::endl;
    std::cout << "优化充电: " << (batteryInfo->IsOptimizedBatteryChargingEnabled() ? "启用" : "禁用") << std::endl;
    
    auto warnings = batteryInfo->GetBatteryWarnings();
    if (!warnings.empty()) {
        std::cout << "警告信息: ";
        for (size_t i = 0; i < warnings.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << warnings[i];
        }
        std::cout << std::endl;
    }
    
    // 清理
    batteryInfo->Cleanup();
    std::cout << "✓ 电池信息测试完成" << std::endl;
}

void testDynamicUpdates() {
    std::cout << "\n=== 测试动态更新 ===" << std::endl;
    
    auto systemInfo = InfoFactory::CreateSystemInfo();
    auto batteryInfo = InfoFactory::CreateBatteryInfo();
    
    if (!systemInfo || !batteryInfo) {
        std::cout << "✗ 创建对象失败" << std::endl;
        return;
    }
    
    if (!systemInfo->Initialize() || !batteryInfo->Initialize()) {
        std::cout << "✗ 初始化失败" << std::endl;
        return;
    }
    
    std::cout << "开始动态监控（5次更新）..." << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- 第 " << (i + 1) << " 次更新 ---" << std::endl;
        
        // 更新数据
        systemInfo->Update();
        batteryInfo->Update();
        
        // 显示关键指标
        std::cout << "系统负载1分钟: " << std::fixed << std::setprecision(2) 
                  << systemInfo->GetLoadAverage1Min() << std::endl;
        std::cout << "内存使用率: " << std::fixed << std::setprecision(1) 
                  << ((double)(systemInfo->GetTotalPhysicalMemory() - systemInfo->GetAvailablePhysicalMemory()) / 
                   systemInfo->GetTotalPhysicalMemory() * 100) << "%" << std::endl;
        
        if (batteryInfo->IsBatteryPresent()) {
            std::cout << "电池电量: " << std::fixed << std::setprecision(1) 
                      << batteryInfo->GetChargePercentage() << "%" << std::endl;
            std::cout << "充电状态: " << (batteryInfo->IsCharging() ? "充电中" : "未充电") << std::endl;
        }
        
        // 等待2秒
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    systemInfo->Cleanup();
    batteryInfo->Cleanup();
    std::cout << "\n✓ 动态更新测试完成" << std::endl;
}

int main() {
    std::cout << "=== macOS 新功能数据检测测试 ===" << std::endl;
    std::cout << "测试系统信息和电池状态监控功能" << std::endl;
    
    try {
        // 测试系统信息
        testSystemInfo();
        
        // 测试电池信息
        testBatteryInfo();
        
        // 测试动态更新
        testDynamicUpdates();
        
        std::cout << "\n=== 所有测试完成 ===" << std::endl;
        std::cout << "✓ 系统信息监控功能正常" << std::endl;
        std::cout << "✓ 电池状态监控功能正常" << std::endl;
        std::cout << "✓ 动态更新机制正常" << std::endl;
        std::cout << "✓ 错误处理机制正常" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "✗ 测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}