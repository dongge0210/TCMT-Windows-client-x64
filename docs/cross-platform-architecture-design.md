# 跨平台编译架构设计文档

## 1. 项目概述

### 当前状态
- **目标平台**: Windows (主要)
- **编译器**: MSVC 2022 (v143/v145)
- **标准**: C++20 / C17
- **依赖**: Windows API, WMI, COM, PDH, CUDA, Qt 6.9.0

### 跨平台目标
- **目标平台**: Windows, macOS, Linux
- **编译器**: MSVC (Windows), Clang (macOS/Linux), GCC (Linux)
- **保持功能**: 所有现有硬件监控功能

## 2. 平台抽象层架构

### 2.1 核心设计原则
```
┌─────────────────────────────────────────┐
│              应用层 (Application)        │
├─────────────────────────────────────────┤
│            业务逻辑层 (Business)          │
├─────────────────────────────────────────┤
│          平台抽象层 (Platform Abstraction)│
├─────────────────────────────────────────┤
│    Windows API   │   macOS API   │ Linux API │
└─────────────────────────────────────────┘
```

### 2.2 目录结构设计
```
src/
├── core/
│   ├── common/           # 跨平台通用代码
│   │   ├── BaseInfo.h    # 基础信息接口
│   │   ├── DataTypes.h   # 通用数据类型
│   │   └── Utils.h       # 通用工具函数
│   ├── platform/         # 平台特定实现
│   │   ├── windows/      # Windows 实现
│   │   │   ├── WinCpuInfo.cpp/h
│   │   │   ├── WinMemoryInfo.cpp/h
│   │   │   ├── WinNetworkAdapter.cpp/h
│   │   │   ├── WinDiskInfo.cpp/h
│   │   │   ├── WinGpuInfo.cpp/h
│   │   │   ├── WinTemperatureWrapper.cpp/h
│   │   │   ├── WinTpmInfo.cpp/h
│   │   │   ├── WinUsbInfo.cpp/h
│   │   │   ├── WinOSInfo.cpp/h
│   │   │   ├── WinUtils.cpp/h
│   │   │   └── WinWMIManager.cpp/h
│   │   ├── macos/        # macOS 实现
│   │   │   ├── MacCpuInfo.cpp/h
│   │   │   ├── MacMemoryInfo.cpp/h
│   │   │   ├── MacNetworkAdapter.cpp/h
│   │   │   ├── MacDiskInfo.cpp/h
│   │   │   ├── MacGpuInfo.cpp/h
│   │   │   ├── MacTemperatureWrapper.cpp/h
│   │   │   ├── MacTpmInfo.cpp/h
│   │   │   ├── MacUsbInfo.cpp/h
│   │   │   ├── MacOSInfo.cpp/h
│   │   │   ├── MacUtils.cpp/h
│   │   │   └── MacIOKitManager.cpp/h
│   │   └── linux/        # Linux 实现
│   │       ├── LinuxCpuInfo.cpp/h
│   │       ├── LinuxMemoryInfo.cpp/h
│   │       ├── LinuxNetworkAdapter.cpp/h
│   │       ├── LinuxDiskInfo.cpp/h
│   │       ├── LinuxGpuInfo.cpp/h
│   │       ├── LinuxTemperatureWrapper.cpp/h
│   │       ├── LinuxTpmInfo.cpp/h
│   │       ├── LinuxUsbInfo.cpp/h
│   │       ├── LinuxOSInfo.cpp/h
│   │       ├── LinuxUtils.cpp/h
│   │       └── LinuxSysfsManager.cpp/h
│   └── factory/           # 工厂模式实现
│       ├── InfoFactory.h
│       ├── InfoFactory.cpp
│       └── PlatformDetector.h
```

## 3. 条件编译策略

### 3.1 平台检测宏
```cpp
// core/common/PlatformDefs.h
#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #define PLATFORM_NAME "Windows"
#elif defined(__APPLE__)
    #define PLATFORM_MACOS
    #define PLATFORM_NAME "macOS"
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #define PLATFORM_NAME "Linux"
#else
    #error "Unsupported platform"
#endif

// 编译器检测
#if defined(_MSC_VER)
    #define COMPILER_MSVC
    #define COMPILER_NAME "MSVC"
#elif defined(__clang__)
    #define COMPILER_CLANG
    #define COMPILER_NAME "Clang"
#elif defined(__GNUC__)
    #define COMPILER_GCC
    #define COMPILER_NAME "GCC"
#else
    #error "Unsupported compiler"
#endif
```

