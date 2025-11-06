#pragma once
#include "PlatformDefs.h"
#include <memory>
#include <string>
#include <vector>

// 前向声明
struct SystemInfoData;

// 数据结构定义
struct TemperatureSensor {
    std::string name;
    double temperature;
    std::string type;
    std::string location;
};

struct DiskInfo {
    std::string name;
    std::string model;
    std::string serialNumber;
    uint64_t totalSpace;
    uint64_t freeSpace;
    uint64_t usedSpace;
    double usagePercentage;
    std::string fileSystem;
    double readSpeed;
    double writeSpeed;
    uint64_t totalReadBytes;
    uint64_t totalWriteBytes;
    bool isHealthy;
    std::string healthStatus;
    uint32_t powerOnHours;
    uint32_t startStopCount;
    uint32_t reallocatedSectors;
    double temperature;
    uint64_t remainingBlocks;
    bool isActive;
    double diskUtilization;
    uint32_t activeProcesses;
    bool isEncrypted;
    std::string encryptionType;
};

struct NetworkInterface {
    std::string name;
    std::string description;
    std::string type;
    std::string status;
    std::string macAddress;
    std::vector<std::string> ipAddresses;
    double downloadSpeed;
    double uploadSpeed;
    uint64_t totalRxBytes;
    uint64_t totalTxBytes;
    std::string ssid;
    double signalStrength;
    uint32_t channel;
    std::string securityType;
    bool isConnected;
    double latency;
    double packetLoss;
    std::string networkQuality;
};

struct BatteryCell {
    uint32_t cellIndex;
    double voltage;
    double temperature;
    double capacity;
    bool isHealthy;
};

struct BatteryInfo {
    double currentCapacity;
    double maxCapacity;
    double designCapacity;
    int cycleCount;
    bool isCharging;
    bool isPresent;
    double voltage;
    double current;
    double temperature;
    std::string healthStatus;
    int timeToEmpty;
    int timeToFullCharge;
    std::string powerSourceState;
    double powerConsumption;
    double healthPercentage;
    std::string batterySerial;
    std::string manufacturingDate;
    double batteryWearLevel;
    bool isCalibrated;
    uint32_t powerOnTime;
    std::vector<std::string> batteryWarnings;
};

// 基础信息接口类
class IBaseInfo {
public:
    virtual ~IBaseInfo() = default;
    
    // 初始化和清理
    virtual bool Initialize() = 0;
    virtual void Cleanup() = 0;
    virtual bool IsInitialized() const = 0;
    
    // 更新数据
    virtual bool Update() = 0;
    virtual bool IsDataValid() const = 0;
    virtual uint64_t GetLastUpdateTime() const = 0;
    
    // 错误处理
    virtual std::string GetLastError() const = 0;
    virtual void ClearError() = 0;
};

// CPU信息接口
class ICpuInfo : public IBaseInfo {
public:
    virtual ~ICpuInfo() = default;
    
    // 基本CPU信息
    virtual std::string GetName() const = 0;
    virtual std::string GetVendor() const = 0;
    virtual std::string GetArchitecture() const = 0;
    virtual uint32_t GetTotalCores() const = 0;
    virtual uint32_t GetLogicalCores() const = 0;
    virtual uint32_t GetPhysicalCores() const = 0;
    
    // 核心类型信息 (支持大小核架构)
    virtual uint32_t GetPerformanceCores() const = 0;
    virtual uint32_t GetEfficiencyCores() const = 0;
    virtual bool HasHybridArchitecture() const = 0;
    
    // CPU使用率
    virtual double GetTotalUsage() const = 0;
    virtual std::vector<double> GetPerCoreUsage() const = 0;
    virtual double GetPerformanceCoreUsage() const = 0;
    virtual double GetEfficiencyCoreUsage() const = 0;
    
    // 频率信息
    virtual double GetCurrentFrequency() const = 0;
    virtual double GetBaseFrequency() const = 0;
    virtual double GetMaxFrequency() const = 0;
    virtual std::vector<double> GetPerCoreFrequencies() const = 0;
    
