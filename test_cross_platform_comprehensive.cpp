#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

#include "src/core/common/BaseInfo.h"
#include "src/core/factory/InfoFactory.h"

#ifdef PLATFORM_MACOS
#include "src/core/platform/macos/MacCpuInfo.h"
#include "src/core/platform/macos/MacMemoryInfo.h"
#include "src/core/platform/macos/MacGpuInfo.h"
#include "src/core/platform/macos/MacSystemInfo.h"
#include "src/core/platform/macos/MacBatteryInfo.h"
#include "src/core/platform/macos/MacTemperatureInfo.h"
#include "src/core/platform/macos/MacDiskInfo.h"
#include "src/core/platform/macos/MacNetworkInfo.h"
#endif

class ComprehensiveMonitor {
private:
    std::unique_ptr<ICpuInfo> m_cpuInfo;
    std::unique_ptr<IMemoryInfo> m_memoryInfo;
    std::unique_ptr<IGpuInfo> m_gpuInfo;
    std::unique_ptr<ISystemInfo> m_systemInfo;
    std::unique_ptr<IBatteryInfo> m_batteryInfo;
    std::unique_ptr<ITemperatureInfo> m_temperatureInfo;
    std::unique_ptr<IDiskInfo> m_diskInfo;
    std::unique_ptr<INetworkInfo> m_networkInfo;
    
    bool m_initialized;
    
public:
    ComprehensiveMonitor() : m_initialized(false) {}
    
    bool Initialize() {
        std::cout << "=== 初始化跨平台硬件监控系统 ===" << std::endl;
        
        try {
            // 使用工厂模式创建各个监控组件
            m_cpuInfo = InfoFactory::CreateCpuInfo();
            m_memoryInfo = InfoFactory::CreateMemoryInfo();
            m_gpuInfo = InfoFactory::CreateGpuInfo();
            
            InfoFactory factory;
            m_systemInfo = factory.CreateSystemInfo();
            m_batteryInfo = factory.CreateBatteryInfo();
            m_temperatureInfo = factory.CreateTemperatureInfo();
            m_diskInfo = factory.CreateDiskInfo("");
            m_networkInfo = factory.CreateNetworkInfo();
            
            // 初始化各个组件
            bool success = true;
            
            if (m_cpuInfo && !m_cpuInfo->Initialize()) {
                std::cerr << "CPU信息初始化失败" << std::endl;
                success = false;
            }
            
            if (m_memoryInfo && !m_memoryInfo->Initialize()) {
                std::cerr << "内存信息初始化失败" << std::endl;
                success = false;
            }
            
            if (m_gpuInfo && !m_gpuInfo->Initialize()) {
                std::cerr << "GPU信息初始化失败" << std::endl;
                success = false;
            }
            
            if (m_systemInfo && !m_systemInfo->Initialize()) {
                std::cerr << "系统信息初始化失败" << std::endl;
                success = false;
            }
            
            if (m_batteryInfo && !m_batteryInfo->Initialize()) {
                std::cerr << "电池信息初始化失败" << std::endl;
                success = false;
            }
            
            if (m_temperatureInfo && !m_temperatureInfo->Initialize()) {
                std::cerr << "温度信息初始化失败" << std::endl;
                success = false;
            }
            
            if (m_diskInfo && !m_diskInfo->Initialize()) {
                std::cerr << "磁盘信息初始化失败" << std::endl;
                success = false;
            }
            
            if (m_networkInfo && !m_networkInfo->Initialize()) {
                std::cerr << "网络信息初始化失败" << std::endl;
                success = false;
            }
            
            m_initialized = success;
            
            if (m_initialized) {
                std::cout << "✓ 所有监控组件初始化成功" << std::endl;
            } else {
                std::cout << "✗ 部分监控组件初始化失败" << std::endl;
            }
            
            return m_initialized;
            
        } catch (const std::exception& e) {
            std::cerr << "初始化异常: " << e.what() << std::endl;
            return false;
        }
    }
    
    void UpdateAllData() {
        if (!m_initialized) return;
        
        try {
            if (m_cpuInfo) m_cpuInfo->Update();
            if (m_memoryInfo) m_memoryInfo->Update();
            if (m_gpuInfo) m_gpuInfo->Update();
            if (m_systemInfo) m_systemInfo->Update();
            if (m_batteryInfo) m_batteryInfo->Update();
            if (m_temperatureInfo) m_temperatureInfo->Update();
            if (m_diskInfo) m_diskInfo->Update();
            if (m_networkInfo) m_networkInfo->Update();
        } catch (const std::exception& e) {
            std::cerr << "更新数据异常: " << e.what() << std::endl;
        }
    }
    
