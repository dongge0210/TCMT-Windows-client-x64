// SharedMemoryBlock.h - TCMT Shared Memory Structure Definitions
// Version: v0.14 (Based on architecture document)
// C++20 Compatible - Pack(1) alignment for precise memory layout
#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

#pragma pack(push, 1) // 确保内存对齐为1字节

// 数据有效性位标定义
#define DATA_VALID_CPU          0x01
#define DATA_VALID_MEMORY       0x02
#define DATA_VALID_GPU          0x04
#define DATA_VALID_NETWORK      0x08
#define DATA_VALID_DISK         0x10
#define DATA_VALID_TEMP         0x20
#define DATA_VALID_TPM          0x40
#define DATA_VALID_PROCESS      0x80

// 写入序列状态定义
#define WRITE_SEQ_EVEN          0x00000000
#define WRITE_SEQ_ODD           0x00000001

// 错误状态码定义
#define ERROR_NONE              0x00
#define ERROR_INIT_FAILED       0x01
#define ERROR_READ_TIMEOUT      0x02
#define ERROR_WRITE_FAILED      0x03
#define ERROR_INVALID_DATA      0x04
#define ERROR_HASH_MISMATCH     0x05

namespace TCMT {

// 基础时间戳和状态信息
struct TimestampInfo {
    uint64_t timestamp;        // Unix 时间戳（毫秒）
    uint32_t updateInterval;   // 更新间隔(ms)
    uint32_t writeSequence;    // 写入序列号（奇偶标志）
    uint8_t dataValid;         // 数据有效性标志
    uint8_t reserved[3];       // 对齐保留
};

// SMART 属性数据
struct SmartAttributeData {
    uint8_t id;                    // 属性ID
    uint8_t flags;                 // 状态标志
    uint8_t current;               // 当前值
    uint8_t worst;                 // 最坏值
    uint8_t threshold;             // 阈值
    uint8_t reserved[3];           // 对齐保留
    uint64_t rawValue;             // 原始值
    wchar_t name[64];              // 属性名称
    wchar_t description[128];      // 属性描述
    bool isCritical;               // 是否关键属性
    double physicalValue;          // 物理值（经过转换）
    wchar_t units[16];             // 单位
    uint8_t padding[7];            // 对齐保留
};

// 物理磁盘SMART信息
struct PhysicalDiskSmartData {
    wchar_t model[128];            // 磁盘型号
    wchar_t serialNumber[64];      // 序列号
    wchar_t firmwareVersion[32];   // 固件版本
    wchar_t interfaceType[32];     // 接口类型 (SATA/NVMe/etc)
    wchar_t diskType[16];          // 磁盘类型 (SSD/HDD)
    uint64_t capacity;             // 总容量（字节）
    double temperature;            // 温度
    uint8_t healthPercentage;      // 健康百分比
    bool isSystemDisk;             // 是否系统盘
    bool smartEnabled;             // SMART是否启用
    bool smartSupported;           // 是否支持SMART
    uint8_t reserved[5];           // 对齐保留
    
    // SMART属性数组（最多32个常用属性）
    SmartAttributeData attributes[32];
    int32_t attributeCount;        // 实际属性数量
    
    // 关键健康指标
    uint64_t powerOnHours;         // 通电时间（小时）
    uint64_t powerCycleCount;      // 开机次数
    uint64_t reallocatedSectorCount; // 重新分配扇区数
    uint64_t currentPendingSector; // 当前待处理扇区
    uint64_t uncorrectableErrors;  // 不可纠正错误
    double wearLeveling;           // 磨损均衡（SSD）
    uint64_t totalBytesWritten;    // 总写入字节数
    uint64_t totalBytesRead;       // 总读取字节数
    
    // 关联的逻辑驱动器
    char logicalDriveLetters[8];   // 关联的驱动器盘符
    int32_t logicalDriveCount;     // 关联驱动器数量
    
