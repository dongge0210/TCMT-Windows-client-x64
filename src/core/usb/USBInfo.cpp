#include "USBInfo.h"
#include "../Utils/Logger.h"
#include <windows.h>
#include <filesystem>
#include <set>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <bitset>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

// USB monitoring implementation class
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
        Logger::Info("USB monitoring thread started");
    }
    
    void StopMonitoring() {
        stopFlag = true;
        if (monitorThread.joinable()) {
            monitorThread.join();
            Logger::Info("USB monitoring thread stopped");
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
                std::this_thread::sleep_for(500ms); // Check every 500ms
            }
            catch (const std::exception& e) {
                Logger::Error("USB monitoring loop exception: " + std::string(e.what()));
                std::this_thread::sleep_for(1s); // Wait longer on exception
            }
        }
    }
    
    void DetectUSBChanges() {
        std::set<std::string> newUSBPaths;
        DWORD drives = GetLogicalDrives();
        std::bitset<26> currentDrives(drives);
        
    // Detect all removable drives
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
                    // New device inserted
                    USBDeviceInfo deviceInfo;
                    if (parentManager->GetDriveInfo(path, deviceInfo)) {
                        deviceInfo.state = USBState::Inserted;
                        if (deviceInfo.isUpdateReady) {
                            deviceInfo.state = USBState::UpdateReady;
                            Logger::Info("Update USB drive detected: " + path);
                        } else {
                            Logger::Info("USB device inserted: " + path);
                        }
                        
                        currentDevices.push_back(deviceInfo);
                        currentUSBPaths.insert(path);
                        
                        // Notify state change
                        if (parentManager->stateCallback) {
                            parentManager->stateCallback(deviceInfo);
                        }
                    }
                }
            }
            
            // Check for removed devices
            for (auto it = currentDevices.begin(); it != currentDevices.end();) {
                if (newUSBPaths.find(it->drivePath) == newUSBPaths.end()) {
                    // Device removed
                    Logger::Info("USB device removed: " + it->drivePath);
                    
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
        Logger::Info("Initializing USB monitoring manager");
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("USB monitoring manager initialization failed: " + std::string(e.what()));
        return false;
    }
}

void USBInfoManager::Cleanup() {
    StopMonitoring();
    Logger::Info("USB monitoring manager cleaned up");
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
        Logger::Error("Failed to get drive information: " + std::string(e.what()));
        return false;
    }
}

bool USBInfoManager::HasUpdateFolder(const std::string& drivePath) {
    try {
        fs::path updatePath = fs::path(drivePath) / "update";
        if (fs::exists(updatePath) && fs::is_directory(updatePath)) {
            // Check if directory is empty
            auto it = fs::directory_iterator(updatePath);
            return it != fs::directory_iterator(); // Return true if not empty
        }
        return false;
    }
    catch (const std::exception& e) {
        Logger::Warn("Exception when checking update folder: " + std::string(e.what()));
        return false;
    }
}