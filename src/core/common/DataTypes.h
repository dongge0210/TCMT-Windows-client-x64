#pragma once
#include "PlatformDefs.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>

// 基础数据类型定义
namespace TCMT {
    // 时间戳类型
    using TimeStamp = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::steady_clock::duration;
    
    // 字符串类型
    #ifdef PLATFORM_WINDOWS
        using String = std::wstring;
        using Char = wchar_t;
        #define STR(x) L ## x
        #define TO_STR(str) WinUtils::Utf8ToWstring(str)
        #define FROM_STR(wstr) WinUtils::WstringToUtf8(wstr)
    #else
        using String = std::string;
        using Char = char;
        #define STR(x) x
        #define TO_STR(str) (str)
        #define FROM_STR(str) (str)
    #endif
    
    // 容器类型
    template<typename T>
    using Vector = std::vector<T>;
    
    template<typename K, typename V>
    using Map = std::map<K, V>;
    
    template<typename T>
    using UniquePtr = std::unique_ptr<T>;
    
    template<typename T>
    using SharedPtr = std::shared_ptr<T>;
    
    // 数值类型
    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;
    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;
    
    using float32 = float;
    using float64 = double;
    
    // 系统信息数据结构
    struct SystemInfo {
        TimeStamp timestamp;
        String computerName;
        String osName;
        String osVersion;
        String architecture;
        uint64 uptime;
        uint32 processCount;
        uint32 threadCount;
    };
    
    // CPU信息数据结构
    struct CpuInfo {
        String name;
        String vendor;
        String architecture;
        uint32 totalCores;
        uint32 logicalCores;
        uint32 physicalCores;
        uint32 performanceCores;
        uint32 efficiencyCores;
        bool hasHybridArchitecture;
        double totalUsage;
        Vector<double> perCoreUsage;
        double currentFrequency;
        double baseFrequency;
        double maxFrequency;
        Vector<double> perCoreFrequencies;
        bool isHyperThreadingEnabled;
        bool isVirtualizationEnabled;
        double temperature;
        Vector<double> perCoreTemperatures;
        double powerUsage;
    };
    
    // 内存信息数据结构
    struct MemoryInfo {
        uint64 totalPhysical;
        uint64 availablePhysical;
        uint64 usedPhysical;
        double physicalUsage;
        uint64 totalVirtual;
        uint64 availableVirtual;
        uint64 usedVirtual;
        double virtualUsage;
        uint64 totalSwap;
        uint64 availableSwap;
        uint64 usedSwap;
        double swapUsage;
        double memorySpeed;
        String memoryType;
        uint32 memoryChannels;
        uint64 cachedMemory;
        uint64 bufferedMemory;
        uint64 sharedMemory;
        double memoryPressure;
        bool isMemoryLow;
        bool isMemoryCritical;
    };
    
    // GPU信息数据结构
    struct GpuInfo {
        String name;
        String vendor;
        String driverVersion;
        uint64 dedicatedMemory;
        uint64 sharedMemory;
        double gpuUsage;
        double memoryUsage;
        double videoDecoderUsage;
        double videoEncoderUsage;
        double currentFrequency;
        double baseFrequency;
        double maxFrequency;
        double memoryFrequency;
        double temperature;
        double hotspotTemperature;
        double memoryTemperature;
        double powerUsage;
        double boardPower;
        double maxPowerLimit;
        double fanSpeed;
        double fanSpeedPercent;
        uint64 computeUnits;
        String architecture;
        double performanceRating;
    };
    
    // 网络适配器数据结构
    struct NetworkAdapter {
        String name;
        String description;
        String macAddress;
        Vector<String> ipAddresses;
        bool isConnected;
        double uploadSpeed;
        double downloadSpeed;
        uint64 totalBytesSent;
        uint64 totalBytesReceived;
        String connectionType;
        String status;
        int signalStrength;
        String dnsServer;
        String gateway;
        String subnetMask;
        uint64 packetsSent;
        uint64 packetsReceived;
        uint64 errorsSent;
        uint64 errorsReceived;
        uint64 packetsDropped;
    };
    
    // 磁盘信息数据结构
    struct DiskInfo {
        String name;
        String model;
        String serialNumber;
        uint64 totalSize;
        uint64 availableSpace;
        uint64 usedSpace;
        double usagePercentage;
        String fileSystem;
        double readSpeed;
        double writeSpeed;
        uint64 totalReadBytes;
        uint64 totalWriteBytes;
        bool isHealthy;
        int healthPercentage;
        String healthStatus;
        uint64 powerOnHours;
        Vector<std::pair<String, String>> smartAttributes;
        int reallocatedSectorCount;
        int pendingSectorCount;
        int uncorrectableSectorCount;
        bool isSSD;
        bool isHDD;
        bool isNVMe;
        String interfaceType;
    };
    