    // 特性支持
    virtual bool IsHyperThreadingEnabled() const = 0;
    virtual bool IsVirtualizationEnabled() const = 0;
    virtual bool SupportsAVX() const = 0;
    virtual bool SupportsAVX2() const = 0;
    virtual bool SupportsAVX512() const = 0;
    
    // 温度信息
    virtual double GetTemperature() const = 0;
    virtual std::vector<double> GetPerCoreTemperatures() const = 0;
    
    // 功耗信息
    virtual double GetPowerUsage() const = 0;
    virtual double GetPackagePower() const = 0;
    virtual double GetCorePower() const = 0;
};

// 内存信息接口
class IMemoryInfo : public IBaseInfo {
public:
    virtual ~IMemoryInfo() = default;
    
    // 物理内存信息
    virtual uint64_t GetTotalPhysicalMemory() const = 0;
    virtual uint64_t GetAvailablePhysicalMemory() const = 0;
    virtual uint64_t GetUsedPhysicalMemory() const = 0;
    virtual double GetPhysicalMemoryUsage() const = 0;
    
    // 虚拟内存信息
    virtual uint64_t GetTotalVirtualMemory() const = 0;
    virtual uint64_t GetAvailableVirtualMemory() const = 0;
    virtual uint64_t GetUsedVirtualMemory() const = 0;
    virtual double GetVirtualMemoryUsage() const = 0;
    
    // 交换文件/页面文件信息
    virtual uint64_t GetTotalSwapMemory() const = 0;
    virtual uint64_t GetAvailableSwapMemory() const = 0;
    virtual uint64_t GetUsedSwapMemory() const = 0;
    virtual double GetSwapMemoryUsage() const = 0;
    
    // 内存速度信息
    virtual double GetMemorySpeed() const = 0;
    virtual std::string GetMemoryType() const = 0;
    virtual uint32_t GetMemoryChannels() const = 0;
    
    // 内存使用详情
    virtual uint64_t GetCachedMemory() const = 0;
    virtual uint64_t GetBufferedMemory() const = 0;
    virtual uint64_t GetSharedMemory() const = 0;
    
    // 内存压力
    virtual double GetMemoryPressure() const = 0;
    virtual bool IsMemoryLow() const = 0;
    virtual bool IsMemoryCritical() const = 0;
};

// GPU信息接口
class IGpuInfo : public IBaseInfo {
public:
    virtual ~IGpuInfo() = default;
    
    // 基本GPU信息
    virtual std::string GetName() const = 0;
    virtual std::string GetVendor() const = 0;
    virtual std::string GetDriverVersion() const = 0;
    virtual uint64_t GetDedicatedMemory() const = 0;
    virtual uint64_t GetSharedMemory() const = 0;
    
    // GPU使用率
    virtual double GetGpuUsage() const = 0;
    virtual double GetMemoryUsage() const = 0;
    virtual double GetVideoDecoderUsage() const = 0;
    virtual double GetVideoEncoderUsage() const = 0;
    
    // 频率信息
    virtual double GetCurrentFrequency() const = 0;
    virtual double GetBaseFrequency() const = 0;
    virtual double GetMaxFrequency() const = 0;
    virtual double GetMemoryFrequency() const = 0;
    
    // 温度信息
    virtual double GetTemperature() const = 0;
    virtual double GetHotspotTemperature() const = 0;
    virtual double GetMemoryTemperature() const = 0;
    
    // 功耗信息
    virtual double GetPowerUsage() const = 0;
    virtual double GetBoardPower() const = 0;
    virtual double GetMaxPowerLimit() const = 0;
    
    // 风扇信息
    virtual double GetFanSpeed() const = 0;
    virtual double GetFanSpeedPercent() const = 0;
    
    // 性能指标
    virtual uint64_t GetComputeUnits() const = 0;
    virtual std::string GetArchitecture() const = 0;
    virtual double GetPerformanceRating() const = 0;
};

// 网络适配器接口
class INetworkAdapter : public IBaseInfo {
public:
    virtual ~INetworkAdapter() = default;
    
