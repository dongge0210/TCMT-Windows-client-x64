#include "InfoFactory.h"
#include "core/common/PlatformDefs.h"

// 平台特定包含
#ifdef PLATFORM_WINDOWS
    #include "../platform/windows/WinCpuInfo.h"
    #include "../platform/windows/WinMemoryInfo.h"
    #include "../platform/windows/WinGpuInfo.h"
    #include "../platform/windows/WinNetworkAdapter.h"
    #include "../platform/windows/WinDiskInfo.h"
    #include "../platform/windows/WinTemperatureWrapper.h"
    #include "../platform/windows/WinTpmInfo.h"
    #include "../platform/windows/WinUsbInfo.h"
    #include "../platform/windows/WinOSInfo.h"
    #include "../platform/windows/WinDataCollector.h"
#elif defined(PLATFORM_MACOS)
    #include "../platform/macos/MacCpuInfo.h"
    #include "../platform/macos/MacMemoryInfo.h"
    #include "../platform/macos/MacGpuInfo.h"
    #include "../platform/macos/MacNetworkAdapter.h"
    #include "../platform/macos/MacDiskInfo.h"
    #include "../platform/macos/MacTemperatureWrapper.h"
    #include "../platform/macos/MacTpmInfo.h"
    #include "../platform/macos/MacUsbInfo.h"
    #include "../platform/macos/MacOSInfo.h"
    #include "../platform/macos/MacDataCollector.h"
#elif defined(PLATFORM_LINUX)
    #include "../platform/linux/LinuxCpuInfo.h"
    #include "../platform/linux/LinuxMemoryInfo.h"
    #include "../platform/linux/LinuxGpuInfo.h"
    #include "../platform/linux/LinuxNetworkAdapter.h"
    #include "../platform/linux/LinuxDiskInfo.h"
    #include "../platform/linux/LinuxTemperatureWrapper.h"
    #include "../platform/linux/LinuxTpmInfo.h"
    #include "../platform/linux/LinuxUsbInfo.h"
    #include "../platform/linux/LinuxOSInfo.h"
    #include "../platform/linux/LinuxDataCollector.h"
#endif

#include <sstream>

// 静态成员初始化
std::string InfoFactory::lastError;
uint32_t InfoFactory::defaultUpdateInterval = 1000; // 默认1秒更新间隔

