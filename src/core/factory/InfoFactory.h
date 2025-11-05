#pragma once
#include "../common/BaseInfo.h"
#include "../common/PlatformDefs.h"
#include "../cpu/CpuInfo.h"
#include <memory>
#include <string>
#include <vector>

#ifdef PLATFORM_MACOS
class MacMemoryAdapter;
class MacGpuAdapter;
class MacSystemInfo;
class MacBatteryInfo;
#endif

class InfoFactory {
public:
    // 创建CPU信息实例
    static std::unique_ptr<ICpuInfo> CreateCpuInfo();
    
    // 创建内存信息实例
    static std::unique_ptr<IMemoryInfo> CreateMemoryInfo();
    
    // 创建GPU信息实例
    static std::unique_ptr<IGpuInfo> CreateGpuInfo();
    
    // 创建网络适配器实例
    static std::unique_ptr<INetworkAdapter> CreateNetworkAdapter(const std::string& adapterName = "");
    
    // 创建磁盘信息实例
    static std::unique_ptr<IDiskInfo> CreateDiskInfo(const std::string& diskName = "");
    
    // 创建温度监控实例
    static std::unique_ptr<ITemperatureMonitor> CreateTemperatureMonitor();
    
    // 创建TPM信息实例
    static std::unique_ptr<ITpmInfo> CreateTpmInfo();
    
    // 创建USB监控实例
    static std::unique_ptr<IUsbMonitor> CreateUsbMonitor();
    
    // 创建操作系统信息实例
    static std::unique_ptr<IOSInfo> CreateOSInfo();
    
    // 创建系统信息监控实例
    static std::unique_ptr<ISystemInfo> CreateSystemInfo();
    
    // 创建电池信息监控实例
    static std::unique_ptr<IBatteryInfo> CreateBatteryInfo();
    
    // 创建数据收集器实例
    static std::unique_ptr<IDataCollector> CreateDataCollector();
    
    // 获取所有可用的网络适配器
    static std::vector<std::string> GetAvailableNetworkAdapters();
    
    // 获取所有可用的磁盘
    static std::vector<std::string> GetAvailableDisks();
    
    // 检查特定功能是否可用
    static bool IsGpuMonitoringAvailable();
    static bool IsTpmAvailable();
    static bool IsUsbMonitoringAvailable();
    static bool IsTemperatureMonitoringAvailable();
    
    // 获取平台特定信息
    static std::string GetPlatformName();
    static std::string GetCompilerName();
    static std::string GetArchitectureName();
    static std::string GetBuildInfo();
    
    // 工厂配置
    static void SetDefaultUpdateInterval(uint32_t intervalMs);
    static uint32_t GetDefaultUpdateInterval();
    
    // 错误处理
    static std::string GetLastError();
    static void ClearError();
    
private:
    // 平台特定的创建函数
    #ifdef PLATFORM_WINDOWS
        static std::unique_ptr<ICpuInfo> CreateWinCpuInfo();
        static std::unique_ptr<IMemoryInfo> CreateWinMemoryInfo();
        static std::unique_ptr<IGpuInfo> CreateWinGpuInfo();
        static std::unique_ptr<INetworkAdapter> CreateWinNetworkAdapter(const std::string& adapterName);
        static std::unique_ptr<IDiskInfo> CreateWinDiskInfo(const std::string& diskName);
        static std::unique_ptr<ITemperatureMonitor> CreateWinTemperatureMonitor();
        static std::unique_ptr<ITpmInfo> CreateWinTpmInfo();
        static std::unique_ptr<IUsbMonitor> CreateWinUsbMonitor();
        static std::unique_ptr<IOSInfo> CreateWinOSInfo();
        static std::unique_ptr<IDataCollector> CreateWinDataCollector();
    #elif defined(PLATFORM_MACOS)
        static std::unique_ptr<ICpuInfo> CreateMacCpuInfo();
        static std::unique_ptr<ICpuAdapter> CreateMacCpuAdapter();
        static std::unique_ptr<IMemoryInfo> CreateMacMemoryInfo();
        static std::unique_ptr<IGpuInfo> CreateMacGpuInfo();
        static std::unique_ptr<INetworkAdapter> CreateMacNetworkAdapter(const std::string& adapterName);
        static std::unique_ptr<IDiskInfo> CreateMacDiskInfo(const std::string& diskName);
        static std::unique_ptr<ITemperatureMonitor> CreateMacTemperatureMonitor();
        static std::unique_ptr<ITpmInfo> CreateMacTpmInfo();
        static std::unique_ptr<IUsbMonitor> CreateMacUsbMonitor();
        static std::unique_ptr<IOSInfo> CreateMacOSInfo();
        static std::unique_ptr<ISystemInfo> CreateMacSystemInfo();
        static std::unique_ptr<IBatteryInfo> CreateMacBatteryInfo();
        static std::unique_ptr<IDataCollector> CreateMacDataCollector();
    #elif defined(PLATFORM_LINUX)
        static std::unique_ptr<ICpuInfo> CreateLinuxCpuInfo();
        static std::unique_ptr<IMemoryInfo> CreateLinuxMemoryInfo();
        static std::unique_ptr<IGpuInfo> CreateLinuxGpuInfo();
        static std::unique_ptr<INetworkAdapter> CreateLinuxNetworkAdapter(const std::string& adapterName);
        static std::unique_ptr<IDiskInfo> CreateLinuxDiskInfo(const std::string& diskName);
        static std::unique_ptr<ITemperatureMonitor> CreateLinuxTemperatureMonitor();
        static std::unique_ptr<ITpmInfo> CreateLinuxTpmInfo();
        static std::unique_ptr<IUsbMonitor> CreateLinuxUsbMonitor();
        static std::unique_ptr<IOSInfo> CreateLinuxOSInfo();
        static std::unique_ptr<IDataCollector> CreateLinuxDataCollector();
    #endif

    // 兼容性支持：创建原有类实例
    static std::unique_ptr<CpuInfo> CreateLegacyCpuInfo();
    static std::unique_ptr<ICpuAdapter> CreateCpuAdapter();
    
    // macOS特定适配器
    static std::unique_ptr<MacMemoryAdapter> CreateMacMemoryAdapter();
    static std::unique_ptr<MacGpuAdapter> CreateMacGpuAdapter();
    
    // 错误处理
    static std::string lastError;
    
    // 配置
    static uint32_t defaultUpdateInterval;
    
    // 辅助函数
    static void SetError(const std::string& error);
    static bool ValidateComponentName(const std::string& name);
    static std::string SanitizeComponentName(const std::string& name);
};