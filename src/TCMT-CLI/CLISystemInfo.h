#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../core/DataStruct/DataStruct.h"

/**
 * CLI专用的GPU数据结构
 */
struct CLIGpuData {
    std::string name;
    std::string brand;
    uint64_t memory;
    double coreClock;
    bool isVirtual;
    
    CLIGpuData() : memory(0), coreClock(0.0), isVirtual(false) {}
};

/**
 * CLI专用的网络适配器数据结构
 */
struct CLINetworkAdapterData {
    std::string name;
    std::string mac;
    std::string ipAddress;
    std::string adapterType;
    uint64_t speed;
    
    CLINetworkAdapterData() : speed(0) {}
};

/**
 * CLI专用的磁盘数据结构
 */
struct CLIDiskData {
    char letter;
    std::string label;
    std::string fileSystem;
    uint64_t totalSize;
    uint64_t usedSpace;
    uint64_t freeSpace;
    int physicalDiskIndex;
    
    CLIDiskData() : letter('C'), totalSize(0), usedSpace(0), freeSpace(0), physicalDiskIndex(-1) {}
};

/**
 * CLI专用的温度数据结构
 */
struct CLITemperatureData {
    std::string sensorName;
    double temperature;
    
    CLITemperatureData() : temperature(-999.0) {}
    CLITemperatureData(const std::string& name, double temp) : sensorName(name), temperature(temp) {}
};

/**
 * CLI专用的USB设备数据结构
 */
struct CLIUSBDeviceData {
    std::string drivePath;
    std::string volumeLabel;
    uint64_t totalSize;
    uint64_t freeSpace;
    bool isUpdateReady;
    int state;
    SYSTEMTIME lastUpdate;
    
    CLIUSBDeviceData() : totalSize(0), freeSpace(0), isUpdateReady(false), state(0) {
        memset(&lastUpdate, 0, sizeof(lastUpdate));
    }
};

/**
 * CLI专用的TPM数据结构
 */
struct CLITpmData {
    bool hasTpm;
    std::string manufacturer;
    std::string manufacturerId;
    std::string version;
    std::string firmwareVersion;
    std::string status;
    bool isEnabled;
    bool isActivated;
    bool isOwned;
    bool isReady;
    bool tbsAvailable;
    bool physicalPresenceRequired;
    uint32_t specVersion;
    uint32_t tbsVersion;
    std::string errorMessage;
    std::string detectionMethod;
    
    CLITpmData() : hasTpm(false), isEnabled(false), isActivated(false), 
                   isOwned(false), isReady(false), tbsAvailable(false),
                   physicalPresenceRequired(false), specVersion(0), tbsVersion(0) {}
};

/**
 * CLI专用的物理磁盘SMART数据结构
 */
struct CLIPhysicalDiskSmartData {
    std::string model;
    std::string serialNumber;
    std::string firmwareVersion;
    std::string interfaceType;
    std::string diskType;
    uint64_t capacity;
    double temperature;
    uint8_t healthPercentage;
    bool isSystemDisk;
    bool smartEnabled;
    bool smartSupported;
    
    CLIPhysicalDiskSmartData() : capacity(0), temperature(-999.0), healthPercentage(0),
                                 isSystemDisk(false), smartEnabled(false), smartSupported(false) {}
};

/**
 * CLI专用的系统信息数据结构
 * 这是SharedMemoryReader读取的数据的CLI友好版本
 */
struct CLISystemInfo {
    
    bool connected;
    std::string lastUpdateTime;
    std::string connectionStatus;
    
    
    std::string cpuName;
    int physicalCores;
    int logicalCores;
    int performanceCores;
    int efficiencyCores;
    double cpuUsage;
    double cpuTemperature;
    double cpuBaseFrequencyMHz;
    double cpuCurrentFrequencyMHz;
    double cpuUsageSampleIntervalMs;
    bool hyperThreading;
    bool virtualization;
    
    
    uint64_t totalMemory;
    uint64_t usedMemory;
    uint64_t availableMemory;
    double memoryUsagePercent;
    
    
    std::vector<CLIGpuData> gpus;
    double gpuTemperature;
    
    
    std::vector<CLITemperatureData> temperatures;
    
    
    std::vector<CLIDiskData> disks;
    std::vector<CLIPhysicalDiskSmartData> physicalDisks;
    
    
    std::vector<CLINetworkAdapterData> adapters;
    
    
    std::vector<CLIUSBDeviceData> usbDevices;
    
    
    CLITpmData tpm;
    
    
    std::string osVersion;
    
    
    CLISystemInfo() : connected(false), physicalCores(0), logicalCores(0), 
                      performanceCores(0), efficiencyCores(0), cpuUsage(0.0),
                      cpuTemperature(-999.0), cpuBaseFrequencyMHz(0.0),
                      cpuCurrentFrequencyMHz(0.0), cpuUsageSampleIntervalMs(0.0),
                      hyperThreading(false), virtualization(false),
                      totalMemory(0), usedMemory(0), availableMemory(0),
                      memoryUsagePercent(0.0), gpuTemperature(-999.0) {}
};

