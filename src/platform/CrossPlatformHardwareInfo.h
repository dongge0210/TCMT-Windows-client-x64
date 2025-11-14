#pragma once

#include "../core/common/PlatformDefs.h"
#include "../core/DataStruct/DataStruct.h"
#include <string>
#include <vector>
#include <memory>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <pdh.h>
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <mach/mach.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <IOKit/IOKitLib.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <proc/readproc.h>
#endif

/**
 * Cross-platform hardware information collector
 * Provides unified interface for collecting system hardware information
 * across Windows, macOS, and Linux platforms
 */
class CrossPlatformHardwareInfo {
public:
    CrossPlatformHardwareInfo();
    ~CrossPlatformHardwareInfo();
    
    // Initialize the collector
    bool Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Collect all system information
    bool CollectSystemInfo(SystemInfo& info);
    
    // CPU information
    bool GetCPUInfo(SystemInfo& info);
    
    // Memory information
    bool GetMemoryInfo(SystemInfo& info);
    
    // GPU information
    bool GetGPUInfo(SystemInfo& info);
    
    // Network information
    bool GetNetworkInfo(SystemInfo& info);
    
    // Disk information
    bool GetDiskInfo(SystemInfo& info);
    
    // Temperature information
    bool GetTemperatureInfo(SystemInfo& info);
    
    // USB information
    bool GetUSBInfo(SystemInfo& info);
    
    // Get last error message
    std::string GetLastError() const;
    
    // Check if initialized
    bool IsInitialized() const;

private:
    bool initialized;
    std::string lastError;
    
    // Platform-specific implementations
    bool GetCPUInfo_Windows(SystemInfo& info);
    bool GetCPUInfo_MacOS(SystemInfo& info);
    bool GetCPUInfo_Linux(SystemInfo& info);
    
    bool GetMemoryInfo_Windows(SystemInfo& info);
    bool GetMemoryInfo_MacOS(SystemInfo& info);
    bool GetMemoryInfo_Linux(SystemInfo& info);
    
    bool GetGPUInfo_Windows(SystemInfo& info);
    bool GetGPUInfo_MacOS(SystemInfo& info);
    bool GetGPUInfo_Linux(SystemInfo& info);
    
    bool GetNetworkInfo_Windows(SystemInfo& info);
    bool GetNetworkInfo_MacOS(SystemInfo& info);
    bool GetNetworkInfo_Linux(SystemInfo& info);
    
    bool GetDiskInfo_Windows(SystemInfo& info);
    bool GetDiskInfo_MacOS(SystemInfo& info);
    bool GetDiskInfo_Linux(SystemInfo& info);
    
    bool GetTemperatureInfo_Windows(SystemInfo& info);
    bool GetTemperatureInfo_MacOS(SystemInfo& info);
    bool GetTemperatureInfo_Linux(SystemInfo& info);
    
    bool GetUSBInfo_Windows(SystemInfo& info);
    bool GetUSBInfo_MacOS(SystemInfo& info);
    bool GetUSBInfo_Linux(SystemInfo& info);
    
    // Helper functions
    std::string ExecuteCommand(const std::string& command);
    uint64_t ParseMemoryValue(const std::string& value);
    double ParseTemperature(const std::string& tempStr);
    
#ifdef PLATFORM_WINDOWS
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotalCounter;
    HANDLE hProcess;
#endif
};