### 3.2 条件包含策略
```cpp
// main.cpp 示例
#include "core/common/PlatformDefs.h"

// 平台特定头文件
#ifdef PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <shellapi.h>
    #include <sddl.h>
    #include <Aclapi.h>
    #include <conio.h>
    #include <eh.h>
    #include <pdh.h>
    #include <wbemidl.h>
    #include <comdef.h>
#elif defined(PLATFORM_MACOS)
    #include <mach/mach.h>
    #include <sys/sysctl.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/ps/IOPowerSources.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <Foundation/Foundation.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <sys/statfs.h>
    #include <unistd.h>
    #include <fstream>
    #include <dirent.h>
#endif

// 通用C++标准库
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <locale>
#include <new>
#include <stdexcept>
```

### 3.3 工厂模式实现
```cpp
// core/factory/InfoFactory.h
#pragma once
#include "core/common/BaseInfo.h"
#include "core/common/PlatformDefs.h"

class InfoFactory {
public:
    // 创建CPU信息实例
    static std::unique_ptr<ICpuInfo> CreateCpuInfo() {
#ifdef PLATFORM_WINDOWS
        return std::make_unique<WinCpuInfo>();
#elif defined(PLATFORM_MACOS)
        return std::make_unique<MacCpuInfo>();
#elif defined(PLATFORM_LINUX)
        return std::make_unique<LinuxCpuInfo>();
#endif
    }
    
    // 创建内存信息实例
    static std::unique_ptr<IMemoryInfo> CreateMemoryInfo() {
#ifdef PLATFORM_WINDOWS
        return std::make_unique<WinMemoryInfo>();
#elif defined(PLATFORM_MACOS)
        return std::make_unique<MacMemoryInfo>();
#elif defined(PLATFORM_LINUX)
        return std::make_unique<LinuxMemoryInfo>();
#endif
    }
    
    // ... 其他硬件信息工厂方法
};
```

## 4. 平台特定实现映射

### 4.1 Windows API → macOS/Linux API 映射

| Windows API | macOS API | Linux API | 功能 |
|-------------|-----------|-----------|------|
| `GetSystemInfo()` | `sysctlbyname()` | `/proc/cpuinfo` | CPU信息 |
| `GlobalMemoryStatusEx()` | `sysctlbyname()` | `/proc/meminfo` | 内存信息 |
| `GetAdaptersInfo()` | `getifaddrs()` | `/proc/net/dev` | 网络适配器 |
| `WMI` | `IOKit` | `sysfs`, `/proc` | 硬件信息 |
| `PDH` | `host_statistics()` | `/proc/stat` | 性能计数器 |
| `CreateFileMapping()` | `mmap()` | `mmap()` | 共享内存 |
| `CreatePipe()` | `pipe()` | `pipe()` | 管道通信 |
| `RegOpenKeyEx()` | `CFPreferences` | 配置文件 | 注册表/配置 |

### 4.2 关键组件映射

#### CPU监控
- **Windows**: PDH API + WMI
- **macOS**: `host_statistics()` + `sysctlbyname()`
- **Linux**: `/proc/stat` + `/proc/cpuinfo`

#### 内存监控
- **Windows**: `GlobalMemoryStatusEx()` + 性能计数器
- **macOS**: `vm_statistics64()` + `sysctlbyname()`
- **Linux**: `/proc/meminfo` + `/proc/vmstat`

#### GPU监控
- **Windows**: DirectX + NVAPI/ADL
- **macOS**: Metal API + IOKit
- **Linux**: DRM + NVML/ADL

#### 温度监控
- **Windows**: LibreHardwareMonitor + WMI
- **macOS**: IOKit + `kern.temperature`
- **Linux**: `/sys/class/hwmon/` + `lm-sensors`

## 5. 构建系统配置

### 5.1 CMake配置 (跨平台)
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(TCMT-Monitor VERSION 1.0.0 LANGUAGES CXX C)

# C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 平台检测
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
endif()