    // 基本网络信息
    virtual std::string GetName() const = 0;
    virtual std::string GetDescription() const = 0;
    virtual std::string GetMacAddress() const = 0;
    virtual std::vector<std::string> GetIpAddresses() const = 0;
    virtual bool IsConnected() const = 0;
    
    // 网络速度
    virtual double GetUploadSpeed() const = 0;
    virtual double GetDownloadSpeed() const = 0;
    virtual uint64_t GetTotalBytesSent() const = 0;
    virtual uint64_t GetTotalBytesReceived() const = 0;
    
    // 网络状态
    virtual std::string GetConnectionType() const = 0; // WiFi, Ethernet, etc.
    virtual std::string GetStatus() const = 0;
    virtual int GetSignalStrength() const = 0; // for WiFi
    
    // 网络配置
    virtual std::string GetDnsServer() const = 0;
    virtual std::string GetGateway() const = 0;
    virtual std::string GetSubnetMask() const = 0;
    
    // 错误统计
    virtual uint64_t GetPacketsSent() const = 0;
    virtual uint64_t GetPacketsReceived() const = 0;
    virtual uint64_t GetErrorsSent() const = 0;
    virtual uint64_t GetErrorsReceived() const = 0;
    virtual uint64_t GetPacketsDropped() const = 0;
};

// 磁盘信息接口
class IDiskInfo : public IBaseInfo {
public:
    virtual ~IDiskInfo() = default;
    
    // 基本磁盘信息
    virtual std::string GetName() const = 0;
    virtual std::string GetModel() const = 0;
    virtual std::string GetSerialNumber() const = 0;
    virtual uint64_t GetTotalSize() const = 0;
    virtual uint64_t GetAvailableSpace() const = 0;
    virtual uint64_t GetUsedSpace() const = 0;
    
    // 磁盘使用率
    virtual double GetUsagePercentage() const = 0;
    virtual std::string GetFileSystem() const = 0;
    
    // 磁盘性能
    virtual double GetReadSpeed() const = 0;
    virtual double GetWriteSpeed() const = 0;
    virtual uint64_t GetTotalReadBytes() const = 0;
    virtual uint64_t GetTotalWriteBytes() const = 0;
    
    // 磁盘健康状态
    virtual bool IsHealthy() const = 0;
    virtual int GetHealthPercentage() const = 0;
    virtual std::string GetHealthStatus() const = 0;
    virtual uint64_t GetPowerOnHours() const = 0;
    
    // SMART信息
    virtual std::vector<std::pair<std::string, std::string>> GetSmartAttributes() const = 0;
    virtual int GetReallocatedSectorCount() const = 0;
    virtual int GetPendingSectorCount() const = 0;
    virtual int GetUncorrectableSectorCount() const = 0;
    
    // 磁盘类型
    virtual bool IsSSD() const = 0;
    virtual bool IsHDD() const = 0;
    virtual bool IsNVMe() const = 0;
    virtual std::string GetInterfaceType() const = 0; // SATA, NVMe, USB, etc.
};

// 温度监控接口
class ITemperatureMonitor : public IBaseInfo {
public:
    virtual ~ITemperatureMonitor() = default;
    
    // 温度信息
    virtual std::vector<std::pair<std::string, double>> GetAllTemperatures() const = 0;
    virtual double GetCpuTemperature() const = 0;
    virtual double GetGpuTemperature() const = 0;
    virtual double GetMotherboardTemperature() const = 0;
    virtual double GetAverageTemperature() const = 0;
    
    // 温度阈值
    virtual double GetMaxSafeTemperature() const = 0;
    virtual double GetCriticalTemperature() const = 0;
    virtual bool IsOverheating() const = 0;
    virtual bool IsCriticalTemperature() const = 0;
    
    // 传感器信息
    virtual std::vector<std::string> GetAvailableSensors() const = 0;
    virtual std::string GetSensorType(const std::string& sensorName) const = 0;
};

// TPM信息接口
class ITpmInfo : public IBaseInfo {
public:
    virtual ~ITpmInfo() = default;
    
    // TPM基本信息
    virtual bool IsPresent() const = 0;
    virtual std::string GetVersion() const = 0;
    virtual std::string GetManufacturer() const = 0;
    virtual std::string GetSpecificationVersion() const = 0;
    
