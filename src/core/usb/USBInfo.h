#pragma once
#include "../common/PlatformDefs.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
#elif defined(PLATFORM_MACOS)
    #include <IOKit/IOKitLib.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <diskarbitration/diskarbitration.h>
    #include <sys/mount.h>
    #include <unistd.h>
    #include <thread>
    #include <atomic>
    #include <mutex>
    #include <chrono>
#elif defined(PLATFORM_LINUX)
    #include <libudev.h>
    #include <sys/mount.h>
    #include <unistd.h>
    #include <thread>
    #include <atomic>
    #include <mutex>
    #include <chrono>
#endif

// USB设备状态枚举
enum class USBState {
    Removed,     // USB设备拔出
    Inserted,    // USB设备插入  
    UpdateReady  // 更新U盘（包含update文件夹）
};

// USB设备信息结构
struct USBDeviceInfo {
    std::string drivePath;     // 驱动器路径 (Windows: "E:\\", macOS/Linux: "/Volumes/USB")
    std::string volumeLabel;   // 卷标名称
    std::string devicePath;     // 设备路径 (macOS: "/dev/disk2s1", Linux: "/dev/sdb1")
    std::string mountPoint;     // 挂载点 (macOS/Linux特有)
    uint64_t totalSize;        // 总容量（字节）
    uint64_t freeSpace;        // 可用空间（字节）
    bool isUpdateReady;        // 是否包含update文件夹
    USBState state;            // 当前状态
    
    // 跨平台时间字段
#ifdef PLATFORM_WINDOWS
    SYSTEMTIME lastUpdate;     // 最后更新时间 (Windows特有)
#else
    time_t lastUpdate;         // 最后更新时间 (Unix时间戳)
#endif
    
    // 跨平台特有字段
    std::string vendorId;       // 供应商ID
    std::string productId;      // 产品ID
    std::string serialNumber;   // 序列号
    std::string deviceClass;    // 设备类别
    bool isRemovable;           // 是否为可移动设备
    
    USBDeviceInfo() : totalSize(0), freeSpace(0), isUpdateReady(false), 
                     state(USBState::Removed), isRemovable(true) {
#ifdef PLATFORM_WINDOWS
        memset(&lastUpdate, 0, sizeof(lastUpdate));
#else
        lastUpdate = 0;
#endif
    }
};

// USB监控管理器
class USBInfoManager {
public:
    using USBStateCallback = std::function<void(const USBDeviceInfo&)>;
    
    USBInfoManager();
    ~USBInfoManager();
    
    // 初始化USB监控
    bool Initialize();
    
    // 清理资源
    void Cleanup();
    
    // 开始监控
    void StartMonitoring();
    
    // 停止监控
    void StopMonitoring();
    
    // 获取当前所有USB设备信息
    std::vector<USBDeviceInfo> GetCurrentUSBDevices();
    
    // 设置状态变化回调
    void SetStateCallback(USBStateCallback callback);
    
    // 检查是否已初始化
    bool IsInitialized() const;
    
private:
    class USBMonitorImpl;
    std::unique_ptr<USBMonitorImpl> pImpl;
    
    // 状态变化回调
    USBStateCallback stateCallback;
    
    // 内部状态变化处理
    void OnUSBStateChanged(USBState state, const std::string& drivePath);
    
    // 获取设备详细信息
    bool GetDeviceInfo(const std::string& identifier, USBDeviceInfo& info);
    
    // 检查update文件夹
    bool HasUpdateFolder(const std::string& path);
    
#ifdef PLATFORM_WINDOWS
    bool GetDriveInfo(const std::string& drivePath, USBDeviceInfo& info);
#elif defined(PLATFORM_MACOS)
    bool GetMacDeviceInfo(const std::string& mountPoint, USBDeviceInfo& info);
    std::string GetMacDevicePath(const std::string& mountPoint);
    void GetMacDeviceProperties(const std::string& devicePath, USBDeviceInfo& info);
    bool IsMacUSBDevice(const std::string& devicePath);
    void MonitorMacUSBDevices();
    DADissenterRef CreateDiskArbitrationSession();
    void ReleaseDiskArbitrationSession(DADissenterRef session);
#elif defined(PLATFORM_LINUX)
    bool GetLinuxDeviceInfo(const std::string& mountPoint, USBDeviceInfo& info);
    std::string GetLinuxDevicePath(const std::string& mountPoint);
    void GetLinuxDeviceProperties(const std::string& devicePath, USBDeviceInfo& info);
    bool IsLinuxUSBDevice(const std::string& devicePath);
    void MonitorLinuxUSBDevices();
    struct udev* udev;
    struct udev_monitor* udevMonitor;
    int monitorFd;
#endif
};