    void DisplaySystemInfo() {
        std::cout << "\n=== 系统信息 ===" << std::endl;
        
        if (m_systemInfo) {
            std::cout << "操作系统: " << m_systemInfo->GetOSName() 
                      << " " << m_systemInfo->GetOSVersion() << std::endl;
            std::cout << "架构: " << m_systemInfo->GetArchitecture() << std::endl;
            std::cout << "主机名: " << m_systemInfo->GetHostname() << std::endl;
            std::cout << "运行时间: " << FormatUptime(m_systemInfo->GetUptimeSeconds()) << std::endl;
            std::cout << "进程数: " << m_systemInfo->GetProcessCount() << std::endl;
            std::cout << "线程数: " << m_systemInfo->GetThreadCount() << std::endl;
        }
    }
    
    void DisplayCpuInfo() {
        std::cout << "\n=== CPU 信息 ===" << std::endl;
        
        if (m_cpuInfo) {
            std::cout << "CPU型号: " << m_cpuInfo->GetName() << std::endl;
            std::cout << "制造商: " << m_cpuInfo->GetVendor() << std::endl;
            std::cout << "架构: " << m_cpuInfo->GetArchitecture() << std::endl;
            std::cout << "物理核心: " << m_cpuInfo->GetPhysicalCores() << std::endl;
            std::cout << "逻辑核心: " << m_cpuInfo->GetLogicalCores() << std::endl;
            
            if (m_cpuInfo->HasHybridArchitecture()) {
                std::cout << "性能核心: " << m_cpuInfo->GetPerformanceCores() << std::endl;
                std::cout << "效率核心: " << m_cpuInfo->GetEfficiencyCores() << std::endl;
            }
            
            std::cout << "CPU使用率: " << std::fixed << std::setprecision(1) 
                      << m_cpuInfo->GetTotalUsage() * 100 << "%" << std::endl;
            std::cout << "当前频率: " << std::fixed << std::setprecision(0) 
                      << m_cpuInfo->GetCurrentFrequency() << " MHz" << std::endl;
            std::cout << "CPU温度: " << std::fixed << std::setprecision(1) 
                      << m_cpuInfo->GetTemperature() << "°C" << std::endl;
            std::cout << "CPU功耗: " << std::fixed << std::setprecision(2) 
                      << m_cpuInfo->GetPowerUsage() << " W" << std::endl;
        }
    }
    
    void DisplayMemoryInfo() {
        std::cout << "\n=== 内存信息 ===" << std::endl;
        
        if (m_memoryInfo) {
            std::cout << "总物理内存: " << FormatBytes(m_memoryInfo->GetTotalPhysicalMemory()) << std::endl;
            std::cout << "可用物理内存: " << FormatBytes(m_memoryInfo->GetAvailablePhysicalMemory()) << std::endl;
            std::cout << "已用物理内存: " << FormatBytes(m_memoryInfo->GetUsedPhysicalMemory()) << std::endl;
            std::cout << "物理内存使用率: " << std::fixed << std::setprecision(1) 
                      << m_memoryInfo->GetPhysicalMemoryUsage() * 100 << "%" << std::endl;
            
            std::cout << "总虚拟内存: " << FormatBytes(m_memoryInfo->GetTotalVirtualMemory()) << std::endl;
            std::cout << "可用虚拟内存: " << FormatBytes(m_memoryInfo->GetAvailableVirtualMemory()) << std::endl;
            std::cout << "已用虚拟内存: " << FormatBytes(m_memoryInfo->GetUsedVirtualMemory()) << std::endl;
            std::cout << "虚拟内存使用率: " << std::fixed << std::setprecision(1) 
                      << m_memoryInfo->GetVirtualMemoryUsage() * 100 << "%" << std::endl;
            
            std::cout << "交换文件: " << FormatBytes(m_memoryInfo->GetTotalSwapMemory()) << std::endl;
            std::cout << "可用交换文件: " << FormatBytes(m_memoryInfo->GetAvailableSwapMemory()) << std::endl;
            std::cout << "已用交换文件: " << FormatBytes(m_memoryInfo->GetUsedSwapMemory()) << std::endl;
            std::cout << "交换文件使用率: " << std::fixed << std::setprecision(1) 
                      << m_memoryInfo->GetSwapMemoryUsage() * 100 << "%" << std::endl;
            
            std::cout << "内存压力: " << std::fixed << std::setprecision(1) 
                      << m_memoryInfo->GetMemoryPressure() * 100 << "%" << std::endl;
            std::cout << "内存状态: " << (m_memoryInfo->IsMemoryLow() ? "内存不足" : "正常") << std::endl;
        }
    }
    