// 公共接口实现
std::unique_ptr<ICpuInfo> InfoFactory::CreateCpuInfo() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinCpuInfo();
        #elif defined(PLATFORM_MACOS)
            return CreateMacCpuInfo();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxCpuInfo();
        #else
            SetError("Unsupported platform for CPU info");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create CPU info: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<IMemoryInfo> InfoFactory::CreateMemoryInfo() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinMemoryInfo();
        #elif defined(PLATFORM_MACOS)
            return CreateMacMemoryInfo();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxMemoryInfo();
        #else
            SetError("Unsupported platform for memory info");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create memory info: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<IGpuInfo> InfoFactory::CreateGpuInfo() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinGpuInfo();
        #elif defined(PLATFORM_MACOS)
            return CreateMacGpuInfo();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxGpuInfo();
        #else
            SetError("Unsupported platform for GPU info");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create GPU info: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<INetworkAdapter> InfoFactory::CreateNetworkAdapter(const std::string& adapterName) {
    try {
        std::string sanitizedName = SanitizeComponentName(adapterName);
        
        #ifdef PLATFORM_WINDOWS
            return CreateWinNetworkAdapter(sanitizedName);
        #elif defined(PLATFORM_MACOS)
            return CreateMacNetworkAdapter(sanitizedName);
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxNetworkAdapter(sanitizedName);
        #else
            SetError("Unsupported platform for network adapter info");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create network adapter info: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<IDiskInfo> InfoFactory::CreateDiskInfo(const std::string& diskName) {
    try {
        std::string sanitizedName = SanitizeComponentName(diskName);
        
        #ifdef PLATFORM_WINDOWS
            return CreateWinDiskInfo(sanitizedName);
        #elif defined(PLATFORM_MACOS)
            return CreateMacDiskInfo(sanitizedName);
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxDiskInfo(sanitizedName);
        #else
            SetError("Unsupported platform for disk info");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create disk info: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<ITemperatureMonitor> InfoFactory::CreateTemperatureMonitor() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinTemperatureMonitor();
        #elif defined(PLATFORM_MACOS)
            return CreateMacTemperatureMonitor();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxTemperatureMonitor();
        #else
            SetError("Unsupported platform for temperature monitor");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create temperature monitor: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<ITpmInfo> InfoFactory::CreateTpmInfo() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinTpmInfo();
        #elif defined(PLATFORM_MACOS)
            return CreateMacTpmInfo();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxTpmInfo();
        #else
            SetError("Unsupported platform for TPM info");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create TPM info: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<IUsbMonitor> InfoFactory::CreateUsbMonitor() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinUsbMonitor();
        #elif defined(PLATFORM_MACOS)
            return CreateMacUsbMonitor();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxUsbMonitor();
        #else
            SetError("Unsupported platform for USB monitor");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create USB monitor: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<IOSInfo> InfoFactory::CreateOSInfo() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinOSInfo();
        #elif defined(PLATFORM_MACOS)
            return CreateMacOSInfo();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxOSInfo();
        #else
            SetError("Unsupported platform for OS info");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create OS info: ") + e.what());
        return nullptr;
    }
}

std::unique_ptr<IDataCollector> InfoFactory::CreateDataCollector() {
    try {
        #ifdef PLATFORM_WINDOWS
            return CreateWinDataCollector();
        #elif defined(PLATFORM_MACOS)
            return CreateMacDataCollector();
        #elif defined(PLATFORM_LINUX)
            return CreateLinuxDataCollector();
        #else
            SetError("Unsupported platform for data collector");
            return nullptr;
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create data collector: ") + e.what());
        return nullptr;
    }
}

std::vector<std::string> InfoFactory::GetAvailableNetworkAdapters() {
    std::vector<std::string> adapters;
    
    try {
        #ifdef PLATFORM_WINDOWS
            // Windows特定实现
            // 这里需要调用Windows API获取网络适配器列表
            // 暂时返回空列表，具体实现需要移到WinNetworkAdapter中
        #elif defined(PLATFORM_MACOS)
            // macOS特定实现
            // 使用getifaddrs()等API获取网络接口列表
        #elif defined(PLATFORM_LINUX)
            // Linux特定实现
            // 读取/proc/net/dev或使用netlink获取网络接口列表
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get network adapters: ") + e.what());
    }
    
    return adapters;
}

std::vector<std::string> InfoFactory::GetAvailableDisks() {
    std::vector<std::string> disks;
    
    try {
        #ifdef PLATFORM_WINDOWS
            // Windows特定实现
            // 使用GetLogicalDriveStrings()等API获取磁盘列表
        #elif defined(PLATFORM_MACOS)
            // macOS特定实现
            // 使用IOKit获取磁盘设备列表
        #elif defined(PLATFORM_LINUX)
            // Linux特定实现
            // 读取/proc/mounts或使用udev获取磁盘列表
        #endif
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get disks: ") + e.what());
    }
    
    return disks;
}

bool InfoFactory::IsGpuMonitoringAvailable() {
    #ifdef PLATFORM_WINDOWS
        // Windows通常有GPU监控支持
        return true;
    #elif defined(PLATFORM_MACOS)
        // macOS有Metal API支持
        return true;
    #elif defined(PLATFORM_LINUX)
        // Linux需要检查DRM/NVML/ADL支持
        // 这里可以检查相关文件是否存在
        return true; // 暂时返回true，具体实现需要检查
    #else
        return false;
    #endif
}

bool InfoFactory::IsTpmAvailable() {
    #ifdef PLATFORM_WINDOWS
        // Windows有TPM支持
        return true;
    #elif defined(PLATFORM_MACOS)
        // macOS通常不使用TPM
        return false;
    #elif defined(PLATFORM_LINUX)
        // Linux需要检查TPM设备
        return true; // 暂时返回true，需要检查/dev/tpm*等设备
    #else
        return false;
    #endif
}

bool InfoFactory::IsUsbMonitoringAvailable() {
    #ifdef PLATFORM_WINDOWS
        // Windows有USB监控支持
        return true;
    #elif defined(PLATFORM_MACOS)
        // macOS有IOKit USB监控支持
        return true;
    #elif defined(PLATFORM_LINUX)
        // Linux有udev USB监控支持
        return true;
    #else
        return false;
    #endif
}

bool InfoFactory::IsTemperatureMonitoringAvailable() {
    #ifdef PLATFORM_WINDOWS
        // Windows有温度监控支持
        return true;
    #elif defined(PLATFORM_MACOS)
        // macOS有IOKit温度传感器支持
        return true;
    #elif defined(PLATFORM_LINUX)
        // Linux有hwmon温度监控支持
        return true;
    #else
        return false;
    #endif
}

std::string InfoFactory::GetPlatformName() {
    return PLATFORM_NAME;
}

std::string InfoFactory::GetCompilerName() {
    return COMPILER_NAME;
}

std::string InfoFactory::GetArchitectureName() {
    return ARCH_NAME;
}

std::string InfoFactory::GetBuildInfo() {
    std::ostringstream oss;
    oss << "Platform: " << GetPlatformName() 
        << ", Compiler: " << GetCompilerName()
        << ", Architecture: " << GetArchitectureName()
        << ", Build: " << BUILD_TIMESTAMP;
    return oss.str();
}

void InfoFactory::SetDefaultUpdateInterval(uint32_t intervalMs) {
    defaultUpdateInterval = intervalMs;
}

uint32_t InfoFactory::GetDefaultUpdateInterval() {
    return defaultUpdateInterval;
}

std::string InfoFactory::GetLastError() {
    return lastError;
}

void InfoFactory::ClearError() {
    lastError.clear();
}

// 私有辅助函数实现
void InfoFactory::SetError(const std::string& error) {
    lastError = error;
}

bool InfoFactory::ValidateComponentName(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    
    // 检查名称是否包含非法字符
    const std::string invalidChars = "<>:\"|?*";
    return name.find_first_of(invalidChars) == std::string::npos;
}

std::string InfoFactory::SanitizeComponentName(const std::string& name) {
    if (name.empty()) {
        return name;
    }
    
    std::string sanitized = name;
    
    #ifdef PLATFORM_WINDOWS
        // Windows特定清理
        // 移除驱动器前缀（如果存在）
        if (sanitized.length() >= 2 && sanitized[1] == ':') {
            sanitized = sanitized.substr(2);
        }
        
        // 移除开头的反斜杠
        while (!sanitized.empty() && (sanitized[0] == '\\' || sanitized[0] == '/')) {
            sanitized = sanitized.substr(1);
        }
    #elif defined(PLATFORM_MACOS) || defined(PLATFORM_LINUX)
        // Unix-like系统特定清理
        // 移除开头的斜杠
        while (!sanitized.empty() && sanitized[0] == '/') {
            sanitized = sanitized.substr(1);
        }
    #endif
    
    return sanitized;
}