#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>

// USB device state enumeration
enum class USBState {
    Removed,     // USB device removed
    Inserted,    // USB device inserted
    UpdateReady  // Update USB (contains update folder)
};

// USB device information structure
struct USBDeviceInfo {
    std::string drivePath;     // Drive path (e.g. "E:\\")
    std::string volumeLabel;   // Volume label
    uint64_t totalSize;        // Total size (bytes)
    uint64_t freeSpace;        // Free space (bytes)
    bool isUpdateReady;        // Whether contains update folder
    USBState state;            // Current state
    SYSTEMTIME lastUpdate;     // Last update time
    
    USBDeviceInfo() : totalSize(0), freeSpace(0), isUpdateReady(false), 
                     state(USBState::Removed) {}
};

// USB monitoring manager
class USBInfoManager {
public:
    using USBStateCallback = std::function<void(const USBDeviceInfo&)>;
    
    USBInfoManager();
    ~USBInfoManager();
    
    // Initialize USB monitoring
    bool Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Start monitoring
    void StartMonitoring();
    
    // Stop monitoring
    void StopMonitoring();
    
    // Get all current USB device information
    std::vector<USBDeviceInfo> GetCurrentUSBDevices();
    
    // Set state change callback
    void SetStateCallback(USBStateCallback callback);
    
    // Check if initialized
    bool IsInitialized() const;
    
private:
    class USBMonitorImpl;
    std::unique_ptr<USBMonitorImpl> pImpl;
    
    // State change callback
    USBStateCallback stateCallback;
    
    // Internal state change handler
    void OnUSBStateChanged(USBState state, const std::string& drivePath);
    
    // Get drive detailed information
    bool GetDriveInfo(const std::string& drivePath, USBDeviceInfo& info);
    
    // Check update folder
    bool HasUpdateFolder(const std::string& drivePath);
};