    SYSTEMTIME lastScanTime;       // 最后扫描时间
    TimestampInfo timestamp;       // 时间戳信息
};

// CPU 核心数据
struct CpuCoreData {
    uint32_t coreId;               // 核心ID
    double utilizationPercent;     // 使用率百分比
    uint32_t frequencyMHz;         // 频率（MHz）
    double temperatureCelsius;     // 温度（摄氏度）
    uint64_t cycleCount;           // 时钟周期计数
    bool isPerformanceCore;        // 是否为性能核心
    bool isEfficiencyCore;         // 是否为效率核心
    uint8_t reserved[6];           // 对齐保留
};

// CPU 数据
struct CpuData {
    wchar_t cpuName[128];          // CPU名称
    uint32_t coreCount;            // 核心数
    uint32_t threadCount;          // 线程数
    double overallUtilization;     // 整体使用率
    double averageTemperature;     // 平均温度
    uint32_t baseFrequency;        // 基础频率（MHz）
    uint32_t maxFrequency;         // 最大频率（MHz）
    uint32_t currentFrequency;     // 当前频率（MHz）
    uint32_t performanceCores;     // 性能核心数
    uint32_t efficiencyCores;      // 效率核心数
    bool hyperThreading;           // 超线程启用
    bool virtualization;           // 虚拟化启用
    uint8_t reserved[6];           // 对齐保留
    CpuCoreData cores[64];         // 核心数据（最多64核心）
    TimestampInfo timestamp;       // 时间戳信息
};

// 内存数据
struct MemoryData {
    uint64_t totalPhysical;        // 总物理内存（字节）
    uint64_t availablePhysical;    // 可用物理内存（字节）
    uint64_t usedPhysical;         // 已用物理内存（字节）
    uint64_t totalVirtual;         // 总虚拟内存（字节）
    uint64_t availableVirtual;     // 可用虚拟内存（字节）
    uint64_t usedVirtual;          // 已用虚拟内存（字节）
    uint64_t totalPageFile;        // 页面文件总大小（字节）
    uint64_t availablePageFile;    // 可用页面文件（字节）
    double memoryLoad;             // 内存负载百分比
    uint32_t pageSize;             // 页面大小（字节）
    uint32_t reserved;             // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// GPU数据
struct GpuData {
    wchar_t name[128];             // GPU名称
    wchar_t brand[64];             // 品牌
    wchar_t driverVersion[32];     // 驱动版本
    uint64_t memoryTotal;          // 总显存（字节）
    uint64_t memoryUsed;           // 已用显存（字节）
    uint64_t memoryFree;           // 可用显存（字节）
    double coreClock;              // 核心频率（MHz）
    double memoryClock;            // 显存频率（MHz）
    double temperature;            // 温度（摄氏度）
    double utilization;            // GPU使用率（百分比）
    uint32_t fanSpeed;             // 风扇转速（RPM）
    bool isVirtual;                // 是否为虚拟显卡
    uint8_t reserved[7];           // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// 网络适配器数据
struct NetworkAdapterData {
    wchar_t name[128];             // 适配器名称
    wchar_t description[256];      // 适配器描述
    wchar_t mac[32];               // MAC地址
    wchar_t ipv4Address[64];       // IPv4地址
    wchar_t ipv6Address[128];      // IPv6地址
    wchar_t adapterType[32];       // 网卡类型（无线/有线/蓝牙等）
    uint64_t speed;                // 链路速度（bps）
    uint64_t bytesReceived;        // 接收字节数
    uint64_t bytesSent;            // 发送字节数
    uint64_t packetsReceived;      // 接收包数
    uint64_t packetsSent;          // 发送包数
    bool isConnected;              // 是否连接
    bool isEnabled;                // 是否启用
    uint8_t reserved[6];           // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// 磁盘逻辑卷数据
struct LogicalDiskData {
    char letter;                   // 盘符（如'C'）
    wchar_t label[128];            // 卷标
    wchar_t fileSystem[32];        // 文件系统
    uint64_t totalSize;            // 总容量（字节）
    uint64_t usedSpace;            // 已用空间（字节）
    uint64_t freeSpace;            // 可用空间（字节）
    double usagePercent;           // 使用率百分比
    bool isSystemDrive;            // 是否系统盘
    bool isReady;                  // 是否就绪
    uint8_t reserved[6];           // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// 温度传感器数据
struct TemperatureData {
    wchar_t sensorName[64];        // 传感器名称
    wchar_t sensorType[32];        // 传感器类型
    double temperature;            // 温度（摄氏度）
    double minTemperature;         // 最低温度
    double maxTemperature;         // 最高温度
    bool isValid;                  // 数据是否有效
    uint8_t reserved[7];           // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// TPM数据
struct TpmData {
    wchar_t manufacturerName[128]; // TPM制造商名称
    wchar_t manufacturerId[32];    // 制造商ID
    wchar_t version[32];           // TPM版本
    wchar_t firmwareVersion[32];   // 固件版本
    wchar_t status[64];            // TPM状态
    bool isEnabled;                // TPM是否启用
    bool isActivated;              // TPM是否激活
    bool isOwned;                  // TPM是否已拥有
    bool isReady;                  // TPM是否就绪
    bool tbsAvailable;             // TBS是否可用
    bool physicalPresenceRequired; // 是否需要物理存在
    uint8_t reserved[2];           // 对齐保留
    uint32_t specVersion;          // TPM规范版本
    uint32_t tbsVersion;           // TBS版本
    wchar_t errorMessage[256];     // 错误信息
    wchar_t detectionMethod[64];   // 检测方法
    bool wmiDetectionWorked;       // WMI检测是否成功
    bool tbsDetectionWorked;       // TBS检测是否成功
    uint8_t reserved2[6];          // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// 进程信息（简化版，不含CPU使用率）
struct ProcessData {
    uint32_t processId;            // 进程ID
    wchar_t processName[64];       // 进程名称
    wchar_t executablePath[256];   // 可执行文件路径
    uint64_t memoryUsage;          // 内存使用量（字节）
    uint32_t threadCount;          // 线程数
    bool is64Bit;                  // 是否64位进程
    uint8_t reserved[7];           // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// USB设备数据
struct UsbDeviceData {
    wchar_t deviceName[128];       // 设备名称
    wchar_t vendorId[16];          // 厂商ID
    wchar_t productId[16];         // 产品ID
    wchar_t serialNumber[64];      // 序列号
    wchar_t driverName[64];        // 驱动名称
    bool isConnected;              // 是否连接
    uint8_t usbVersion;            // USB版本
    uint8_t reserved[6];           // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// 主板/BIOS信息数据
struct MainboardData {
    wchar_t manufacturer[128];     // 主板制造商
    wchar_t product[128];          // 主板型号
    wchar_t version[32];           // 主板版本
    wchar_t serialNumber[64];      // 主板序列号
    wchar_t biosVendor[128];       // BIOS厂商
    wchar_t biosVersion[64];       // BIOS版本
    wchar_t biosDate[32];          // BIOS日期
    wchar_t chipset[64];           // 芯片组
    uint8_t reserved[8];           // 对齐保留
    TimestampInfo timestamp;       // 时间戳信息
};

// 主共享内存块结构 - 根据架构文档v0.14设计
struct SharedMemoryBlock {
    // === 头部信息 (16字节) ===
    uint32_t structVersion;        // 结构版本号 (当前为1)
    uint32_t totalSize;            // 总大小 (字节)
    uint32_t writeSequence;        // 写入序列号（奇偶标志）
    uint32_t reserved;             // 对齐保留
    