/**
 * 共享内存数据解析器类
 * 负责将从共享内存读取的原始数据转换为CLI友好的数据结构
 */
class SharedMemoryDataParser {
public:
    /**
     * 将SystemInfo转换为CLISystemInfo
     * @param source 源数据（从共享内存读取的SystemInfo）
     * @param target 目标数据（CLI友好的CLISystemInfo）
     * @return 转换是否成功
     */
    static bool ConvertToCLISystemInfo(const SystemInfo& source, CLISystemInfo& target);
    
    /**
     * 解析GPU数据
     * @param source 源GPU数据向量
     * @return CLI友好的GPU数据向量
     */
    static std::vector<CLIGpuData> ParseGpuData(const std::vector<GPUData>& source);
    
    /**
     * 解析网络适配器数据
     * @param source 源网络适配器数据向量
     * @return CLI友好的网络适配器数据向量
     */
    static std::vector<CLINetworkAdapterData> ParseNetworkAdapterData(const std::vector<NetworkAdapterData>& source);
    
    /**
     * 解析磁盘数据
     * @param source 源磁盘数据向量
     * @return CLI友好的磁盘数据向量
     */
    static std::vector<CLIDiskData> ParseDiskData(const std::vector<DiskData>& source);
    
    /**
     * 解析温度数据
     * @param source 源温度数据向量
     * @return CLI友好的温度数据向量
     */
    static std::vector<CLITemperatureData> ParseTemperatureData(const std::vector<std::pair<std::string, double>>& source);
    
    /**
     * 解析USB设备数据
     * @param source 源USB设备数据向量
     * @return CLI友好的USB设备数据向量
     */
    static std::vector<CLIUSBDeviceData> ParseUSBDeviceData(const std::vector<USBDeviceInfo>& source);
    
    /**
     * 解析物理磁盘SMART数据
     * @param source 源物理磁盘数据向量
     * @return CLI友好的物理磁盘数据向量
     */
    static std::vector<CLIPhysicalDiskSmartData> ParsePhysicalDiskSmartData(const std::vector<PhysicalDiskSmartData>& source);
    
    /**
     * 解析TPM数据
     * @param source 源SystemInfo中的TPM信息
     * @return CLI友好的TPM数据
     */
    static CLITpmData ParseTpmData(const SystemInfo& source);
    
    /**
     * 格式化字节数为可读字符串
     * @param bytes 字节数
     * @return 格式化后的字符串
     */
    static std::string FormatBytes(uint64_t bytes);
    
    /**
     * 格式化频率为可读字符串
     * @param mhz 频率（MHz）
     * @return 格式化后的字符串
     */
    static std::string FormatFrequency(double mhz);
    
    /**
     * 格式化百分比为可读字符串
     * @param value 百分比值
     * @return 格式化后的字符串
     */
    static std::string FormatPercentage(double value);
    
    /**
     * 格式化温度为可读字符串
     * @param temperature 温度值
     * @return 格式化后的字符串
     */
    static std::string FormatTemperature(double temperature);
    
    /**
     * 格式化网络速度为可读字符串
     * @param bps 速度（比特每秒）
     * @return 格式化后的字符串
     */
    static std::string FormatNetworkSpeed(uint64_t bps);
    
    /**
     * 获取当前时间戳字符串
     * @return 时间戳字符串
     */
    static std::string GetCurrentTimestamp();

private:
    /**
     * 转换宽字符串到UTF-8字符串
     * @param wideStr 宽字符串
     * @return UTF-8字符串
     */
    static std::string WideStringToUTF8(const std::wstring& wideStr);
    
    /**
     * 解析宽字符数组为字符串
     * @param data 宽字符数组
     * @param maxLength 最大长度
     * @return 解析后的字符串
     */
    static std::string ParseWideString(const wchar_t* data, int maxLength);
};