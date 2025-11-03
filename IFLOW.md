# 项目概述 (Project Overview)

## 项目名称
TCMT-Windows-client-x64

## 项目描述
这是一个系统硬件监控工具，用于实时监测和展示 Windows 系统的硬件状态。它旨在为用户提供全面、准确且易于理解的系统性能视图。

项目采用 C++ 后端高效地收集数据，并通过共享内存与 C# WPF 前端进行通信，以提供图形化用户界面。

**核心监控功能包括：**
- **CPU**: 实时利用率、核心频率、线程负载、温度等。
- **内存**: 物理内存和虚拟内存使用情况、可用内存、页面文件使用情况等。
- **GPU**: 利用率、显存使用情况、温度、风扇转速等（支持 NVIDIA 和 AMD 显卡）。
- **网络**: 各网络适配器的上传/下载速度、数据总量、连接状态等。
- **磁盘**: 读写速度、数据总量、SMART 健康状态等。
- **温度**: 系统各关键部件的实时温度监控（通过 LibreHardwareMonitor 库实现）。
- **TPM**: 可信平台模块的状态和信息（如果系统支持）。
- **操作系统**: 操作系统版本、启动时间等基本信息。
- **USB**: USB 设备监控，包括插入/拔出检测、更新U盘识别等。
- **诊断管道**: 实时诊断数据管道，用于调试和监控系统状态。

## 许可证
本项目遵循 GNU General Public License v3.0 (GPL-3.0) 开源协议。

## 核心技术栈
- **后端 (数据收集)**: 
  - C++ (用于高性能数据收集和处理，遵循 C++20 标准)
  - C (用于底层系统交互，遵循 C17 标准)
  - Windows API (用于底层系统交互)
  - PDH (Performance Data Helper) (用于收集系统性能计数器数据)
  - WMI (Windows Management Instrumentation) (用于获取硬件和操作系统信息)
  - CUDA (用于 NVIDIA GPU 监控，支持 CUDA 12.6+)
- **前端 (用户界面)**: 
  - C# (.NET 8.0) (用于构建 Windows 应用程序)
  - WPF (Windows Presentation Foundation) (用于创建丰富的用户界面)
  - MVVM 模式 (Model-View-ViewModel，用于分离 UI 逻辑和业务逻辑)
  - Material Design (用于现代化 UI 设计)