    void DisplayGpuInfo() {
        std::cout << "\n=== GPU 信息 ===" << std::endl;
        
        if (m_gpuInfo) {
            std::cout << "GPU型号: " << m_gpuInfo->GetName() << std::endl;
            std::cout << "制造商: " << m_gpuInfo->GetVendor() << std::endl;
            std::cout << "驱动版本: " << m_gpuInfo->GetDriverVersion() << std::endl;
            std::cout << "专用内存: " << FormatBytes(m_gpuInfo->GetDedicatedMemory()) << std::endl;
            std::cout << "共享内存: " << FormatBytes(m_gpuInfo->GetSharedMemory()) << std::endl;
            
            std::cout << "GPU使用率: " << std::fixed << std::setprecision(1) 
                      << m_gpuInfo->GetGpuUsage() * 100 << "%" << std::endl;
            std::cout << "显存使用率: " << std::fixed << std::setprecision(1) 
                      << m_gpuInfo->GetMemoryUsage() * 100 << "%" << std::endl;
            std::cout << "当前频率: " << std::fixed << std::setprecision(0) 
                      << m_gpuInfo->GetCurrentFrequency() << " MHz" << std::endl;
            std::cout << "显存频率: " << std::fixed << std::setprecision(0) 
                      << m_gpuInfo->GetMemoryFrequency() << " MHz" << std::endl;
            std::cout << "GPU温度: " << std::fixed << std::setprecision(1) 
                      << m_gpuInfo->GetTemperature() << "°C" << std::endl;
            std::cout << "GPU功耗: " << std::fixed << std::setprecision(2) 
                      << m_gpuInfo->GetPowerUsage() << " W" << std::endl;
        }
    }
    
    void DisplayBatteryInfo() {
        std::cout << "\n=== 电池信息 ===" << std::endl;
        
        if (m_batteryInfo) {
            if (m_batteryInfo->IsBatteryPresent()) {
                std::cout << "电池型号: " << m_batteryInfo->GetBatteryModel() << std::endl;
                std::cout << "制造商: " << m_batteryInfo->GetBatteryManufacturer() << std::endl;
                std::cout << "序列号: " << m_batteryInfo->GetBatterySerialNumber() << std::endl;
                
                std::cout << "当前容量: " << m_batteryInfo->GetCurrentCapacity() << " mAh" << std::endl;
                std::cout << "最大容量: " << m_batteryInfo->GetMaxCapacity() << " mAh" << std::endl;
                std::cout << "设计容量: " << m_batteryInfo->GetDesignCapacity() << " mAh" << std::endl;
                
                std::cout << "充电百分比: " << std::fixed << std::setprecision(1) 
                          << m_batteryInfo->GetChargePercentage() << "%" << std::endl;
                std::cout << "健康百分比: " << std::fixed << std::setprecision(1) 
                          << m_batteryInfo->GetHealthPercentage() << "%" << std::endl;
                
                std::cout << "电压: " << std::fixed << std::setprecision(2) 
                          << m_batteryInfo->GetVoltage() << " V" << std::endl;
                std::cout << "电流: " << std::fixed << std::setprecision(0) 
                          << m_batteryInfo->GetAmperage() * 1000 << " mA" << std::endl;
                std::cout << "功率: " << std::fixed << std::setprecision(2) 
                          << m_batteryInfo->GetWattage() << " W" << std::endl;
                
                std::cout << "充电状态: " << m_batteryInfo->GetChargingState() << std::endl;
                std::cout << "电源状态: " << m_batteryInfo->GetPowerSourceState() << std::endl;
                
                std::cout << "剩余时间: " << m_batteryInfo->GetTimeRemaining() << " 分钟" << std::endl;
                std::cout << "充满时间: " << m_batteryInfo->GetTimeToFullCharge() << " 分钟" << std::endl;
                
                std::cout << "循环次数: " << m_batteryInfo->GetCycleCount() << std::endl;
                std::cout << "电池温度: " << std::fixed << std::setprecision(1) 
                          << m_batteryInfo->GetTemperature() << "°C" << std::endl;
                
                // 显示详细电池信息
                auto detailedInfo = m_batteryInfo->GetDetailedBatteryInfo();
                std::cout << "电池磨损: " << std::fixed << std::setprecision(1) 
                          << detailedInfo.batteryWearLevel * 100 << "%" << std::endl;
                std::cout << "制造日期: " << detailedInfo.manufacturingDate << std::endl;
                std::cout << "通电时间: " << detailedInfo.powerOnTime << " 小时" << std::endl;
                std::cout << "校准状态: " << (detailedInfo.isCalibrated ? "已校准" : "未校准") << std::endl;
                
                // 显示警告信息
                auto warnings = m_batteryInfo->GetBatteryWarnings();
                if (!warnings.empty()) {
                    std::cout << "电池警告:" << std::endl;
                    for (const auto& warning : warnings) {
                        std::cout << "  - " << warning << std::endl;
                    }
                }
                
            } else {
                std::cout << "未检测到电池" << std::endl;
            }
        }
    }
    