    // TPM状态
    virtual bool IsEnabled() const = 0;
    virtual bool IsActivated() const = 0;
    virtual bool IsOwned() const = 0;
    virtual std::string GetStatus() const = 0;
    
    // TPM功能
    virtual bool SupportsPCRs() const = 0;
    virtual bool SupportsAttestation() const = 0;
    virtual bool SupportsSealing() const = 0;
    virtual int GetAlgorithmCount() const = 0;
    
    // 安全特性
    virtual std::vector<std::string> GetSupportedAlgorithms() const = 0;
    virtual std::string GetSelectedAlgorithm() const = 0;
    virtual int GetPCRCOUNT() const = 0;
};

// USB设备监控接口
class IUsbMonitor : public IBaseInfo {
public:
    virtual ~IUsbMonitor() = default;
    
    // USB设备列表
    virtual std::vector<std::pair<std::string, std::string>> GetConnectedDevices() const = 0;
    virtual int GetDeviceCount() const = 0;
    
    // 设备信息
    virtual std::string GetDeviceName(const std::string& deviceId) const = 0;
    virtual std::string GetDeviceVendor(const std::string& deviceId) const = 0;
    virtual std::string GetDeviceType(const std::string& deviceId) const = 0;
    virtual std::string GetDeviceVersion(const std::string& deviceId) const = 0;
    
    // 设备状态
    virtual bool IsDeviceActive(const std::string& deviceId) const = 0;
    virtual uint64_t GetDevicePowerUsage(const std::string& deviceId) const = 0;
    virtual std::string GetDeviceStatus(const std::string& deviceId) const = 0;
    
    // 事件监听
    virtual bool StartMonitoring() = 0;
    virtual void StopMonitoring() = 0;
    virtual bool IsMonitoring() const = 0;
};

// 操作系统信息接口
class IOSInfo : public IBaseInfo {
public:
    virtual ~IOSInfo() = default;
    
    // 系统基本信息
    virtual std::string GetName() const = 0;
    virtual std::string GetVersion() const = 0;
    virtual std::string GetBuildNumber() const = 0;
    virtual std::string GetArchitecture() const = 0;
    virtual std::string GetServicePack() const = 0;
    
    // 系统状态
    virtual uint64_t GetUptime() const = 0;
    virtual std::string GetBootTime() const = 0;
    virtual int GetProcessCount() const = 0;
    virtual int GetThreadCount() const = 0;
    virtual int GetHandleCount() const = 0;
    
    // 系统负载
    virtual double GetCpuLoad() const = 0;
    virtual std::vector<double> GetLoadAverages() const = 0; // 1min, 5min, 15min
    virtual int GetRunningProcesses() const = 0;
    
    // 系统配置
    virtual std::string GetComputerName() const = 0;
    virtual std::string GetUserName() const = 0;
    virtual std::string GetDomainName() const = 0;
    virtual std::string GetTimeZone() const = 0;
    
    // 系统特性
    virtual bool Is64Bit() const = 0;
    virtual bool IsServer() const = 0;
    virtual bool IsVirtualMachine() const = 0;
    virtual std::string GetHypervisor() const = 0;
    
    // 系统更新
    virtual std::string GetLastSystemUpdateTime() const = 0;
    virtual bool IsUpToDate() const = 0;
    virtual std::string GetUpdateStatus() const = 0;
};

class ISystemInfo : public IBaseInfo {
public:
    virtual ~ISystemInfo() = default;
    
    // 操作系统信息
    virtual std::string GetOSName() const = 0;
    virtual std::string GetOSVersion() const = 0;
    virtual std::string GetOSBuild() const = 0;
    virtual std::string GetArchitecture() const = 0;
    virtual std::string GetHostname() const = 0;
    virtual std::string GetDomain() const = 0;
    
    // 系统时间信息
    virtual uint64_t GetUptimeSeconds() const = 0;
    virtual std::string GetBootTime() const = 0;
    virtual std::string GetLocalTime() const = 0;
    virtual std::string GetUTCTime() const = 0;
    virtual std::string GetTimezone() const = 0;
    