    // 温度传感器数据结构
    struct TemperatureSensor {
        String name;
        String type;
        double temperature;
        double maxTemperature;
        double criticalTemperature;
        bool isOverheating;
        bool isCritical;
    };
    
    // 温度监控数据结构
    struct TemperatureInfo {
        Vector<TemperatureSensor> sensors;
        double cpuTemperature;
        double gpuTemperature;
        double motherboardTemperature;
        double averageTemperature;
        double maxSafeTemperature;
        double criticalTemperature;
        bool isOverheating;
        bool isCriticalTemperature;
    };
    
    // TPM信息数据结构
    struct TpmInfo {
        bool isPresent;
        String version;
        String manufacturer;
        String specificationVersion;
        bool isEnabled;
        bool isActivated;
        bool isOwned;
        String status;
        bool supportsPCRs;
        bool supportsAttestation;
        bool supportsSealing;
        int algorithmCount;
        Vector<String> supportedAlgorithms;
        String selectedAlgorithm;
        int pcrCount;
    };
    
    // USB设备数据结构
    struct UsbDevice {
        String id;
        String name;
        String vendor;
        String type;
        String version;
        bool isActive;
        uint64 powerUsage;
        String status;
        TimeStamp connectTime;
    };
    
    // USB监控数据结构
    struct UsbMonitorInfo {
        Vector<UsbDevice> connectedDevices;
        int deviceCount;
        bool isMonitoring;
        TimeStamp lastUpdateTime;
    };
    
    // 操作系统信息数据结构
    struct OSInfo {
        String name;
        String version;
        String buildNumber;
        String architecture;
        String servicePack;
        uint64 uptime;
        String bootTime;
        int processCount;
        int threadCount;
        int handleCount;
        double cpuLoad;
        Vector<double> loadAverages;
        int runningProcesses;
        String computerName;
        String userName;
        String domainName;
        String timeZone;
        bool is64Bit;
        bool isServer;
        bool isVirtualMachine;
        String hypervisor;
        String lastUpdateTime;
        bool isUpToDate;
        String updateStatus;
    };
    
    // 性能计数器数据结构
    struct PerformanceCounter {
        String name;
        String category;
        String instance;
        double value;
        TimeStamp timestamp;
        String unit;
    };
    
    // 系统事件数据结构
    struct SystemEvent {
        TimeStamp timestamp;
        String type;
        String source;
        String description;
        uint32 eventId;
        uint16 severity;
        Map<String, String> properties;
    };
    
    // 错误信息数据结构
    struct ErrorInfo {
        TimeStamp timestamp;
        String component;
        String errorCode;
        String description;
        uint32 severity;
        bool isRecoverable;
        String stackTrace;
    };
    
    // 日志级别枚举
    enum class LogLevel {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4
    };
    
    // 日志条目数据结构
    struct LogEntry {
        TimeStamp timestamp;
        LogLevel level;
        String component;
        String message;
        Map<String, String> context;
    };
    
    // 配置设置数据结构
    struct Configuration {
        uint32 updateInterval;
        bool enableLogging;
        LogLevel logLevel;
        bool enableTemperatureMonitoring;
        bool enableUsbMonitoring;
        bool enableTpmMonitoring;
        bool enableGpuMonitoring;
        String logFilePath;
        uint32 maxLogFileSize;
        bool enableDataExport;
        String exportFormat;
        String exportPath;
        Map<String, String> customSettings;
    };
    
    // 数据收集状态数据结构
    struct CollectionStatus {
        bool isCollecting;
        TimeStamp startTime;
        TimeStamp lastUpdateTime;
        uint32 totalUpdates;
        uint32 failedUpdates;
        Vector<String> activeComponents;
        Vector<String> failedComponents;
        double averageUpdateTime;
        ErrorInfo lastError;
    };
    
    // 系统健康状态数据结构
    struct SystemHealth {
        double overallScore;
        Map<String, double> componentScores;
        Vector<String> warnings;
        Vector<String> errors;
        Vector<String> recommendations;
        TimeStamp lastAssessment;
    };
    
    // 数据导出格式枚举
    enum class ExportFormat {
        Json = 0,
        Xml = 1,
        Csv = 2,
        Binary = 3
    };
    
    // 导出配置数据结构
    struct ExportConfig {
        ExportFormat format;
        String filePath;
        bool includeTimestamps;
        bool includeMetadata;
        Vector<String> selectedComponents;
        Map<String, String> customOptions;
    };
}