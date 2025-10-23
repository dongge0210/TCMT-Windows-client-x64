// TCMT Shared Memory Structure Offset Calculator
// C++20 compatible - Generates memory layout documentation
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#else
struct SYSTEMTIME {
    std::uint16_t wYear, wMonth, wDayOfWeek, wDay;
    std::uint16_t wHour, wMinute, wSecond, wMilliseconds;
};
// Mock wchar_t for cross-platform
#ifndef wchar_t
#define wchar_t char16_t
#endif
#endif

#pragma pack(push, 1)

// 基础时间戳信息
struct TimestampInfo {
    std::uint64_t timestamp;        // Unix 时间戳（毫秒）
    std::uint32_t updateInterval;   // 更新间隔(ms)
    std::uint32_t writeSequence;    // 写入序列号（奇偶标志）
    std::uint8_t dataValid;         // 数据有效性标志
    std::uint8_t reserved[3];       // 对齐保留
};

// 主共享内存块结构（简化版用于偏移计算）
struct SharedMemoryBlock {
    // 头部信息 (16字节)
    std::uint32_t structVersion;        // 结构版本号
    std::uint32_t totalSize;            // 总大小
    std::uint32_t writeSequence;        // 写入序列号
    std::uint32_t reserved;             // 对齐保留
    
    // 数据偏移量 (44字节)
    std::uint32_t cpuDataOffset;
    std::uint32_t memoryDataOffset;
    std::uint32_t gpuDataOffset;
    std::uint32_t networkDataOffset;
    std::uint32_t logicalDiskDataOffset;
    std::uint32_t physicalDiskDataOffset;
    std::uint32_t temperatureDataOffset;
    std::uint32_t tpmDataOffset;
    std::uint32_t processDataOffset;
    std::uint32_t usbDataOffset;
    std::uint32_t mainboardDataOffset;
    
    // 数据计数 (32字节)
    std::uint32_t gpuCount;
    std::uint32_t networkAdapterCount;
    std::uint32_t logicalDiskCount;
    std::uint32_t physicalDiskCount;
    std::uint32_t temperatureCount;
    std::uint32_t processCount;
    std::uint32_t usbDeviceCount;
    std::uint32_t reserved2;
    
    // 有效性标志 (16字节)
    bool cpuDataValid;
    bool memoryDataValid;
    bool gpuDataValid;
    bool networkDataValid;
    bool logicalDiskDataValid;
    bool physicalDiskDataValid;
    bool temperatureDataValid;
    bool tpmDataValid;
    bool processDataValid;
    bool usbDataValid;
    bool mainboardDataValid;
    std::uint8_t reserved3[5];
    
    // 全局时间戳 (16字节)
    TimestampInfo globalTimestamp;
    
    // 哈希校验 (32字节)
    std::uint8_t structureHash[32];
};

#pragma pack(pop)