    // 系统负载
    virtual double GetLoadAverage1Min() const = 0;
    virtual double GetLoadAverage5Min() const = 0;
    virtual double GetLoadAverage15Min() const = 0;
    virtual double GetCPULoadAverage() const = 0;
    
    // 进程信息
    virtual uint32_t GetProcessCount() const = 0;
    virtual uint32_t GetRunningProcessCount() const = 0;
    virtual uint32_t GetSleepingProcessCount() const = 0;
    virtual uint32_t GetThreadCount() const = 0;
    virtual uint32_t GetMaxProcesses() const = 0;
    
    // 内存信息
    virtual uint64_t GetTotalMemory() const = 0;
    virtual uint64_t GetAvailableMemory() const = 0;
    virtual uint64_t GetUsedMemory() const = 0;
    virtual uint64_t GetCacheMemory() const = 0;
    virtual uint64_t GetSwapMemory() const = 0;
    virtual double GetMemoryUsagePercentage() const = 0;
    virtual double GetMemoryPressure() const = 0;
    
    // 磁盘信息
    virtual uint64_t GetTotalDiskSpace() const = 0;
    virtual uint64_t GetAvailableDiskSpace() const = 0;
    virtual uint64_t GetUsedDiskSpace() const = 0;
    virtual double GetDiskUsagePercentage() const = 0;
    virtual uint32_t GetDiskReadOps() const = 0;
    virtual uint32_t GetDiskWriteOps() const = 0;
    virtual uint64_t GetDiskReadBytes() const = 0;
    virtual uint64_t GetDiskWriteBytes() const = 0;
    
    // 网络信息
    virtual uint32_t GetNetworkInterfaceCount() const = 0;
    virtual uint64_t GetTotalBytesReceived() const = 0;
    virtual uint64_t GetTotalBytesSent() const = 0;
    virtual double GetNetworkUtilization() const = 0;
    
    // 系统状态
    virtual bool IsSystemHealthy() const = 0;
    virtual std::string GetSystemStatus() const = 0;
    virtual std::vector<std::string> GetSystemWarnings() const = 0;
    virtual std::vector<std::string> GetSystemErrors() const = 0;
    virtual double GetSystemHealthScore() const = 0;
    
    // 安全信息
    virtual bool IsSecureBootEnabled() const = 0;
    virtual bool IsFirewallEnabled() const = 0;
    virtual bool IsAntivirusRunning() const = 0;
    virtual std::string GetSecurityStatus() const = 0;
    
    // 硬件信息
    virtual std::string GetMotherboardModel() const = 0;
    virtual std::string GetBIOSVersion() const = 0;
    virtual std::string GetFirmwareVersion() const = 0;
    virtual std::string GetSerialNumber() const = 0;
    
    // 虚拟化信息
    virtual bool IsVirtualMachine() const = 0;
    virtual std::string GetVirtualizationPlatform() const = 0;
    virtual uint32_t GetVirtualCPUCount() const = 0;
    virtual uint64_t GetVirtualMemory() const = 0;
    
    // 环境变量
    virtual std::vector<std::string> GetEnvironmentVariables() const = 0;
    virtual std::string GetEnvironmentVariable(const std::string& name) const = 0;
    
    // 系统更新
    virtual std::string GetLastSystemUpdateTime() const = 0;
    virtual bool UpdatesAvailable() const = 0;
    virtual std::vector<std::string> GetPendingUpdates() const = 0;
};

// 新增接口
class ITemperatureInfo : public IBaseInfo {
public:
    virtual ~ITemperatureInfo() = default;
    
    // CPU温度
    virtual double GetCPUTemperature() const = 0;
    virtual double GetCpuMaxTemperature() const = 0;
    virtual double GetCpuMinTemperature() const = 0;
    
    // GPU温度
    virtual double GetGPUTemperature() const = 0;
    virtual double GetGpuMaxTemperature() const = 0;
    
    // 系统温度
    virtual double GetSystemTemperature() const = 0;
    virtual double GetAmbientTemperature() const = 0;
    
    // 存储温度
    virtual double GetSSDTemperature() const = 0;
    virtual std::vector<std::string> GetHDDTemperatures() const = 0;
    