# 编译器特定设置
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Werror)
endif()

# 平台特定源文件
if(PLATFORM_WINDOWS)
    set(PLATFORM_SOURCES
        src/core/platform/windows/*.cpp
        src/core/platform/windows/*.h
    )
    set(PLATFORM_LIBS kernel32 user32 gdi32 winspool shell32 ole32 oleaut32)
elseif(PLATFORM_MACOS)
    set(PLATFORM_SOURCES
        src/core/platform/macos/*.cpp
        src/core/platform/macos/*.h
    )
    set(PLATFORM_LIBS "-framework Foundation" "-framework IOKit" "-framework CoreFoundation")
elseif(PLATFORM_LINUX)
    set(PLATFORM_SOURCES
        src/core/platform/linux/*.cpp
        src/core/platform/linux/*.h
    )
    set(PLATFORM_LIBS pthread rt)
endif()

# 共享源文件
set(COMMON_SOURCES
    src/core/common/*.cpp
    src/core/common/*.h
    src/core/factory/*.cpp
    src/core/factory/*.h
)

add_executable(TCMT-Monitor
    src/main.cpp
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
)

target_link_libraries(TCMT-Monitor ${PLATFORM_LIBS})
```

### 5.2 MSVC项目文件修改
```xml
<!-- Project1.vcxproj 关键修改 -->
<PropertyGroup Condition="'$(Platform)'=='x64'">
    <LanguageStandard>stdcpp20</LanguageStandard>
    <LanguageStandard_C>stdc17</LanguageStandard_C>
</PropertyGroup>

<!-- 条件包含 -->
<ItemDefinitionGroup Condition="'$(Platform)'=='x64'">
    <ClCompile>
        <PreprocessorDefinitions>
            PLATFORM_WINDOWS;
            _WIN32_WINNT=0x0A00;  <!-- Windows 10 -->
            WIN32_LEAN_AND_MEAN;
            NOMINMAX;
            %(PreprocessorDefinitions)
        </PreprocessorDefinitions>
    </ClCompile>
</ItemDefinitionGroup>
```

## 6. 迁移策略

### 6.1 阶段性迁移计划

**阶段1: 基础架构搭建**
1. 创建平台抽象层目录结构
2. 实现平台检测宏
3. 创建基础接口定义
4. 实现工厂模式框架

**阶段2: 核心组件迁移**
1. CPU信息监控 (优先级: 高)
2. 内存信息监控 (优先级: 高)
3. 网络适配器监控 (优先级: 中)
4. 磁盘信息监控 (优先级: 中)

**阶段3: 高级功能迁移**
1. GPU监控 (优先级: 低 - 需要平台特定API)
2. 温度监控 (优先级: 低 - 需要传感器驱动)
3. TPM信息 (优先级: 低 - 平台差异大)
4. USB监控 (优先级: 中 - 接口相对标准)

**阶段4: 集成测试**
1. 跨平台构建测试
2. 功能一致性验证
3. 性能基准测试
4. 文档完善

### 6.2 兼容性保证

**向后兼容**
- 保持现有Windows功能完整性
- 现有API接口不变
- 数据结构保持兼容

**渐进式迁移**
- 支持混合编译模式
- 逐步替换平台特定代码
- 保持测试覆盖率

## 7. 特殊注意事项

### 7.1 权限和安全
- **Windows**: 需要管理员权限获取某些硬件信息
- **macOS**: 需要适当的系统权限
- **Linux**: 需要root权限访问某些系统文件

### 7.2 第三方库依赖
- **LibreHardwareMonitor**: 仅Windows，需要平台替代方案
- **CUDA**: 仅Windows/Linux，macOS需要Metal替代
- **Qt**: 跨平台支持良好
- **TPM库**: 需要平台特定实现

### 7.3 性能考虑
- 不同平台API性能差异
- 缓存策略需要平台优化
- 内存使用模式差异

## 8. 测试策略

### 8.1 单元测试
- 每个平台实现独立测试
- 工厂模式测试
- 数据一致性测试

### 8.2 集成测试
- 跨平台数据格式验证
- 性能回归测试
- 长时间运行稳定性测试

### 8.3 持续集成
- 多平台构建矩阵
- 自动化测试流水线
- 代码覆盖率监控