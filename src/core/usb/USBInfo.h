#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>

// USB设备状态枚举
enum class USBState {
    Removed,     // USB设备拔出
    Inserted,    // USB设备插入  
    UpdateReady  // 更新U盘（包含update文件夹）
};

// USB设备信息结构
struct USBDeviceInfo {
    std::string drivePath;     // 驱动器路径 (如 "E:\\")
    std::string volumeLabel;   // 卷标名称
    uint64_t totalSize;        // 总容量（字节）
    uint64_t freeSpace;        // 可用空间（字节）
    bool isUpdateReady;        // 是否包含update文件夹
    USBState state;            // 当前状态
    SYSTEMTIME lastUpdate;     // 最后更新时间
    
    USBDeviceInfo() : totalSize(0), freeSpace(0), isUpdateReady(false), 
                     state(USBState::Removed) {}
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
    
    // 获取驱动器详细信息
    bool GetDriveInfo(const std::string& drivePath, USBDeviceInfo& info);
    
    // 检查update文件夹
    bool HasUpdateFolder(const std::string& drivePath);
};