    // 温度传感器列表
    virtual std::vector<TemperatureSensor> GetAllSensors() const = 0;
    virtual size_t GetSensorCount() const = 0;
    
    // 温度状态
    virtual bool IsOverheating() const = 0;
    virtual bool IsThermalThrottling() const = 0;
    virtual double GetThermalPressure() const = 0;
    
    // 温度历史
    virtual std::vector<double> GetTemperatureHistory(int minutes = 60) const = 0;
    virtual double GetAverageTemperature(int minutes = 10) const = 0;
    
    // 温度预警
    virtual std::vector<std::string> GetTemperatureWarnings() const = 0;
    virtual bool HasHighTemperatureAlert() const = 0;
};



class INetworkInfo : public IBaseInfo {
public:
    virtual ~INetworkInfo() = default;
    
    // 基本网络信息
    virtual size_t GetInterfaceCount() const = 0;
    virtual std::vector<NetworkInterface> GetAllInterfaces() const = 0;
    virtual NetworkInterface GetInterfaceByIndex(size_t index) const = 0;
    virtual NetworkInterface GetInterfaceByName(const std::string& name) const = 0;
    virtual NetworkInterface GetPrimaryInterface() const = 0;
    
    // 网络流量统计
    virtual uint64_t GetTotalRxBytes() const = 0;
    virtual uint64_t GetTotalTxBytes() const = 0;
    virtual double GetCurrentDownloadSpeed() const = 0; // Mbps
    virtual double GetCurrentUploadSpeed() const = 0; // Mbps
    virtual double GetAverageDownloadSpeed(int minutes = 5) const = 0;
    virtual double GetAverageUploadSpeed(int minutes = 5) const = 0;
    
    // 连接状态
    virtual bool IsConnected() const = 0;
    virtual bool IsInternetAvailable() const = 0;
    virtual std::string GetConnectionType() const = 0; // WiFi, Ethernet, Cellular
    virtual std::string GetConnectionStatus() const = 0; // Connected, Disconnected, etc.
    
    // WiFi特定信息
    virtual std::string GetCurrentSSID() const = 0;
    virtual double GetSignalStrength() const = 0; // dBm
    virtual uint32_t GetChannel() const = 0;
    virtual std::string GetSecurityType() const = 0;
    virtual std::vector<std::string> GetAvailableNetworks() const = 0;
    
    // 网络质量
    virtual double GetLatency() const = 0; // ms
    virtual double GetPacketLoss() const = 0; // percentage
    virtual std::string GetNetworkQuality() const = 0; // Excellent, Good, Fair, Poor
    virtual bool HasNetworkIssues() const = 0;
    
    // 带宽使用
    virtual double GetBandwidthUsage() const = 0; // percentage
    virtual double GetMaxBandwidth() const = 0; // Mbps
    virtual std::vector<std::string> GetNetworkWarnings() const = 0;
    
    // 历史数据
    virtual std::vector<std::pair<uint64_t, double>> GetDownloadHistory(int minutes = 60) const = 0;
    virtual std::vector<std::pair<uint64_t, double>> GetUploadHistory(int minutes = 60) const = 0;
    virtual uint64_t GetDataUsageToday() const = 0;
    virtual uint64_t GetDataUsageThisMonth() const = 0;
};

class IBatteryInfo : public IBaseInfo {
public:
    virtual ~IBatteryInfo() = default;
    
    // 基本电池信息
    virtual bool IsBatteryPresent() const = 0;
    virtual bool IsCharging() const = 0;
    virtual bool IsACPowered() const = 0;
    virtual std::string GetBatteryType() const = 0;
    virtual std::string GetBatteryModel() const = 0;
    virtual std::string GetBatteryManufacturer() const = 0;
    virtual std::string GetBatterySerialNumber() const = 0;
    
    // 容量和电量信息
    virtual uint32_t GetCurrentCapacity() const = 0;
    virtual uint32_t GetMaxCapacity() const = 0;
    virtual uint32_t GetDesignCapacity() const = 0;
    virtual uint32_t GetNominalCapacity() const = 0;
    
