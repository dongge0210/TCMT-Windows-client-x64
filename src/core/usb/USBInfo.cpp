#include "USBInfo.h"
#include "../Utils/Logger.h"
#include <windows.h>
#include <filesystem>
#include <set>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

// USB监控实现类
class USBInfoManager::USBMonitorImpl {
public:
    USBMonitorImpl(USBInfoManager* parent) : parentManager(parent), stopFlag(false) {}
    
    ~USBMonitorImpl() {
        StopMonitoring();
    }
    
    void StartMonitoring() {
        if (monitorThread.joinable()) return;
        
        stopFlag = false;
        monitorThread = std::thread(&USBMonitorImpl::MonitorLoop, this);
        Logger::Info("USB监控线程已启动");
    }
    
    void StopMonitoring() {
        stopFlag = true;
        if (monitorThread.joinable()) {
            monitorThread.join();
            Logger::Info("USB监控线程已停止");
        }
    }
    
    std::vector<USBDeviceInfo> GetCurrentDevices() {
        std::lock_guard<std::mutex> lock(devicesMutex);
        return currentDevices;
    }
    
private:
    void MonitorLoop() {
        while (!stopFlag) {
            try {
                DetectUSBChanges();
                std::this_thread::sleep_for(500ms); // 每500ms检查一次
            }
            catch (const std::exception& e) {
                Logger::Error("USB监控循环异常: " + std::string(e.what()));
                std::this_thread::sleep_for(1s); // 异常时等待更长时间
            }
        }
    }
    
    void DetectUSBChanges() {
        std::set<std::string> newUSBPaths;
        DWORD drives = GetLogicalDrives();
        std::bitset<26> currentDrives(drives);
        
        // 检测所有可移动驱动器
        for (int i = 0; i < 26; ++i) {
            if (currentDrives[i]) {
                char driveLetter = 'A' + i;
                if (IsRemovableDrive(driveLetter)) {
                    std::string drivePath = std::string(1, driveLetter) + ":\\";
                    newUSBPaths.insert(drivePath);
                }
            }
        }
        
        // 比较新旧设备列表
        {
            std::lock_guard<std::mutex> lock(devicesMutex);
            
            // 检查新插入的设备
            for (const auto& path : newUSBPaths) {
                if (currentUSBPaths.find(path) == currentUSBPaths.end()) {
                    // 新设备插入
                    USBDeviceInfo deviceInfo;
                    if (parentManager->GetDriveInfo(path, deviceInfo)) {
                        deviceInfo.state = USBState::Inserted;
                        if (deviceInfo.isUpdateReady) {
                            deviceInfo.state = USBState::UpdateReady;
                            Logger::Info("检测到更新U盘插入: " + path);
                        } else {
                            Logger::Info("检测到USB设备插入: " + path);
                        }
                        
                        currentDevices.push_back(deviceInfo);
                        currentUSBPaths.insert(path);
                        
                        // 通知状态变化
                        if (parentManager->stateCallback) {
                            parentManager->stateCallback(deviceInfo);
                        }
                    }
                }
            }
            
            // 检查拔出的设备
            for (auto it = currentDevices.begin(); it != currentDevices.end();) {
                if (newUSBPaths.find(it->drivePath) == newUSBPaths.end()) {
                    // 设备拔出
                    Logger::Info("USB设备已拔出: " + it->drivePath);
                    
                    USBDeviceInfo removedDevice = *it;
                    removedDevice.state = USBState::Removed;
                    
                    if (parentManager->stateCallback) {
                        parentManager->stateCallback(removedDevice);
                    }
                    
                    currentUSBPaths.erase(it->drivePath);
                    it = currentDevices.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    
    bool IsRemovableDrive(char letter) {
        char rootPath[] = "A:\\";
        rootPath[0] = letter;
        UINT driveType = GetDriveTypeA(rootPath);
        return driveType == DRIVE_REMOVABLE;
    }
    
    USBInfoManager* parentManager;
    std::thread monitorThread;
    std::atomic<bool> stopFlag;
    std::set<std::string> currentUSBPaths;
    std::vector<USBDeviceInfo> currentDevices;
    std::mutex devicesMutex;
};

// USBInfoManager 实现
USBInfoManager::USBInfoManager() : pImpl(std::make_unique<USBMonitorImpl>(this)) {}

USBInfoManager::~USBInfoManager() {
    Cleanup();
}

bool USBInfoManager::Initialize() {
    try {
        Logger::Info("初始化USB监控管理器");
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("USB监控管理器初始化失败: " + std::string(e.what()));
        return false;
    }
}

void USBInfoManager::Cleanup() {
    StopMonitoring();
    Logger::Info("USB监控管理器已清理");
}

void USBInfoManager::StartMonitoring() {
    if (pImpl) {
        pImpl->StartMonitoring();
    }
}

void USBInfoManager::StopMonitoring() {
    if (pImpl) {
        pImpl->StopMonitoring();
    }
}

std::vector<USBDeviceInfo> USBInfoManager::GetCurrentUSBDevices() {
    if (pImpl) {
        return pImpl->GetCurrentDevices();
    }
    return {};
}

void USBInfoManager::SetStateCallback(USBStateCallback callback) {
    stateCallback = callback;
}

bool USBInfoManager::IsInitialized() const {
    return pImpl != nullptr;
}

bool USBInfoManager::GetDriveInfo(const std::string& drivePath, USBDeviceInfo& info) {
    try {
        info.drivePath = drivePath;
        
        // 获取卷标
        char volumeName[MAX_PATH] = {0};
        if (GetVolumeInformationA(drivePath.c_str(), volumeName, MAX_PATH, 
                                   nullptr, nullptr, nullptr, nullptr, 0)) {
            info.volumeLabel = volumeName;
        }
        
        // 获取磁盘空间信息
        ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
        if (GetDiskFreeSpaceExA(drivePath.c_str(), &freeBytesAvailable, 
                                 &totalBytes, &totalFreeBytes)) {
            info.totalSize = totalBytes.QuadPart;
            info.freeSpace = freeBytesAvailable.QuadPart;
        }
        
        // 检查update文件夹
        info.isUpdateReady = HasUpdateFolder(drivePath);
        
        // 设置更新时间
        GetSystemTime(&info.lastUpdate);
        
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("获取驱动器信息失败: " + std::string(e.what()));
        return false;
    }
}

bool USBInfoManager::HasUpdateFolder(const std::string& drivePath) {
    try {
        fs::path updatePath = fs::path(drivePath) / "update";
        if (fs::exists(updatePath) && fs::is_directory(updatePath)) {
            // 检查目录是否为空
            auto it = fs::directory_iterator(updatePath);
            return it != fs::directory_iterator(); // 不为空返回true
        }
        return false;
    }
    catch (const std::exception& e) {
        Logger::Warn("检查update文件夹时发生异常: " + std::string(e.what()));
        return false;
    }
}