void generateMarkdownTable() {
    std::ofstream mdFile("shared_memory_offsets.md");
    if (!mdFile.is_open()) {
        std::cerr << "无法创建 Markdown 文件\n";
        return;
    }
    
    mdFile << "# TCMT 共享内存结构偏移量表\n\n";
    mdFile << "**生成时间**: " << __DATE__ << " " << __TIME__ << "\n";
    mdFile << "**C++标准**: C++20\n";
    mdFile << "**内存对齐**: #pragma pack(1)\n\n";
    
    mdFile << "## 结构体大小\n\n";
    mdFile << "- SharedMemoryBlock: " << sizeof(SharedMemoryBlock) << " 字节\n";
    mdFile << "- TimestampInfo: " << sizeof(TimestampInfo) << " 字节\n\n";
    
    mdFile << "## 字段偏移量表\n\n";
    mdFile << "| 字段名 | 偏移量 | 大小(字节) | 描述 |\n";
    mdFile << "|--------|--------|-----------|------|\n";

#define MD_OFFSET(field, desc) \
    mdFile << "| " << #field \
           << " | " << offsetof(SharedMemoryBlock, field) \
           << " | " << sizeof(((SharedMemoryBlock*)0)->field) \
           << " | " << desc << " |\n"

    MD_OFFSET(structVersion, "结构版本号");
    MD_OFFSET(totalSize, "总大小");
    MD_OFFSET(writeSequence, "写入序列号");
    MD_OFFSET(reserved, "对齐保留");
    MD_OFFSET(cpuDataOffset, "CPU数据偏移");
    MD_OFFSET(memoryDataOffset, "内存数据偏移");
    MD_OFFSET(gpuDataOffset, "GPU数据偏移");
    MD_OFFSET(networkDataOffset, "网络数据偏移");
    MD_OFFSET(logicalDiskDataOffset, "逻辑磁盘数据偏移");
    MD_OFFSET(physicalDiskDataOffset, "物理磁盘数据偏移");
    MD_OFFSET(temperatureDataOffset, "温度数据偏移");
    MD_OFFSET(tpmDataOffset, "TPM数据偏移");
    MD_OFFSET(processDataOffset, "进程数据偏移");
    MD_OFFSET(usbDataOffset, "USB数据偏移");
    MD_OFFSET(mainboardDataOffset, "主板数据偏移");
    MD_OFFSET(gpuCount, "GPU数量");
    MD_OFFSET(networkAdapterCount, "网络适配器数量");
    MD_OFFSET(logicalDiskCount, "逻辑磁盘数量");
    MD_OFFSET(physicalDiskCount, "物理磁盘数量");
    MD_OFFSET(temperatureCount, "温度传感器数量");
    MD_OFFSET(processCount, "进程数量");
    MD_OFFSET(usbDeviceCount, "USB设备数量");
    MD_OFFSET(reserved2, "对齐保留2");
    MD_OFFSET(cpuDataValid, "CPU数据有效");
    MD_OFFSET(memoryDataValid, "内存数据有效");
    MD_OFFSET(gpuDataValid, "GPU数据有效");
    MD_OFFSET(networkDataValid, "网络数据有效");
    MD_OFFSET(logicalDiskDataValid, "逻辑磁盘数据有效");
    MD_OFFSET(physicalDiskDataValid, "物理磁盘数据有效");
    MD_OFFSET(temperatureDataValid, "温度数据有效");
    MD_OFFSET(tpmDataValid, "TPM数据有效");
    MD_OFFSET(processDataValid, "进程数据有效");
    MD_OFFSET(usbDataValid, "USB数据有效");
    MD_OFFSET(mainboardDataValid, "主板数据有效");
    MD_OFFSET(reserved3, "对齐保留3");
    MD_OFFSET(globalTimestamp, "全局时间戳");
    MD_OFFSET(structureHash, "结构哈希");

    mdFile << "\n## 内存布局\n\n";
    mdFile << "```\n";
    mdFile << "SharedMemoryBlock (" << sizeof(SharedMemoryBlock) << " 字节固定头部):\n";
    mdFile << "┌─────────────────────────────────────────────────┐\n";
    mdFile << "│ 头部信息 (16字节)                                │\n";
    mdFile << "├─────────────────────────────────────────────────┤\n";
    mdFile << "│ 数据偏移量 (44字节)                              │\n";
    mdFile << "├─────────────────────────────────────────────────┤\n";
    mdFile << "│ 数据计数 (32字节)                                │\n";
    mdFile << "├─────────────────────────────────────────────────┤\n";
    mdFile << "│ 有效性标志 (16字节)                              │\n";
    mdFile << "├─────────────────────────────────────────────────┤\n";
    mdFile << "│ 全局时间戳 (16字节)                              │\n";
    mdFile << "├─────────────────────────────────────────────────┤\n";
    mdFile << "│ 结构哈希 (32字节)                                │\n";
    mdFile << "├─────────────────────────────────────────────────┤\n";
    mdFile << "│ 动态数据区域 (通过偏移量访问)                    │\n";
    mdFile << "└─────────────────────────────────────────────────┘\n";
    mdFile << "```\n";
    
    mdFile.close();
}

