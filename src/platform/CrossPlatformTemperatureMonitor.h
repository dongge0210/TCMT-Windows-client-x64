#pragma once

#include "../core/common/TCMTCoreExports.h"
#include "../core/DataStruct/DataStruct.h"
#include <vector>
#include <string>
#include <memory>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <IOKit/IOKitLib.h>
    #include <CoreFoundation/CoreFoundation.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
#endif

/**
 * Cross-platform temperature monitoring
 * Provides unified interface for collecting temperature information
 * across different platforms
 */
class CrossPlatformTemperatureMonitor {
public:
    CrossPlatformTemperatureMonitor();
    ~CrossPlatformTemperatureMonitor();
    
    // Initialize the monitor
    bool Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Collect temperature information
    bool CollectTemperatures(std::vector<std::pair<std::string, double>>& temperatures);
    
    // Get CPU temperature
    double GetCPUTemperature();
    
    // Get GPU temperature
    double GetGPUTemperature();
    
    // Get all temperature sensors
    std::vector<std::pair<std::string, double>> GetAllTemperatures();
    
    // Get last error message
    std::string GetLastError() const;
    
    // Check if initialized
    bool IsInitialized() const;

private:
    bool initialized;
    std::string lastError;
    
    // Platform-specific implementations
    bool Initialize_Windows();
    bool Initialize_MacOS();
    bool Initialize_Linux();
    
    bool CollectTemperatures_Windows(std::vector<std::pair<std::string, double>>& temperatures);
    bool CollectTemperatures_MacOS(std::vector<std::pair<std::string, double>>& temperatures);
    bool CollectTemperatures_Linux(std::vector<std::pair<std::string, double>>& temperatures);
    
    double GetCPUTemperature_Windows();
    double GetCPUTemperature_MacOS();
    double GetCPUTemperature_Linux();
    
    double GetGPUTemperature_Windows();
    double GetGPUTemperature_MacOS();
    double GetGPUTemperature_Linux();
    
    // Helper functions
    std::string ExecuteCommand(const std::string& command);
    double ParseTemperature(const std::string& tempStr);
    
#ifdef PLATFORM_WINDOWS
    // Windows specific members
    std::vector<void*> temperatureSensors;
    
#elif defined(PLATFORM_MACOS)
    // macOS specific members
    io_iterator_t iterator;
    
#elif defined(PLATFORM_LINUX)
    // Linux specific members
    std::vector<std::string> temperaturePaths;
#endif
};

// Global temperature monitoring interface
TCMT_CORE_API bool TCMT_InitializeTemperatureMonitor();
TCMT_CORE_API void TCMT_CleanupTemperatureMonitor();
TCMT_CORE_API bool TCMT_GetTemperatures(std::vector<std::pair<std::string, double>>& temperatures);
TCMT_CORE_API double TCMT_GetCPUTemperature();
TCMT_CORE_API double TCMT_GetGPUTemperature();