    void DisplayTemperatureInfo() {
        std::cout << "\n=== 温度信息 ===" << std::endl;
        
        if (m_temperatureInfo) {
            std::cout << "CPU温度: " << std::fixed << std::setprecision(1) 
                      << m_temperatureInfo->GetCPUTemperature() << "°C" << std::endl;
            std::cout << "GPU温度: " << std::fixed << std::setprecision(1) 
                      << m_temperatureInfo->GetGPUTemperature() << "°C" << std::endl;
            std::cout << "系统温度: " << std::fixed << std::setprecision(1) 
                      << m_temperatureInfo->GetSystemTemperature() << "°C" << std::endl;
            std::cout << "SSD温度: " << std::fixed << std::setprecision(1) 
                      << m_temperatureInfo->GetSSDTemperature() << "°C" << std::endl;
            
            std::cout << "温度状态: " << (m_temperatureInfo->IsOverheating() ? "过热" : "正常") << std::endl;
            std::cout << "热节流: " << (m_temperatureInfo->IsThermalThrottling() ? "是" : "否") << std::endl;
            std::cout << "热压力: " << std::fixed << std::setprecision(1) 
                      << m_temperatureInfo->GetThermalPressure() * 100 << "%" << std::endl;
            
            // 显示所有传感器
            auto sensors = m_temperatureInfo->GetAllSensors();
            std::cout << "传感器数量: " << sensors.size() << std::endl;
            for (const auto& sensor : sensors) {
                std::cout << "  " << sensor.name << ": " << std::fixed << std::setprecision(1) 
                          << sensor.temperature << "°C" << std::endl;
            }
            
            // 显示温度警告
            auto warnings = m_temperatureInfo->GetTemperatureWarnings();
            if (!warnings.empty()) {
                std::cout << "温度警告:" << std::endl;
                for (const auto& warning : warnings) {
                    std::cout << "  - " << warning << std::endl;
                }
            }
        }
    }
    