- **通信机制**: 
  - 共享内存 (Shared Memory) (用于 C++ 后端和 C# 前端之间的高效数据交换)
  - 命名管道 (Named Pipe) (用于诊断数据传输)
- **第三方库/子模块**:
  - [LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor) (用于温度等传感器数据，MPL-2.0)
  - [curl](https://curl.se/) (用于网络传输)
  - [nlohmann/json](https://github.com/nlohmann/json) (用于 JSON 处理, MIT)
  - [openssl](https://www.openssl.org/) (用于 TLS/SSL 加密, Apache 2.0)
  - [PDCurses](https://github.com/wmcbrine/PDCurses) (用于终端界面处理)
  - [zlib](https://zlib.net/) (用于数据压缩)
  - [FFmpeg](https://ffmpeg.org/) (用于视频/音频相关功能)
  - [websocketpp](https://github.com/zaphoyd/websocketpp) (用于 WebSocket 通信)
  - [tpm2-tss](https://github.com/tpm2-software/tpm2-tss) (用于 TPM 2.0 相关功能)
  - [USBMonitor-cpp](https://github.com/dongge0210/USBMonitor-cpp) (用于 USB 设备监控)
  - [TC](https://github.com/dongge0210/TC) (自定义库)
  - [CPP-parsers](https://github.com/dongge0210/CPP-parsers) (配置文件解析器)

## 项目架构
```
TCMT-Windows-client-x64/
├── src/                     # C++ 后端源代码 (遵循 C++20 和 C17 标准)
│   ├── main.cpp             # 程序入口点
│   ├── core/                # 核心硬件信息收集模块
│   │   ├── cpu/             # CPU 信息
│   │   │   ├── CpuInfo.cpp
│   │   │   └── CpuInfo.h
│   │   ├── memory/          # 内存信息
│   │   │   ├── MemoryInfo.cpp
│   │   │   └── MemoryInfo.h
│   │   │   └── gpu/         # GPU 相关内存信息
│   │   ├── gpu/             # GPU 信息
│   │   │   ├── GpuInfo.cpp
│   │   │   └── GpuInfo.h
│   │   ├── network/         # 网络适配器信息
│   │   │   ├── NetworkAdapter.cpp
│   │   │   └── NetworkAdapter.h
│   │   ├── disk/            # 磁盘信息
│   │   │   ├── DiskInfo.cpp
│   │   │   └── DiskInfo.h
│   │   ├── temperature/     # 温度传感器信息
│   │   │   ├── TemperatureWrapper.cpp
│   │   │   ├── TemperatureWrapper.h
│   │   │   ├── LibreHardwareMonitorBridge.cpp
│   │   │   └── LibreHardwareMonitorBridge.h
│   │   ├── tpm/             # TPM 信息
│   │   │   ├── TpmInfo.cpp
│   │   │   ├── TpmInfo.h
│   │   │   ├── TpmInfoEnhanced.cpp
│   │   │   └── TpmInfoEnhanced.h
│   │   ├── usb/             # USB 设备信息 (新增)
│   │   │   ├── USBInfo.cpp
│   │   │   └── USBInfo.h
│   │   ├── os/              # 操作系统信息
│   │   │   ├── OSInfo.cpp
│   │   │   └── OSInfo.h
│   │   ├── DataStruct/      # 共享数据结构和内存管理
│   │   │   ├── DataStruct.h
│   │   │   ├── DiagnosticsPipe.h  # 诊断管道 (新增)
│   │   │   ├── SharedMemoryManager.cpp
│   │   │   ├── SharedMemoryManager.h
│   │   │   └── Producer.cpp
│   │   └── Utils/           # 工具类 (日志、时间、WMI 管理等)
│   │       ├── Logger.cpp
│   │       ├── Logger.h
│   │       ├── TimeUtils.cpp
│   │       ├── TimeUtils.h
│   │       ├── WinUtils.cpp
│   │       ├── WinUtils.h
│   │       ├── WMIManager.cpp
│   │       ├── WMIManager.h
│   │       ├── ComInitializationHelper.cpp
│   │       ├── MotherboardInfo.cpp
│   │       ├── MotherboardInfo.h
│   │       ├── LibreHardwareMonitorBridge.cpp
│   │       └── LibreHardwareMonitorBridge.h
│   ├── third_party/         # 第三方库 (作为 Git 子模块)
│   │   ├── LibreHardwareMonitor
│   │   ├── curl
│   │   ├── openssl
│   │   ├── PDCurses
│   │   ├── TC
│   │   ├── tpm2-tss
│   │   ├── USBMonitor-cpp
│   │   ├── websocketpp
│   │   └── FFmpeg-7.1
│   └── CPP-parsers/         # 配置文件解析器 (作为 Git 子模块)
├── WPF-UI1/                 # C# WPF 前端
│   ├── App.xaml             # 应用程序入口点
│   ├── App.xaml.cs
│   ├── MainWindow.xaml      # 主窗口
│   ├── MainWindow.xaml.cs
│   ├── ViewModels/          # MVVM 视图模型
│   │   └── MainWindowViewModel.cs
│   ├── Models/              # 数据模型
│   │   └── SystemInfo.cs
│   ├── Services/            # 服务层 (如共享内存服务)
│   │   ├── SharedMemoryService.cs
│   │   └── DiagnosticsPipeClient.cs  # 诊断管道客户端 (新增)
│   ├── Converters/          # 数据转换器
│   │   └── ValueConverters.cs
│   └── ...                  # 其他 UI 文件和项目配置
├── Project1/                # Visual Studio C++ 项目文件
│   ├── Project1.sln
│   ├── Project1.vcxproj
│   └── ...
└── ...                      # 构建配置、文档等
```

# 构建和运行 (Building and Running)

## 环境要求
- **Windows 操作系统** (项目为 Windows 平台设计)
  - 推荐 Windows 10 (版本 10.0.26100.0) 或更高版本。
- **Visual Studio** (推荐用于 C++ 和 C# 项目开发与构建)
  - 建议使用 Visual Studio 2022 (17.0) 或更高版本。
  - 需要安装 "使用 C++ 的桌面开发" 和 ".NET 桌面开发" 工作负载。
  - 需要支持 C++20 和 C# .NET 8.0
- **CUDA 工具包** (用于 NVIDIA GPU 监控)
  - 推荐 CUDA 12.6 或更高版本
- **CMake** (可能用于部分子模块或构建流程)
  - 建议使用 CMake 3.10 或更高版本。

## 构建步骤
1.  **初始化子模块**:
    ```bash
    git submodule update --init --recursive
    ```
    这将拉取所有必要的第三方库。
2.  **构建 C++ 后端**:
    - 打开 `Project1/Project1.sln` 解决方案文件。
    - 在 Visual Studio 中选择合适的平台 (x64) 和配置 (Debug 或 Release)。
    - 构建解决方案。这将生成 `Project1.exe`，负责收集硬件信息并写入共享内存。
3.  **构建 C# WPF 前端**:
    - 打开 `WPF-UI1/WPF-UI1.csproj` 项目文件。
    - 在 Visual Studio 中选择合适的平台 (x64) 和配置 (Debug 或 Release)。
    - 构建项目。这将生成 `WPF-UI1.exe`，负责读取共享内存并显示图形界面。

## 运行步骤
1.  **运行 C++ 后端**:
    - 启动 `Project1.exe` (或等效的可执行文件)。
    - **重要**: 它需要管理员权限以访问某些硬件信息（如温度、SMART）。右键点击可执行文件并选择"以管理员身份运行"。
2.  **运行 C# WPF 前端**:
    - 启动 `WPF-UI1.exe`。
    - 前端将自动尝试连接到由 C++ 后端创建的共享内存，并开始显示实时数据。

## 常见问题与解决方案
- **无法获取温度或 SMART 信息**:
  - 确保 C++ 后端以管理员权限运行。
  - 检查 `LibreHardwareMonitor` 库是否正确初始化。
- **前端无法连接到共享内存**:
  - 确保 C++ 后端在前端启动之前已经成功启动并创建了共享内存。
  - 检查共享内存名称是否在 C++ 和 C# 端保持一致。
- **构建失败**:
  - 确保所有子模块都已正确初始化。
  - 检查 Visual Studio 的工作负载是否安装完整。
  - 查看构建输出日志以获取具体错误信息。
- **CUDA 相关错误**:
  - 确保 CUDA 工具包正确安装。
  - 检查 CUDA_PATH 环境变量是否设置正确。

# 开发约定 (Development Conventions)

## C++ 后端
- **模块化**: 不同的硬件信息由 `src/core/` 下的独立模块负责收集。
- **数据结构**: 使用 `src/core/DataStruct/` 中定义的结构体来组织和传递数据。
- **共享内存**: 通过 `SharedMemoryManager` 类管理共享内存的创建、写入和清理。
- **诊断管道**: 通过 `DiagnosticsPipe` 提供实时诊断数据输出。
- **USB 监控**: 通过 `USBInfoManager` 类管理 USB 设备的插入/拔出检测。
- **日志**: 使用 `Logger` 类进行日志记录。
- **内存对齐**: 使用 `#pragma pack(push, 1)` 确保跨语言数据结构对齐。
- **命名规范**: 文件名和类名通常使用大驼峰命名法 (PascalCase)。

## C# WPF 前端
- **MVVM 模式**: 严格遵循 MVVM (Model-View-ViewModel) 模式进行开发。
  - **Models**: `WPF-UI1/Models/` 定义了与共享内存数据结构对应的 C# 类。
  - **ViewModels**: `WPF-UI1/ViewModels/` 包含视图逻辑，通过 `SharedMemoryService` 读取数据并绑定到 UI。
  - **Views**: `WPF-UI1/MainWindow.xaml` 及其代码隐藏文件定义了用户界面。
- **依赖注入**: 使用 `Microsoft.Extensions.DependencyInjection` 进行服务注册和解析。
- **UI 框架**: 使用 `MaterialDesignThemes` 提供现代化的 UI 设计。
- **图表库**: 使用 `LiveChartsCore` 进行数据可视化（如温度图表）。
- **日志**: 使用 `Serilog` 进行日志记录。
- **诊断客户端**: 通过 `DiagnosticsPipeClient` 连接诊断管道获取实时调试信息。
- **命名规范**: 与 C++ 后端类似，采用大驼峰命名法 (PascalCase)。
- **数据绑定**: 大量使用数据绑定将 ViewModel 的属性与 UI 控件连接。

## 通信约定
- **共享内存名称**: `Global\SystemMonitorSharedMemory` 或 `Local\SystemMonitorSharedMemory` 或 `SystemMonitorSharedMemory`。
- **共享内存大小**: 3212 字节 (包含 USB 设备信息)。
- **诊断管道**: `\\.\pipe\SysMonDiag` 用于传输 JSON 格式的诊断数据。
- **数据结构同步**: C# 端的 `SharedMemoryService.cs` 中定义了与 C++ 端 `DataStruct.h` 和 `SharedMemoryBlock` 结构严格对应的结构体，并使用 `StructLayout` 和 `MarshalAs` 确保内存布局一致。
- **字符编码**: 使用 `wchar_t` (C++) 和 `ushort[]` (C#) 来处理宽字符字符串，确保跨语言字符串传递的正确性。
- **内存对齐**: 必须保证 C++ 和 C# 端的共享内存数据结构完全对齐，以确保跨语言数据传递的正确性。任何不匹配都可能导致 WPF-UI 显示内容缺失或有问题，需要自动检查所有步骤并修复。

## 新增功能特性
- **USB 设备监控**: 实时监控 USB 设备的插入/拔出状态，支持更新U盘自动识别。
- **诊断管道**: 提供实时诊断数据输出，便于调试和监控系统状态。
- **增强的 TPM 支持**: 提供更详细的 TPM 信息和多种检测方法。
- **SMART 数据扩展**: 提供更详细的磁盘 SMART 属性和健康指标。
- **温度监控优化**: 改进温度传感器数据收集和处理逻辑。

# 贡献指南 (Contribution Guidelines)

我们欢迎任何形式的贡献！在提交贡献之前，请确保您已阅读并理解以下指南。

## 如何贡献
1.  **Fork 本仓库**: 在 GitHub 上 Fork 本项目到您的个人账户。
2.  **创建分支**: 为您的功能或修复创建一个新的分支。
3.  **进行修改**: 在您的分支上进行代码修改。
4.  **提交更改**: 提交您的更改，并确保提交信息清晰、简洁。
5.  **发起 Pull Request**: 向本项目的 `feature/iflow-cli-integration` 分支发起 Pull Request。

## 代码规范
- 请遵循项目现有的代码风格和约定。
- C++ 代码请遵循 Google C++ Style Guide 或项目中已有的风格。
- C# 代码请遵循 Microsoft C# Coding Conventions 或项目中已有的风格。
- 为您的代码添加适当的注释，特别是复杂的逻辑部分。

## 提交信息规范
- 使用清晰、简洁的语言描述您的更改。
- 遵循 conventional commits 规范 (例如: `feat: 添加新功能`, `fix: 修复某个 bug`, `docs: 更新文档`)。

## 测试
- 在提交 Pull Request 之前，请确保您的更改通过了所有现有测试。
- 如果您添加了新功能，请添加相应的单元测试或集成测试。

## 报告问题
如果您发现任何问题或有改进建议，请通过 GitHub Issues 进行报告。

# Litho 文档引用

## 项目分析文档

本项目包含详细的 Litho 分析文档，位于 `litho.docs/` 目录下。这些文档提供了系统的全面分析报告，包括项目概述、架构设计、工作流程和各领域的深入分析。

### 文档结构

```
litho.docs/
├── __Litho_Summary_Brief__.md    # 分析摘要报告
├── __Litho_Summary_Detail__.md   # 详细分析报告 (注意：文件较大)
├── 1、项目概述.md                 # 项目详细介绍
├── 2、架构概览.md                 # 系统架构说明
├── 3、工作流程.md                 # 核心工作流程
├── 4、深入探索/                   # 各领域深度分析
│   ├── 安全监控域.md
│   ├── 第三方集成域.md
│   ├── 平台适配域.md
│   ├── 数据管理域.md
│   ├── 系统工具域.md
│   └── 硬件监控域.md
└── 5、边界调用.md                 # 系统边界与接口说明
```

### 使用说明

- 如需了解项目的整体分析结果，请参考 `__Litho_Summary_Brief__.md`
- 详细的系统分析报告请查看 `__Litho_Summary_Detail__.md`（注意：该文件较大）
- 项目概述、架构设计等具体内容请参考对应的编号文档
- 各专业领域的深入分析请查看 `4、深入探索/` 目录下的相关文档

**注意**: `__Litho_Summary_Detail__.md` 文件较大（约 13.7MB），建议根据需要选择性查看相关章节。

## 编码规范
- **字符编码**: 使用 `wchar_t` (C++) 和 `ushort[]` (C#) 来处理宽字符字符串，确保跨语言字符串传递的正确性。所有源代码文件必须使用 UTF-8 (with BOM) 编码和 CRLF 换行符。
- **内存对齐**: 必须保证 C++ 和 C# 端的共享内存数据结构完全对齐，以确保跨语言数据传递的正确性。任何不匹配都可能导致 WPF-UI 显示内容缺失或有问题，需要自动检查所有步骤并修复。