void generateJsonTable() {
    std::ofstream jsonFile("shared_memory_offsets.json");
    if (!jsonFile.is_open()) {
        std::cerr << "无法创建 JSON 文件\n";
        return;
    }
    
    jsonFile << "{\n";
    jsonFile << "  \"schemaVersion\": \"1.0\",\n";
    jsonFile << "  \"generatedAt\": \"" << __DATE__ << " " << __TIME__ << "\",\n";
    jsonFile << "  \"architectureVersion\": \"v0.14\",\n";
    jsonFile << "  \"cppStandard\": \"C++20\",\n";
    jsonFile << "  \"packAlignment\": 1,\n";
    jsonFile << "  \"totalSize\": " << sizeof(SharedMemoryBlock) << ",\n";
    jsonFile << "  \"fieldOffsets\": {\n";
    
#define JSON_OFFSET(field) \
    jsonFile << "    \"" << #field << "\": " << offsetof(SharedMemoryBlock, field)

    JSON_OFFSET(structVersion); jsonFile << ",\n";
    JSON_OFFSET(totalSize); jsonFile << ",\n";
    JSON_OFFSET(writeSequence); jsonFile << ",\n";
    JSON_OFFSET(reserved); jsonFile << ",\n";
    JSON_OFFSET(cpuDataOffset); jsonFile << ",\n";
    JSON_OFFSET(memoryDataOffset); jsonFile << ",\n";
    JSON_OFFSET(gpuDataOffset); jsonFile << ",\n";
    JSON_OFFSET(networkDataOffset); jsonFile << ",\n";
    JSON_OFFSET(logicalDiskDataOffset); jsonFile << ",\n";
    JSON_OFFSET(physicalDiskDataOffset); jsonFile << ",\n";
    JSON_OFFSET(temperatureDataOffset); jsonFile << ",\n";
    JSON_OFFSET(tpmDataOffset); jsonFile << ",\n";
    JSON_OFFSET(processDataOffset); jsonFile << ",\n";
    JSON_OFFSET(usbDataOffset); jsonFile << ",\n";
    JSON_OFFSET(mainboardDataOffset); jsonFile << ",\n";
    JSON_OFFSET(globalTimestamp); jsonFile << ",\n";
    JSON_OFFSET(structureHash); jsonFile << "\n";
    
    jsonFile << "  }\n}\n";
    jsonFile.close();
}

int main() {
    std::cout << "=== TCMT 共享内存偏移量计算工具 ===\n";
    std::cout << "架构版本: v0.14 | C++20 标准\n\n";
    
    std::cout << "结构体大小:\n";
    std::cout << "===========\n";
    std::cout << std::left << std::setw(25) << "SharedMemoryBlock:" << sizeof(SharedMemoryBlock) << " 字节\n";
    std::cout << std::left << std::setw(25) << "TimestampInfo:" << sizeof(TimestampInfo) << " 字节\n\n";
    
    std::cout << "主要字段偏移量:\n";
    std::cout << "===============\n";
    std::cout << std::left << std::setw(25) << "字段名" 
              << std::setw(10) << "偏移量" 
              << std::setw(8) << "大小" 
              << "描述\n";
    std::cout << std::string(60, '-') << '\n';

#define PRINT_OFFSET(field, desc) \
    std::cout << std::left << std::setw(25) << #field \
              << std::setw(10) << offsetof(SharedMemoryBlock, field) \
              << std::setw(8) << sizeof(((SharedMemoryBlock*)0)->field) \
              << desc << '\n'

    PRINT_OFFSET(structVersion, "结构版本号");
    PRINT_OFFSET(totalSize, "总大小");
    PRINT_OFFSET(writeSequence, "写入序列号");
    PRINT_OFFSET(cpuDataOffset, "CPU数据偏移");
    PRINT_OFFSET(memoryDataOffset, "内存数据偏移");
    PRINT_OFFSET(gpuDataOffset, "GPU数据偏移");
    PRINT_OFFSET(globalTimestamp, "全局时间戳");
    PRINT_OFFSET(structureHash, "结构哈希");
    
    std::cout << "\n生成文档:\n";
    std::cout << "=========\n";
    
    generateMarkdownTable();
    std::cout << "✓ Markdown文档: shared_memory_offsets.md\n";
    
    generateJsonTable();
    std::cout << "✓ JSON文件: shared_memory_offsets.json\n";
    
    std::cout << "\n偏移量计算完成!\n";
    return 0;
}