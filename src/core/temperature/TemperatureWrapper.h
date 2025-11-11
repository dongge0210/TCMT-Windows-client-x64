#pragma once
#include "../common/PlatformDefs.h"
#include <string>
#include <vector>
#include <utility>

#ifdef PLATFORM_WINDOWS
    #include "../gpu/GpuInfo.h"
    class WmiManager; // 前向声明
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/graphics/IOGraphicsLib.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <unistd.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
#endif

// 跨平台温度监控包装器类
class TemperatureWrapper {
public:
    static void Initialize();
    static void Cleanup();
    static std::vector<std::pair<std::string, double>> GetTemperatures();
    static bool IsInitialized();

private:
    static bool initialized;
    
#ifdef PLATFORM_WINDOWS
    static GpuInfo* gpuInfo;
    static WmiManager* wmiManager;
    static int temperatureCallCount;
    static void LogRealGpuNames(const std::vector<GpuInfo::GpuData>& gpus, bool isDetailedLogging);
#elif defined(PLATFORM_MACOS)
    static std::vector<std::pair<std::string, double>> GetMacTemperatures();
    static double GetMacCpuTemperature();
    static double GetMacGpuTemperature();
    static std::vector<std::pair<std::string, double>> GetMacSensorTemperatures();
    static double GetMacSensorTemperature(const std::string& sensorPath);
    static std::string GetMacSensorName(io_service_t service);
    static void LogMacSensorInfo(const std::vector<std::pair<std::string, double>>& sensors);
#elif defined(PLATFORM_LINUX)
    static std::vector<std::pair<std::string, double>> GetLinuxTemperatures();
    static double GetLinuxCpuTemperature();
    static std::vector<std::pair<std::string, double>> GetLinuxSensorTemperatures();
    static double GetLinuxSensorTemperature(const std::string& sensorPath);
#endif
};