    virtual double GetChargePercentage() const = 0;
    virtual double GetHealthPercentage() const = 0;
    virtual double GetDesignHealthPercentage() const = 0;
    
    // 电压和电流信息
    virtual double GetVoltage() const = 0;
    virtual double GetAmperage() const = 0;
    virtual double GetWattage() const = 0;
    
    // 时间信息
    virtual uint32_t GetTimeToFullCharge() const = 0;
    virtual uint32_t GetTimeToEmpty() const = 0;
    virtual uint32_t GetTimeRemaining() const = 0;
    
    // 循环计数
    virtual uint32_t GetCycleCount() const = 0;
    virtual uint32_t GetCycleCountLimit() const = 0;
    virtual double GetCycleCountPercentage() const = 0;
    
    // 温度信息
    virtual double GetTemperature() const = 0;
    virtual std::vector<BatteryCell> GetCellInfo() const = 0;
    
    // 电源管理信息
    virtual std::string GetPowerSourceState() const = 0;
    virtual bool IsPowerSavingMode() const = 0;
    virtual bool IsOptimizedBatteryCharging() const = 0;
    
    // 扩展功能
    virtual BatteryInfo GetDetailedBatteryInfo() const = 0;
    virtual std::vector<std::string> GetBatteryWarnings() const = 0;
    virtual double GetBatteryWearLevel() const = 0;
    virtual std::string GetBatterySerial() const = 0;
    virtual std::string GetManufacturingDate() const = 0;
    virtual uint32_t GetPowerOnTime() const = 0;
    virtual bool IsBatteryCalibrated() const = 0;
    virtual std::string GetChargingState() const = 0;
    
    // 电池状态
    virtual bool IsBatteryHealthy() const = 0;
    virtual std::string GetBatteryHealthStatus() const = 0;
    virtual std::vector<std::string> GetWarnings() const = 0;
    virtual std::vector<std::string> GetErrors() const = 0;
    
    // 估算信息
    virtual double GetEstimatedRuntime() const = 0;
    virtual double GetEstimatedChargingTime() const = 0;
};

// 数据收集器接口
class IDataCollector : public IBaseInfo {
public:
    virtual ~IDataCollector() = default;
    
    // 数据收集控制
    virtual bool StartCollection() = 0;
    virtual void StopCollection() = 0;
    virtual bool IsCollecting() const = 0;
    
    // 数据更新
    virtual bool UpdateAllData() = 0;
    virtual bool UpdateComponentData(const std::string& component) = 0;
    
    // 数据访问
    virtual std::shared_ptr<ICpuInfo> GetCpuInfo() const = 0;
    virtual std::shared_ptr<IMemoryInfo> GetMemoryInfo() const = 0;
    virtual std::shared_ptr<IGpuInfo> GetGpuInfo() const = 0;
    virtual std::shared_ptr<INetworkAdapter> GetNetworkAdapter(const std::string& name) const = 0;
    virtual std::shared_ptr<IDiskInfo> GetDiskInfo(const std::string& name) const = 0;
    virtual std::shared_ptr<ITemperatureMonitor> GetTemperatureMonitor() const = 0;
    virtual std::shared_ptr<ITpmInfo> GetTpmInfo() const = 0;
    virtual std::shared_ptr<IUsbMonitor> GetUsbMonitor() const = 0;
    virtual std::shared_ptr<IOSInfo> GetOSInfo() const = 0;
    virtual std::shared_ptr<ISystemInfo> GetSystemInfo() const = 0;
    virtual std::shared_ptr<IBatteryInfo> GetBatteryInfo() const = 0;
    
    // 数据导出
    virtual bool ExportToStruct(SystemInfoData& data) const = 0;
    virtual bool ExportToJson(std::string& json) const = 0;
    virtual bool ExportToXml(std::string& xml) const = 0;
    
    // 配置管理
    virtual bool LoadConfiguration(const std::string& configPath) = 0;
    virtual bool SaveConfiguration(const std::string& configPath) const = 0;
    virtual void SetUpdateInterval(uint32_t intervalMs) = 0;
    virtual uint32_t GetUpdateInterval() const = 0;
};