    void DisplayDiskInfo() {
        std::cout << "\n=== 磁盘信息 ===" << std::endl;
        
        if (m_diskInfo) {
            std::cout << "磁盘名称: " << m_diskInfo->GetName() << std::endl;
            std::cout << "磁盘型号: " << m_diskInfo->GetModel() << std::endl;
            std::cout << "序列号: " << m_diskInfo->GetSerialNumber() << std::endl;
            std::cout << "总大小: " << FormatBytes(m_diskInfo->GetTotalSize()) << std::endl;
            std::cout << "可用空间: " << FormatBytes(m_diskInfo->GetAvailableSpace()) << std::endl;
            std::cout << "已用空间: " << FormatBytes(m_diskInfo->GetUsedSpace()) << std::endl;
            std::cout << "使用率: " << std::fixed << std::setprecision(1) 
                      << m_diskInfo->GetUsagePercentage() << "%" << std::endl;
            std::cout << "文件系统: " << m_diskInfo->GetFileSystem() << std::endl;
            std::cout << "读取速度: " << std::fixed << std::setprecision(1) 
                      << m_diskInfo->GetReadSpeed() << " MB/s" << std::endl;
            std::cout << "写入速度: " << std::fixed << std::setprecision(1) 
                      << m_diskInfo->GetWriteSpeed() << " MB/s" << std::endl;
            std::cout << "总读取: " << FormatBytes(m_diskInfo->GetTotalReadBytes()) << std::endl;
            std::cout << "总写入: " << FormatBytes(m_diskInfo->GetTotalWriteBytes()) << std::endl;
            std::cout << "健康状态: " << m_diskInfo->GetHealthStatus() << std::endl;
            std::cout << "健康百分比: " << m_diskInfo->GetHealthPercentage() << "%" << std::endl;
            std::cout << "接口类型: " << m_diskInfo->GetInterfaceType() << std::endl;
            std::cout << "是否SSD: " << (m_diskInfo->IsSSD() ? "是" : "否") << std::endl;
            std::cout << "是否HDD: " << (m_diskInfo->IsHDD() ? "是" : "否") << std::endl;
            std::cout << "是否NVMe: " << (m_diskInfo->IsNVMe() ? "是" : "否") << std::endl;
            
            
        }
    }
    
    void DisplayNetworkInfo() {
        std::cout << "\n=== 网络信息 ===" << std::endl;
        
        if (m_networkInfo) {
            std::cout << "网络接口信息:" << std::endl;
            
            auto interfaces = m_networkInfo->GetAllInterfaces();
            for (size_t i = 0; i < interfaces.size(); ++i) {
                const auto& iface = interfaces[i];
                std::cout << "接口 " << i + 1 << ": " << iface.name << std::endl;
                std::cout << "  类型: " << iface.type << std::endl;
                std::cout << "  状态: " << iface.status << std::endl;
                std::cout << "  MAC地址: " << iface.macAddress << std::endl;
                std::cout << "  IP地址: ";
                for (const auto& ip : iface.ipAddresses) {
                    std::cout << ip << " ";
                }
                std::cout << std::endl;
                
                std::cout << "  下载速度: " << std::fixed << std::setprecision(1) 
                          << iface.downloadSpeed << " Mbps" << std::endl;
                std::cout << "  上传速度: " << std::fixed << std::setprecision(1) 
                          << iface.uploadSpeed << " Mbps" << std::endl;
                std::cout << "  总接收: " << FormatBytes(iface.totalRxBytes) << std::endl;
                std::cout << "  总发送: " << FormatBytes(iface.totalTxBytes) << std::endl;
                
                if (iface.type == "WiFi") {
                    std::cout << "  SSID: " << iface.ssid << std::endl;
                    std::cout << "  信号强度: " << std::fixed << std::setprecision(0) 
                              << iface.signalStrength << " dBm" << std::endl;
                    std::cout << "  信道: " << iface.channel << std::endl;
                    std::cout << "  安全类型: " << iface.securityType << std::endl;
                }
            }
            
            // 显示总体统计
            std::cout << "\n总体网络统计:" << std::endl;
            std::cout << "总接收: " << FormatBytes(m_networkInfo->GetTotalRxBytes()) << std::endl;
            std::cout << "总发送: " << FormatBytes(m_networkInfo->GetTotalTxBytes()) << std::endl;
            std::cout << "当前下载速度: " << std::fixed << std::setprecision(1) 
                      << m_networkInfo->GetCurrentDownloadSpeed() << " Mbps" << std::endl;
            std::cout << "当前上传速度: " << std::fixed << std::setprecision(1) 
                      << m_networkInfo->GetCurrentUploadSpeed() << " Mbps" << std::endl;
            
            
        }
    }
    
