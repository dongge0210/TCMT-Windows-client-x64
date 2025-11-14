#pragma once
#include "../common/PlatformDefs.h"

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    typedef unsigned long long ULONGLONG;
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <mach/mach.h>
    #include <mach/mach_types.h>
    #include <mach/mach_host.h>
    #include <sys/utsname.h>
    typedef unsigned long long ULONGLONG;
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
    typedef unsigned long long ULONGLONG;
#endif

class MemoryInfo {
public:
    MemoryInfo();
    
    // 跨平台基础内存信息
    ULONGLONG GetTotalPhysical() const;
    ULONGLONG GetAvailablePhysical() const;
    ULONGLONG GetTotalVirtual() const;
    
    // macOS特有：内存分区信息
    ULONGLONG GetActiveMemory() const;
    ULONGLONG GetInactiveMemory() const;
    ULONGLONG GetWiredMemory() const;
    ULONGLONG GetCompressedMemory() const;
    ULONGLONG GetFreeMemory() const;
    
    // macOS特有：内存压力和交换信息
    double GetMemoryPressure() const;
    ULONGLONG GetSwapUsed() const;
    ULONGLONG GetSwapTotal() const;
    bool IsMemoryPressureWarning() const;
    bool IsMemoryPressureCritical() const;

private:
#ifdef PLATFORM_WINDOWS
    MEMORYSTATUSEX memStatus;
#elif defined(PLATFORM_MACOS)
    vm_statistics64_data_t vmStats;
    vm_statistics64_data_t hostStats;
    mach_port_t hostPort;
    bool vmStatsValid;
#elif defined(PLATFORM_LINUX)
    struct sysinfo sysInfo;
#endif

    // macOS私有方法
    void UpdateMacMemoryStats();
    uint64_t GetMacSwapUsed() const;
    uint64_t GetMacSwapTotal() const;
};                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               