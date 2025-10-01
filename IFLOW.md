# 项目概述 (Project Overview)

## 项目名称
project-monitor-TC

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

## 许可证
本项目遵循 GNU General Public License v3.0 (GPL-3.0) 开源协议。

## 核心技术栈
- **后端 (数据收集)**: 
  - C++ (用于高性能数据收集和处理)
  - Windows API (用于底层系统交互)
  - PDH (Performance Data Helper) (用于收集系统性能计数器数据)
  - WMI (Windows Management Instrumentation) (用于获取硬件和操作系统信息)
- **前端 (用户界面)**: 
  - C# (用于构建 Windows 应用程序)
  - WPF (Windows Presentation Foundation) (用于创建丰富的用户界面)
  - MVVM 模式 (Model-View-ViewModel，用于分离 UI 逻辑和业务逻辑)
- **通信机制**: 
  - 共享内存 (Shared Memory) (用于 C++ 后端和 C# 前端之间的高效数据交换)
- **第三方库/子模块**:
  - [LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor) (用于温度等传感器数据，MPL-2.0)
  - [curl](https://curl.se/) (用于网络传输)
  - [nlohmann/json](https://github.com/nlohmann/json) (用于 JSON 处理, MIT)
  - [openssl](https://www.openssl.org/) (用于 TLS/SSL 加密, Apache 2.0)
  - [PDCurses](https://github.com/wmcbrine/PDCurses) (用于终端界面处理)
  - [zlib](https://zlib.net/) (用于数据压缩)
  - [FFmpeg](https://ffmpeg.org/) (可能用于视频/音频相关功能)
  - [websocketpp](https://github.com/zaphoyd/websocketpp) (用于 WebSocket 通信)
  - [tpm2-tss](https://github.com/tpm2-software/tpm2-tss) (用于 TPM 2.0 相关功能)
  - TC (自定义库，具体用途需进一步分析)

## 项目架构
```
project-monitor-TC/
├── src/                     # C++ 后端源代码
│   ├── core/                # 核心硬件信息收集模块
│   │   ├── cpu/             # CPU 信息
│   │   ├── memory/          # 内存信息
│   │   ├── gpu/             # GPU 信息
│   │   ├── network/         # 网络适配器信息
│   │   ├── disk/            # 磁盘信息
│   │   ├── temperature/     # 温度传感器信息
│   │   ├── tpm/             # TPM 信息
│   │   ├── DataStruct/      # 共享数据结构和内存管理
│   │   └── Utils/           # 工具类 (日志、时间、WMI 管理等)
│   ├── third_party/         # 第三方库 (作为 Git 子模块)
│   └── CPP-parsers/         # 配置文件解析器 (作为 Git 子模块)
├── WPF-UI1/                 # C# WPF 前端
│   ├── ViewModels/          # MVVM 视图模型
│   ├── Models/              # 数据模型
│   ├── Services/            # 服务层 (如共享内存服务)
│   ├── Converters/          # 数据转换器
│   └── ...                  # UI 文件 (XAML) 和应用入口
└── ...                      # 构建配置、文档等
```

# 构建和运行 (Building and Running)

## 环境要求
- **Windows 操作系统** (项目为 Windows 平台设计)
  - 推荐 Windows 10 或更高版本。
- **Visual Studio** (推荐用于 C++ 和 C# 项目开发与构建)
  - 建议使用 Visual Studio 2019 或更高版本。
  - 需要安装 "使用 C++ 的桌面开发" 和 ".NET 桌面开发" 工作负载。
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
    - **重要**: 它需要管理员权限以访问某些硬件信息（如温度、SMART）。右键点击可执行文件并选择“以管理员身份运行”。
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

# 开发约定 (Development Conventions)

## C++ 后端
- **模块化**: 不同的硬件信息由 `src/core/` 下的独立模块负责收集。
- **数据结构**: 使用 `src/core/DataStruct/` 中定义的结构体来组织和传递数据。
- **共享内存**: 通过 `SharedMemoryManager` 类管理共享内存的创建、写入和清理。
- **日志**: 使用 `Logger` 类进行日志记录。
- **命名规范**: 文件名和类名通常使用大驼峰命名法 (PascalCase)。

## C# WPF 前端
- **MVVM 模式**: 严格遵循 MVVM (Model-View-ViewModel) 模式进行开发。
  - **Models**: `WPF-UI1/Models/` 定义了与共享内存数据结构对应的 C# 类。
  - **ViewModels**: `WPF-UI1/ViewModels/` 包含视图逻辑，通过 `SharedMemoryService` 读取数据并绑定到 UI。
  - **Views**: `WPF-UI1/MainWindow.xaml` 及其代码隐藏文件定义了用户界面。
- **依赖注入**: 使用 `Microsoft.Extensions.DependencyInjection` 进行服务注册和解析。
- **图表库**: 使用 `LiveChartsCore` 进行数据可视化（如温度图表）。
- **日志**: 使用 `Serilog` 进行日志记录。
- **命名规范**: 与 C++ 后端类似，采用大驼峰命名法 (PascalCase)。
- **数据绑定**: 大量使用数据绑定将 ViewModel 的属性与 UI 控件连接。

## 通信约定
- **共享内存名称**: `Global\SystemMonitorSharedMemory` 或 `Local\SystemMonitorSharedMemory` 或 `SystemMonitorSharedMemory`。
- **数据结构同步**: C# 端的 `SharedMemoryService.cs` 中定义了与 C++ 端 `DataStruct.h` 和 `SharedMemoryBlock` 结构严格对应的结构体，并使用 `StructLayout` 和 `MarshalAs` 确保内存布局一致。
- **字符编码**: 使用 `wchar_t` (C++) 和 `ushort[]` (C#) 来处理宽字符字符串，确保跨语言字符串传递的正确性。

# 贡献指南 (Contribution Guidelines)

我们欢迎任何形式的贡献！在提交贡献之前，请确保您已阅读并理解以下指南。

## 如何贡献
1.  **Fork 本仓库**: 在 GitHub 上 Fork 本项目到您的个人账户。
2.  **创建分支**: 为您的功能或修复创建一个新的分支。
3.  **进行修改**: 在您的分支上进行代码修改。
4.  **提交更改**: 提交您的更改，并确保提交信息清晰、简洁。
5.  **发起 Pull Request**: 向本项目的 `dev` 分支发起 Pull Request。

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