    void RunRealTimeMonitoring(int durationSeconds = 60) {
        std::cout << "\n=== 实时监控模式 (" << durationSeconds << " 秒) ===" << std::endl;
        
        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::seconds(durationSeconds);
        
        while (std::chrono::steady_clock::now() < endTime) {
            // 清屏或输出分隔符
            std::cout << "\n" << std::string(60, '=') << std::endl;
            std::cout << "监控时间: " << GetCurrentTimeString() << std::endl;
            std::cout << std::string(60, '-') << std::endl;
            
            // 更新所有数据
            UpdateAllData();
            
            // 显示关键指标
            if (m_cpuInfo) {
                std::cout << "CPU: " << std::fixed << std::setprecision(1) 
                          << m_cpuInfo->GetTotalUsage() * 100 << "% | "
                          << m_cpuInfo->GetCurrentFrequency() << " MHz | "
                          << m_cpuInfo->GetTemperature() << "°C" << std::endl;
            }
            
            if (m_memoryInfo) {
                std::cout << "内存: " << std::fixed << std::setprecision(1) 
                          << m_memoryInfo->GetPhysicalMemoryUsage() * 100 << "% | "
                          << FormatBytes(m_memoryInfo->GetAvailablePhysicalMemory()) << " 可用" << std::endl;
            }
            
            if (m_gpuInfo) {
                std::cout << "GPU: " << std::fixed << std::setprecision(1) 
                          << m_gpuInfo->GetGpuUsage() * 100 << "% | "
                          << m_gpuInfo->GetTemperature() << "°C | "
                          << m_gpuInfo->GetPowerUsage() << " W" << std::endl;
            }
            
            if (m_batteryInfo && m_batteryInfo->IsBatteryPresent()) {
                std::cout << "电池: " << std::fixed << std::setprecision(0) 
                          << m_batteryInfo->GetChargePercentage() << "% | "
                          << m_batteryInfo->GetHealthPercentage() << "% 健康 | "
                          << m_batteryInfo->GetWattage() << " W" << std::endl;
            }
            
            if (m_networkInfo) {
                std::cout << "网络: " << std::fixed << std::setprecision(1) 
                          << m_networkInfo->GetCurrentDownloadSpeed() << " ↓ / "
                          << m_networkInfo->GetCurrentUploadSpeed() << " ↑ Mbps" << std::endl;
            }
            
            if (m_diskInfo) {
                std::cout << "磁盘: " << std::fixed << std::setprecision(1) 
                          << m_diskInfo->GetReadSpeed() << " ↓ / "
                          << m_diskInfo->GetWriteSpeed() << " ↑ MB/s" << std::endl;
            }
            
            // 等待下一次更新
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    
private:
    std::string FormatBytes(uint64_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unit < 4) {
            size /= 1024.0;
            unit++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << size << " " << units[unit];
        return oss.str();
    }
    
    std::string FormatUptime(uint64_t seconds) {
        uint64_t days = seconds / 86400;
        uint64_t hours = (seconds % 86400) / 3600;
        uint64_t minutes = (seconds % 3600) / 60;
        
        std::ostringstream oss;
        if (days > 0) {
            oss << days << "天 " << hours << "小时 " << minutes << "分钟";
        } else if (hours > 0) {
            oss << hours << "小时 " << minutes << "分钟";
        } else {
            oss << minutes << "分钟";
        }
        
        return oss.str();
    }
    
    std::string GetCurrentTimeString() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

int main() {
    std::cout << "跨平台硬件监控系统 v1.0" << std::endl;
    std::cout << "支持平台: macOS, Windows, Linux" << std::endl;
    std::cout << "编译信息: " << InfoFactory::GetBuildInfo() << std::endl;
    
    ComprehensiveMonitor monitor;
    
    // 初始化监控系统
    if (!monitor.Initialize()) {
        std::cerr << "监控系统初始化失败，退出..." << std::endl;
        return 1;
    }
    
    // 更新数据
    monitor.UpdateAllData();
    
    // 显示完整信息
    monitor.DisplaySystemInfo();
    monitor.DisplayCpuInfo();
    monitor.DisplayMemoryInfo();
    monitor.DisplayGpuInfo();
    monitor.DisplayBatteryInfo();
    monitor.DisplayTemperatureInfo();
    monitor.DisplayDiskInfo();
    monitor.DisplayNetworkInfo();
    
    // 询问用户是否进入实时监控模式
    std::cout << "\n是否进入实时监控模式? (y/n): ";
    char choice;
    std::cin >> choice;
    
    if (choice == 'y' || choice == 'Y') {
        std::cout << "请输入监控时长(秒，默认60): ";
        int duration = 60;
        std::cin >> duration;
        
        if (duration <= 0) {
            duration = 60;
        }
        
        monitor.RunRealTimeMonitoring(duration);
    }
    
    std::cout << "\n监控系统运行完成，感谢使用！" << std::endl;
    return 0;
}