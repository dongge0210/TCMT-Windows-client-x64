#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

#include "src/core/common/BaseInfo.h"
#include "src/core/factory/InfoFactory.h"

class SimpleMonitor {
private:
    std::unique_ptr<ICpuInfo> m_cpuInfo;
    std::unique_ptr<IMemoryInfo> m_memoryInfo;
    std::unique_ptr<IGpuInfo> m_gpuInfo;
    std::unique_ptr<IBatteryInfo> m_batteryInfo;
    std::unique_ptr<ITemperatureInfo> m_temperatureInfo;
    std::unique_ptr<IDiskInfo> m_diskInfo;
    std::unique_ptr<INetworkInfo> m_networkInfo;
    
    bool m_initialized;
    
public:
    SimpleMonitor() : m_initialized(false) {}
    
    bool Initialize() {
        std::cout << "=== 初始化简化跨平台硬件监控系统 ===" << std::endl;
        
        try {
            // 使用工厂模式创建各个监控组件
            m_cpuInfo = InfoFactory::CreateCpuInfo();
            m_memoryInfo = InfoFactory::CreateMemoryInfo();
            m_gpuInfo = InfoFactory::CreateGpuInfo();
            
            // 使用工厂实例创建新组件
            InfoFactory factory;
            m_batteryInfo = factory.CreateBatteryInfo();
            m_temperatureInfo = factory.CreateTemperatureInfo();
            
            // 检查初始化状态
            bool cpu_ok = m_cpuInfo && m_cpuInfo->Initialize();
            bool memory_ok = m_memoryInfo && m_memoryInfo->Initialize();
            bool gpu_ok = m_gpuInfo && m_gpuInfo->Initialize();
            bool battery_ok = m_batteryInfo && m_batteryInfo->Initialize();
            bool temp_ok = m_temperatureInfo && m_temperatureInfo->Initialize();
            
            m_initialized = cpu_ok && memory_ok && gpu_ok && battery_ok && temp_ok;
            
            std::cout << "初始化结果:" << std::endl;
            std::cout << "  CPU监控: " << (cpu_ok ? "成功" : "失败") << std::endl;
            std::cout << "  内存监控: " << (memory_ok ? "成功" : "失败") << std::endl;
            std::cout << "  GPU监控: " << (gpu_ok ? "成功" : "失败") << std::endl;
            std::cout << "  电池监控: " << (battery_ok ? "成功" : "失败") << std::endl;
            std::cout << "  温度监控: " << (temp_ok ? "成功" : "失败") << std::endl;
            
            return m_initialized;
            
        } catch (const std::exception& e) {
            std::cerr << "初始化失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    void RunMonitor() {
        if (!m_initialized) {
            std::cerr << "监控系统未初始化" << std::endl;
            return;
        }
        
        std::cout << "\n=== 开始实时监控 ===" << std::endl;
        
        for (int i = 0; i < 5; ++i) {
            std::cout << "\n--- 第 " << (i + 1) << " 次监控 ---" << std::endl;
            
            // 更新数据
            bool update_ok = true;
            if (m_cpuInfo && !m_cpuInfo->Update()) update_ok = false;
            if (m_memoryInfo && !m_memoryInfo->Update()) update_ok = false;
            if (m_gpuInfo && !m_gpuInfo->Update()) update_ok = false;
            if (m_batteryInfo && !m_batteryInfo->Update()) update_ok = false;
            if (m_temperatureInfo && !m_temperatureInfo->Update()) update_ok = false;
            
            if (!update_ok) {
                std::cout << "数据更新失败" << std::endl;
                continue;
            }
            
            // 显示CPU信息
            if (m_cpuInfo && m_cpuInfo->IsDataValid()) {
                std::cout << "CPU信息:" << std::endl;
                std::cout << "  名称: " << m_cpuInfo->GetName() << std::endl;
                std::cout << "  使用率: " << std::fixed << std::setprecision(1) << m_cpuInfo->GetTotalUsage() << "%" << std::endl;
                std::cout << "  频率: " << std::fixed << std::setprecision(2) << m_cpuInfo->GetCurrentFrequency() << " GHz" << std::endl;
                std::cout << "  温度: " << std::fixed << std::setprecision(1) << m_cpuInfo->GetTemperature() << "°C" << std::endl;
                std::cout << "  功耗: " << std::fixed << std::setprecision(1) << m_cpuInfo->GetPowerUsage() << "W" << std::endl;
            }
            
            // 显示内存信息
            if (m_memoryInfo && m_memoryInfo->IsDataValid()) {
                std::cout << "内存信息:" << std::endl;
                std::cout << "  总内存: " << FormatBytes(m_memoryInfo->GetTotalPhysicalMemory()) << std::endl;
                std::cout << "  可用内存: " << FormatBytes(m_memoryInfo->GetAvailablePhysicalMemory()) << std::endl;
                std::cout << "  使用率: " << std::fixed << std::setprecision(1) << m_memoryInfo->GetPhysicalMemoryUsage() << "%" << std::endl;
                std::cout << "  内存压力: " << std::fixed << std::setprecision(1) << m_memoryInfo->GetMemoryPressure() << std::endl;
            }
            
            // 显示GPU信息
            if (m_gpuInfo && m_gpuInfo->IsDataValid()) {
                std::cout << "GPU信息:" << std::endl;
                std::cout << "  名称: " << m_gpuInfo->GetName() << std::endl;
                std::cout << "  使用率: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetGpuUsage() << "%" << std::endl;
                std::cout << "  显存使用: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetMemoryUsage() << "%" << std::endl;
                std::cout << "  温度: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetTemperature() << "°C" << std::endl;
            }
            
            // 显示电池信息
            if (m_batteryInfo && m_batteryInfo->IsDataValid()) {
                std::cout << "电池信息:" << std::endl;
                std::cout << "  存在: " << (m_batteryInfo->IsBatteryPresent() ? "是" : "否") << std::endl;
                std::cout << "  充电状态: " << (m_batteryInfo->IsCharging() ? "充电中" : "未充电") << std::endl;
                std::cout << "  电量: " << std::fixed << std::setprecision(1) << m_batteryInfo->GetChargePercentage() << "%" << std::endl;
                std::cout << "  电压: " << std::fixed << std::setprecision(2) << m_batteryInfo->GetVoltage() << "V" << std::endl;
                std::cout << "  电流: " << std::fixed << std::setprecision(1) << m_batteryInfo->GetAmperage() << "mA" << std::endl;
                std::cout << "  健康度: " << std::fixed << std::setprecision(1) << m_batteryInfo->GetHealthPercentage() << "%" << std::endl;
            }
            
            // 显示温度信息
            if (m_temperatureInfo && m_temperatureInfo->IsDataValid()) {
                std::cout << "温度信息:" << std::endl;
                std::cout << "  CPU温度: " << std::fixed << std::setprecision(1) << m_temperatureInfo->GetCPUTemperature() << "°C" << std::endl;
                std::cout << "  GPU温度: " << std::fixed << std::setprecision(1) << m_temperatureInfo->GetGPUTemperature() << "°C" << std::endl;
                std::cout << "  系统温度: " << std::fixed << std::setprecision(1) << m_temperatureInfo->GetSystemTemperature() << "°C" << std::endl;
                std::cout << "  过热警告: " << (m_temperatureInfo->IsOverheating() ? "是" : "否") << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    
    void Cleanup() {
        std::cout << "\n=== 清理监控系统 ===" << std::endl;
        
        if (m_cpuInfo) m_cpuInfo->Cleanup();
        if (m_memoryInfo) m_memoryInfo->Cleanup();
        if (m_gpuInfo) m_gpuInfo->Cleanup();
        if (m_batteryInfo) m_batteryInfo->Cleanup();
        if (m_temperatureInfo) m_temperatureInfo->Cleanup();
        
        m_initialized = false;
    }
    
private:
    std::string FormatBytes(uint64_t bytes) const {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_index = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unit_index < 4) {
            size /= 1024.0;
            unit_index++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
        return oss.str();
    }
};

int main() {
    std::cout << "=== 跨平台硬件监控系统测试 ===" << std::endl;
    std::cout << "平台: macOS" << std::endl;
    std::cout << "编译器: Clang++" << std::endl;
    std::cout << "C++标准: C++20" << std::endl;
    std::cout << "时间: " << __DATE__ << " " << __TIME__ << std::endl;
    
    SimpleMonitor monitor;
    
    if (monitor.Initialize()) {
        monitor.RunMonitor();
        monitor.Cleanup();
    } else {
        std::cerr << "监控系统初始化失败" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}