    // === 各模块数据偏移量 (44字节) ===
    uint32_t cpuDataOffset;        // CPU数据偏移
    uint32_t memoryDataOffset;     // 内存数据偏移
    uint32_t gpuDataOffset;        // GPU数据偏移 (数组)
    uint32_t networkDataOffset;    // 网络数据偏移 (数组)
    uint32_t logicalDiskDataOffset;    // 逻辑磁盘数据偏移 (数组)
    uint32_t physicalDiskDataOffset;   // 物理磁盘数据偏移 (数组)
    uint32_t temperatureDataOffset;    // 温度数据偏移 (数组)
    uint32_t tpmDataOffset;        // TPM数据偏移
    uint32_t processDataOffset;    // 进程数据偏移 (数组)
    uint32_t usbDataOffset;        // USB数据偏移 (数组)
    uint32_t mainboardDataOffset;  // 主板数据偏移
    
    // === 数据计数 (32字节) ===
    uint32_t gpuCount;             // GPU数量
    uint32_t networkAdapterCount;  // 网络适配器数量
    uint32_t logicalDiskCount;     // 逻辑磁盘数量
    uint32_t physicalDiskCount;    // 物理磁盘数量
    uint32_t temperatureCount;     // 温度传感器数量
    uint32_t processCount;         // 进程数量
    uint32_t usbDeviceCount;       // USB设备数量
    uint32_t reserved2;            // 对齐保留
    
