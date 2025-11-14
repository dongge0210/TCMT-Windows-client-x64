#include "MemoryInfo.h"
#include "../Utils/Logger.h"
#include "../Utils/CrossPlatformSystemInfo.h"

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    typedef unsigned long long ULONGLONG;
#elif defined(__APPLE__)
    #include <sys/sysctl.h>
    #include <mach/mach.h>
    #include <mach/mach_types.h>
    #include <mach/mach_host.h>
    #include <unistd.h>
    #include <sys/utsname.h>
    #include <fstream>
    #include <sstream>
    typedef unsigned long long ULONGLONG;
#elif defined(__linux__)
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
    typedef unsigned long long ULONGLONG;
#endif

MemoryInfo::MemoryInfo() {
    try {
#ifdef PLATFORM_WINDOWS
        memStatus.dwLength = sizeof(memStatus);
        GlobalMemoryStatusEx(&memStatus);
#elif defined(PLATFORM_MACOS)
        hostPort = mach_host_self();
        vmStatsValid = false;
        UpdateMacMemoryStats();
#elif defined(PLATFORM_LINUX)
        sysinfo(&sysInfo);
#endif
    }
    catch (const std::exception& e) {
        Logger::Error("MemoryInfo初始化失败: " + std::string(e.what()));
    }
}

// 跨平台基础内存信息实现
ULONGLONG MemoryInfo::GetTotalPhysical() const {
#ifdef PLATFORM_WINDOWS
    return memStatus.ullTotalPhys;
#elif defined(PLATFORM_MACOS)
    if (!vmStatsValid) {
        const_cast<MemoryInfo*>(this)->UpdateMacMemoryStats();
    }
    return vmStats.active_count + vmStats.inactive_count + vmStats.wire_count + 
           vmStats.free_count + vmStats.compressor_page_count;
#elif defined(PLATFORM_LINUX)
    return sysInfo.totalram * sysInfo.mem_unit;
#else
    return 0;
#endif
}

ULONGLONG MemoryInfo::GetAvailablePhysical() const {
#ifdef PLATFORM_WINDOWS
    return memStatus.ullAvailPhys;
#elif defined(PLATFORM_MACOS)
    if (!vmStatsValid) {
        const_cast<MemoryInfo*>(this)->UpdateMacMemoryStats();
    }
    return vmStats.free_count;
#elif defined(PLATFORM_LINUX)
    return sysInfo.freeram * sysInfo.mem_unit;
#else
    return 0;
#endif
}

ULONGLONG MemoryInfo::GetTotalVirtual() const {
#ifdef PLATFORM_WINDOWS
    return memStatus.ullTotalVirtual;
#elif defined(PLATFORM_MACOS)
    // macOS上虚拟内存通常很大，返回交换空间+物理内存
    return GetTotalPhysical() + GetSwapTotal();
#elif defined(PLATFORM_LINUX)
    return sysInfo.totalswap * sysInfo.mem_unit;
#else
    return 0;
#endif
}

// macOS特有：内存分区信息实现
ULONGLONG MemoryInfo::GetActiveMemory() const {
#ifdef PLATFORM_MACOS
    if (!vmStatsValid) {
        const_cast<MemoryInfo*>(this)->UpdateMacMemoryStats();
    }
    return vmStats.active_count * vm_page_size;
#else
    return 0;
#endif
}

ULONGLONG MemoryInfo::GetInactiveMemory() const {
#ifdef PLATFORM_MACOS
    if (!vmStatsValid) {
        const_cast<MemoryInfo*>(this)->UpdateMacMemoryStats();
    }
    return vmStats.inactive_count * vm_page_size;
#else
    return 0;
#endif
}

ULONGLONG MemoryInfo::GetWiredMemory() const {
#ifdef PLATFORM_MACOS
    if (!vmStatsValid) {
        const_cast<MemoryInfo*>(this)->UpdateMacMemoryStats();
    }
    return vmStats.wire_count * vm_page_size;
#else
    return 0;
#endif
}

ULONGLONG MemoryInfo::GetCompressedMemory() const {
#ifdef PLATFORM_MACOS
    if (!vmStatsValid) {
        const_cast<MemoryInfo*>(this)->UpdateMacMemoryStats();
    }
    return vmStats.compressor_page_count * vm_page_size;
#else
    return 0;
#endif
}

