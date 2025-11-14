#pragma once

#include "common/PlatformDefs.h"
#include <string>
#include <vector>
#include <map>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <comdef.h>
    #include <Wbemidl.h>
#elif defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/network/IOEthernetInterface.h>
    #include <IOKit/network/IONetworkInterface.h>
    #include <IOKit/graphics/IOGraphicsLib.h>
    #include <sys/sysctl.h>
    #include <mach/mach.h>
    #include <mach/mach_host.h>
    #include <mach/processor_info.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/sysinfo.h>
    #include <sys/utsname.h>
    #include <fstream>
    #include <dirent.h>
#endif

struct SystemDeviceInfo {
    std::string name;
    std::string description;
    std::string deviceId;
    std::string vendor;
    std::string version;
    std::string driverVersion;
    std::map<std::string, std::string> properties;
};

struct SystemInfoQuery {
    std::string className;
    std::string propertyName;
    std::string filter;
};

class CrossPlatformSystemInfo {
public:
    static CrossPlatformSystemInfo& GetInstance();
    
    // 初始化系统信息管理器
    bool Initialize();
    void Cleanup();
    
    // 通用查询接口
    std::vector<SystemDeviceInfo> QueryDevices(const std::string& deviceClass);
    std::vector<SystemDeviceInfo> QueryDevicesWithFilter(const std::string& deviceClass, const std::string& filter);
    std::string GetDeviceProperty(const std::string& deviceId, const std::string& propertyName);
    
    // 特定设备类型查询
    std::vector<SystemDeviceInfo> GetGpuDevices();
    std::vector<SystemDeviceInfo> GetNetworkAdapters();
    std::vector<SystemDeviceInfo> GetStorageDevices();
    std::vector<SystemDeviceInfo> GetUsbDevices();
    std::vector<SystemDeviceInfo> GetTemperatureSensors();
    std::vector<SystemDeviceInfo> GetMemoryModules();
    std::vector<SystemDeviceInfo> GetCpuInfo();
    
    // 系统性能信息
    struct PerformanceInfo {
        uint64_t totalMemory;
        uint64_t availableMemory;
        uint64_t cpuUsage;
        double cpuTemperature;
        uint64_t diskReadBytes;
        uint64_t diskWriteBytes;
        uint64_t networkBytesReceived;
        uint64_t networkBytesSent;
    };
    
    PerformanceInfo GetPerformanceInfo();
    
    // 特殊文件和路径支持
    struct SpecialPath {
        std::string path;
        std::string description;
        bool exists;
    };
    
    std::vector<SpecialPath> GetSpecialPaths();
    std::vector<std::string> GetConfigFiles(const std::string& pattern);
    std::vector<std::string> GetLogFiles(const std::string& pattern);
    std::vector<std::string> GetDriverFiles();
    std::vector<std::string> GetSystemFiles();
    
    // 硬件特定功能
    bool IsVirtualMachine();
    std::string GetHypervisorType();
    std::vector<std::string> GetVirtualGpuNames();
    
private:
    CrossPlatformSystemInfo();
    ~CrossPlatformSystemInfo();
    
#ifdef PLATFORM_WINDOWS
    // Windows WMI 实现
    bool InitializeWMI();
    void CleanupWMI();
    std::vector<SystemDeviceInfo> ExecuteWMIQuery(const std::wstring& query);
    std::vector<SystemDeviceInfo> ExecuteWMIQueryWithFilter(const std::wstring& query, const std::wstring& filter);
    
    IWbemLocator* m_pLoc;
    IWbemServices* m_pSvc;
    IEnumWbemClassObject* m_pEnumerator;
    
#elif defined(__APPLE__)
    // macOS IOKit 实现
    bool InitializeIOKit();
    void CleanupIOKit();
    std::vector<SystemDeviceInfo> QueryIOKitDevices(const io_name_t className);
    std::vector<SystemDeviceInfo> QueryIOKitDevicesWithProperties(const io_name_t className, const std::map<io_name_t, io_name_t>& properties);
    std::string GetIOKitProperty(io_registry_entry_t entry, const io_name_t propertyName);
    std::string CFStringToString(CFStringRef cfString);
    
    io_iterator_t m_serviceIterator;
    
#elif defined(__linux__)
    // Linux /proc 和 sysfs 实现
    std::vector<SystemDeviceInfo> QueryProcDevices(const std::string& procPath);
    std::vector<SystemDeviceInfo> QuerySysfsDevices(const std::string& sysfsPath);
    std::string ReadSysfsProperty(const std::string& path);
    std::map<std::string, std::string> ParseProcLine(const std::string& line);
#endif
    
    // 通用辅助函数
    std::string TrimString(const std::string& str);
    std::vector<std::string> SplitString(const std::string& str, char delimiter);
    std::string ToLower(const std::string& str);
    bool StringContains(const std::string& str, const std::string& substr);
    
    bool m_initialized;
};