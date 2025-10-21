# TCMT 共享内存结构偏移量表

**生成时间**: Oct 21 2025 15:48:01
**C++标准**: C++20
**内存对齐**: #pragma pack(1)

## 结构体大小

- SharedMemoryBlock: 160 字节
- TimestampInfo: 20 字节

## 字段偏移量表

| 字段名 | 偏移量 | 大小(字节) | 描述 |
|--------|--------|-----------|------|
| structVersion | 0 | 4 | 结构版本号 |
| totalSize | 4 | 4 | 总大小 |
| writeSequence | 8 | 4 | 写入序列号 |
| reserved | 12 | 4 | 对齐保留 |
| cpuDataOffset | 16 | 4 | CPU数据偏移 |
| memoryDataOffset | 20 | 4 | 内存数据偏移 |
| gpuDataOffset | 24 | 4 | GPU数据偏移 |
| networkDataOffset | 28 | 4 | 网络数据偏移 |
| logicalDiskDataOffset | 32 | 4 | 逻辑磁盘数据偏移 |
| physicalDiskDataOffset | 36 | 4 | 物理磁盘数据偏移 |
| temperatureDataOffset | 40 | 4 | 温度数据偏移 |
| tpmDataOffset | 44 | 4 | TPM数据偏移 |
| processDataOffset | 48 | 4 | 进程数据偏移 |
| usbDataOffset | 52 | 4 | USB数据偏移 |
| mainboardDataOffset | 56 | 4 | 主板数据偏移 |
| gpuCount | 60 | 4 | GPU数量 |
| networkAdapterCount | 64 | 4 | 网络适配器数量 |
| logicalDiskCount | 68 | 4 | 逻辑磁盘数量 |
| physicalDiskCount | 72 | 4 | 物理磁盘数量 |
| temperatureCount | 76 | 4 | 温度传感器数量 |
| processCount | 80 | 4 | 进程数量 |
| usbDeviceCount | 84 | 4 | USB设备数量 |
| reserved2 | 88 | 4 | 对齐保留2 |
| cpuDataValid | 92 | 1 | CPU数据有效 |
| memoryDataValid | 93 | 1 | 内存数据有效 |
| gpuDataValid | 94 | 1 | GPU数据有效 |
| networkDataValid | 95 | 1 | 网络数据有效 |
| logicalDiskDataValid | 96 | 1 | 逻辑磁盘数据有效 |
| physicalDiskDataValid | 97 | 1 | 物理磁盘数据有效 |
| temperatureDataValid | 98 | 1 | 温度数据有效 |
| tpmDataValid | 99 | 1 | TPM数据有效 |
| processDataValid | 100 | 1 | 进程数据有效 |
| usbDataValid | 101 | 1 | USB数据有效 |
| mainboardDataValid | 102 | 1 | 主板数据有效 |
| reserved3 | 103 | 5 | 对齐保留3 |
| globalTimestamp | 108 | 20 | 全局时间戳 |
| structureHash | 128 | 32 | 结构哈希 |

## 内存布局

```
SharedMemoryBlock (160 字节固定头部):
┌─────────────────────────────────────────────────┐
│ 头部信息 (16字节)                                │
├─────────────────────────────────────────────────┤
│ 数据偏移量 (44字节)                              │
├─────────────────────────────────────────────────┤
│ 数据计数 (32字节)                                │
├─────────────────────────────────────────────────┤
│ 有效性标志 (16字节)                              │
├─────────────────────────────────────────────────┤
│ 全局时间戳 (16字节)                              │
├─────────────────────────────────────────────────┤
│ 结构哈希 (32字节)                                │
├─────────────────────────────────────────────────┤
│ 动态数据区域 (通过偏移量访问)                    │
└─────────────────────────────────────────────────┘
```