ULONGLONG MemoryInfo::GetFreeMemory() const {
#ifdef PLATFORM_MACOS
    if (!vmStatsValid) {
        const_cast<MemoryInfo*>(this)->UpdateMacMemoryStats();
    }
    return vmStats.free_count * vm_page_size;
#else
    return 0;
#endif
}

// macOS特有：内存压力和交换信息实现
double MemoryInfo::GetMemoryPressure() const {
#ifdef PLATFORM_MACOS
    // 使用macOS系统的内存压力检测
    try {
        // 获取内存压力信息
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        vm_statistics64_data_t vmStats;
        kern_return_t result = host_statistics64(mach_host_self(), HOST_VM_INFO64, 
                                                (host_info64_t)&vmStats, &count);
        
        if (result == KERN_SUCCESS) {
            // 计算可用内存（包括可回收的缓存）
            uint64_t freeMemory = vmStats.free_count;
            uint64_t inactiveMemory = vmStats.inactive_count;
            uint64_t purgeableMemory = vmStats.purgeable_count;
            
            // 获取总物理内存
            uint64_t totalMemory = 0;
            size_t size = sizeof(totalMemory);
            if (sysctlbyname("hw.memsize", &totalMemory, &size, nullptr, 0) == 0) {
                totalMemory = totalMemory / vm_page_size;
                
                // 计算内存压力百分比
                uint64_t availableMemory = freeMemory + inactiveMemory + purgeableMemory;
                double pressure = 100.0 - (double)availableMemory / (double)totalMemory * 100.0;
                
                // 限制在0-100范围内
                if (pressure < 0.0) pressure = 0.0;
                if (pressure > 100.0) pressure = 100.0;
                
                return pressure;
            }
        }
    }
    catch (const std::exception& e) {
        Logger::Error("获取macOS内存压力失败: " + std::string(e.what()));
    }
    
    // 如果获取失败，返回保守估计
    return 50.0;
#else
    return 0.0;
#endif
}

ULONGLONG MemoryInfo::GetSwapUsed() const {
#ifdef PLATFORM_MACOS
    return GetMacSwapUsed();
#else
    return 0;
#endif
}

ULONGLONG MemoryInfo::GetSwapTotal() const {
#ifdef PLATFORM_MACOS
    return GetMacSwapTotal();
#else
    return 0;
#endif
}

bool MemoryInfo::IsMemoryPressureWarning() const {
#ifdef PLATFORM_MACOS
    double pressure = GetMemoryPressure();
    return pressure > 75.0; // 超过75%视为警告
#else
    return false;
#endif
}

bool MemoryInfo::IsMemoryPressureCritical() const {
#ifdef PLATFORM_MACOS
    double pressure = GetMemoryPressure();
    return pressure > 90.0; // 超过90%视为严重
#else
    return false;
#endif
}

// macOS私有方法实现
#ifdef PLATFORM_MACOS
void MemoryInfo::UpdateMacMemoryStats() {
    try {
        // 获取虚拟内存统计信息
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        kern_return_t result = host_statistics64(hostPort, HOST_VM_INFO64, 
                                                (host_info64_t)&vmStats, &count);
        
        if (result == KERN_SUCCESS) {
            vmStatsValid = true;
        } else {
            Logger::Warn("获取macOS虚拟内存统计失败: " + std::to_string(result));
            vmStatsValid = false;
        }
    }
    catch (const std::exception& e) {
        Logger::Error("UpdateMacMemoryStats异常: " + std::string(e.what()));
        vmStatsValid = false;
    }
}

uint64_t MemoryInfo::GetMacSwapUsed() const {
    try {
        int mib[2] = {CTL_VM, VM_SWAPUSAGE};
        struct xsw_usage swapUsage;
        size_t len = sizeof(swapUsage);
        
        if (sysctl(mib, 2, &swapUsage, &len, nullptr, 0) == 0) {
            return swapUsage.xsu_used;
        }
    }
    catch (const std::exception& e) {
        Logger::Error("获取macOS交换使用量失败: " + std::string(e.what()));
    }
    return 0;
}

uint64_t MemoryInfo::GetMacSwapTotal() const {
    try {
        int mib[2] = {CTL_VM, VM_SWAPUSAGE};
        struct xsw_usage swapUsage;
        size_t len = sizeof(swapUsage);
        
        if (sysctl(mib, 2, &swapUsage, &len, nullptr, 0) == 0) {
            return swapUsage.xsu_total;
        }
    }
    catch (const std::exception& e) {
        Logger::Error("获取macOS交换总量失败: " + std::string(e.what()));
    }
    return 0;
}
#endif

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           