    // === 数据有效性标志 (16字节) ===
    bool cpuDataValid;             // CPU数据有效
    bool memoryDataValid;          // 内存数据有效
    bool gpuDataValid;             // GPU数据有效
    bool networkDataValid;         // 网络数据有效
    bool logicalDiskDataValid;     // 逻辑磁盘数据有效
    bool physicalDiskDataValid;    // 物理磁盘数据有效
    bool temperatureDataValid;     // 温度数据有效
    bool tpmDataValid;             // TPM数据有效
    bool processDataValid;         // 进程数据有效
    bool usbDataValid;             // USB数据有效
    bool mainboardDataValid;       // 主板数据有效
    uint8_t reserved3[5];          // 对齐保留
    
    // === 全局时间戳 (16字节) ===
    TimestampInfo globalTimestamp;
    
    // === 哈希校验 (32字节) ===
    uint8_t structureHash[32];     // SHA256哈希
    
    // === 实际数据区域 ===
    // 注意：dataBuffer是变长的，通过偏移量访问各种数据
    // 不在结构体中定义以便计算固定头部大小
    // uint8_t dataBuffer[];
};

// 数据访问辅助模板函数
template<typename T>
inline T* GetDataPtr(SharedMemoryBlock* block, uint32_t offset) {
    if (block == nullptr || offset == 0 || offset >= block->totalSize) {
        return nullptr;
    }
    // dataBuffer在结构体末尾，计算其起始位置
    uint8_t* dataBufferStart = reinterpret_cast<uint8_t*>(block) + sizeof(SharedMemoryBlock);
    return reinterpret_cast<T*>(dataBufferStart + offset);
}

template<typename T>
inline const T* GetDataPtr(const SharedMemoryBlock* block, uint32_t offset) {
    if (block == nullptr || offset == 0 || offset >= block->totalSize) {
        return nullptr;
    }
    const uint8_t* dataBufferStart = reinterpret_cast<const uint8_t*>(block) + sizeof(SharedMemoryBlock);
    return reinterpret_cast<const T*>(dataBufferStart + offset);
}

} // namespace TCMT

#pragma pack(pop)

// 编译时大小检查 (C++20 概念)
static_assert(sizeof(TCMT::SharedMemoryBlock) % 8 == 0, "SharedMemoryBlock must be 8-byte aligned");
static_assert(sizeof(TCMT::TimestampInfo) == 16, "TimestampInfo must be exactly 16 bytes");

// 版本信息常量
namespace TCMT {
    constexpr uint32_t SHARED_MEMORY_VERSION = 1;
    constexpr const char* ARCHITECTURE_VERSION = "v0.14";
    constexpr size_t MIN_SHARED_MEMORY_SIZE = 2 * 1024 * 1024;  // 2MB 最小值
    constexpr size_t MAX_SHARED_MEMORY_SIZE = 10 * 1024 * 1024; // 10MB 最大值
}