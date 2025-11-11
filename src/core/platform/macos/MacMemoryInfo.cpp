#include "MacMemoryInfo.h"
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#ifdef PLATFORM_MACOS

MacMemoryInfo::MacMemoryInfo() : m_totalPhysicalMemory(0), m_availablePhysicalMemory(0), 
    m_totalVirtualMemory(0), m_availableVirtualMemory(0), m_totalSwapMemory(0), 
    m_availableSwapMemory(0), m_cachedMemory(0), m_bufferedMemory(0), 
    m_sharedMemory(0), m_memoryPressure(0.0), m_memoryType("Unknown"), 
    m_memoryChannels(0), m_memorySpeed(0.0), m_initialized(false), 
    m_dataValid(false), m_lastUpdateTime(0) {
}

MacMemoryInfo::~MacMemoryInfo() {
    Cleanup();
}

uint64_t MacMemoryInfo::GetTotalPhysicalMemory() const {
    if (!m_initialized && !GetMemoryInfo()) {
        return 0;
    }
    return m_totalPhysicalMemory;
}

uint64_t MacMemoryInfo::GetAvailablePhysicalMemory() const {
    if (!m_initialized && !GetMemoryInfo()) {
        return 0;
    }
    return m_availablePhysicalMemory;
}

uint64_t MacMemoryInfo::GetUsedPhysicalMemory() const {
    return GetTotalPhysicalMemory() - GetAvailablePhysicalMemory();
}

double MacMemoryInfo::GetPhysicalMemoryUsage() const {
    uint64_t total = GetTotalPhysicalMemory();
    if (total == 0) return 0.0;
    return (static_cast<double>(GetUsedPhysicalMemory()) / static_cast<double>(total)) * 100.0;
}

uint64_t MacMemoryInfo::GetTotalVirtualMemory() const {
    if (!m_initialized && !GetMemoryInfo()) {
        return 0;
    }
    return m_totalVirtualMemory;
}

uint64_t MacMemoryInfo::GetAvailableVirtualMemory() const {
    if (!m_initialized && !GetMemoryInfo()) {
        return 0;
    }
    return m_availableVirtualMemory;
}

uint64_t MacMemoryInfo::GetUsedVirtualMemory() const {
    return GetTotalVirtualMemory() - GetAvailableVirtualMemory();
}

double MacMemoryInfo::GetVirtualMemoryUsage() const {
    uint64_t total = GetTotalVirtualMemory();
    if (total == 0) return 0.0;
    return (static_cast<double>(GetUsedVirtualMemory()) / static_cast<double>(total)) * 100.0;
}

uint64_t MacMemoryInfo::GetTotalSwapMemory() const {
    if (!m_initialized && !GetSwapInfo()) {
        return 0;
    }
    return m_totalSwapMemory;
}

uint64_t MacMemoryInfo::GetAvailableSwapMemory() const {
    if (!m_initialized && !GetSwapInfo()) {
        return 0;
    }
    return m_availableSwapMemory;
}

uint64_t MacMemoryInfo::GetUsedSwapMemory() const {
    return GetTotalSwapMemory() - GetAvailableSwapMemory();
}

double MacMemoryInfo::GetSwapMemoryUsage() const {
    uint64_t total = GetTotalSwapMemory();
    if (total == 0) return 0.0;
    return (static_cast<double>(GetUsedSwapMemory()) / static_cast<double>(total)) * 100.0;
}

double MacMemoryInfo::GetMemorySpeed() const {
    if (!m_initialized && !GetMemoryDetails()) {
        return 0.0;
    }
    return m_memorySpeed;
}

std::string MacMemoryInfo::GetMemoryType() const {
    if (!m_initialized && !GetMemoryDetails()) {
        return "Unknown";
    }
    return m_memoryType;
}

uint32_t MacMemoryInfo::GetMemoryChannels() const {
    if (!m_initialized && !GetMemoryDetails()) {
        return 0;
    }
    return m_memoryChannels;
}

uint64_t MacMemoryInfo::GetCachedMemory() const {
    if (!m_initialized && !GetMemoryDetails()) {
        return 0;
    }
    return m_cachedMemory;
}

uint64_t MacMemoryInfo::GetBufferedMemory() const {
    if (!m_initialized && !GetMemoryDetails()) {
        return 0;
    }
    return m_bufferedMemory;
}

uint64_t MacMemoryInfo::GetSharedMemory() const {
    if (!m_initialized && !GetMemoryDetails()) {
        return 0;
    }
    return m_sharedMemory;
}

double MacMemoryInfo::GetMemoryPressure() const {
    if (!m_initialized && !GetMemoryDetails()) {
        return 0.0;
    }
    return m_memoryPressure;
}

bool MacMemoryInfo::IsMemoryLow() const {
    double usage = GetPhysicalMemoryUsage();
    return usage > 80.0; // 超过80%认为内存不足
}

bool MacMemoryInfo::IsMemoryCritical() const {
    double usage = GetPhysicalMemoryUsage();
    return usage > 95.0; // 超过95%认为内存危急
}

bool MacMemoryInfo::Initialize() {
    return GetMemoryInfo() && GetSwapInfo() && GetMemoryDetails();
}

void MacMemoryInfo::Cleanup() {
    m_initialized = false;
}

bool MacMemoryInfo::IsInitialized() const {
    return m_initialized;
}

bool MacMemoryInfo::IsDataValid() const {
    return m_dataValid;
}

uint64_t MacMemoryInfo::GetLastUpdateTime() const {
    return m_lastUpdateTime;
}

std::string MacMemoryInfo::GetLastError() const {
    return m_lastError;
}

void MacMemoryInfo::ClearError() {
    m_lastError.clear();
}

bool MacMemoryInfo::Update() {
    return GetMemoryInfo() && GetSwapInfo() && GetMemoryDetails();
}

bool MacMemoryInfo::GetMemoryInfo() const {
    // 获取物理内存信息
    int mib[2];
    size_t length;
    
    // 获取总物理内存
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(m_totalPhysicalMemory);
    if (sysctl(mib, 2, &m_totalPhysicalMemory, &length, NULL, 0) != 0) {
        return false;
    }
    
    // 获取可用内存使用vm_stat
    vm_statistics64_data_t vm_stat;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, 
                         (host_info64_t)&vm_stat, &count) != KERN_SUCCESS) {
        return false;
    }
    
    // 计算可用内存
    uint64_t pageSize = sysconf(_SC_PAGESIZE);
    
    // 可用内存 = 空闲内存 + 可回收的非活跃内存的一部分
    // 非活跃内存中只有一部分是真正可用的
    uint64_t freeMemory = vm_stat.free_count * pageSize;
    uint64_t inactiveMemory = vm_stat.inactive_count * pageSize;
    
    // 假设非活跃内存的70%是可回收的
    uint64_t reclaimableInactive = inactiveMemory * 0.7;
    m_availablePhysicalMemory = freeMemory + reclaimableInactive;
    
    // 获取虚拟内存信息（交换文件）
    xsw_usage vmusage;
    length = sizeof(vmusage);
    if (sysctlbyname("vm.swapusage", &vmusage, &length, NULL, 0) == 0) {
        m_totalVirtualMemory = vmusage.xsu_total;
        m_availableVirtualMemory = m_totalVirtualMemory - vmusage.xsu_used;
    } else {
        // 如果无法获取交换文件信息，使用默认值
        m_totalVirtualMemory = m_totalPhysicalMemory;
        m_availableVirtualMemory = m_totalPhysicalMemory;
    }
    
    return true;
}

bool MacMemoryInfo::GetSwapInfo() const {
    // 获取交换文件信息
    xsw_usage vmusage;
    size_t length = sizeof(vmusage);
    if (sysctlbyname("vm.swapusage", &vmusage, &length, NULL, 0) == 0) {
        m_totalSwapMemory = vmusage.xsu_total;
        m_availableSwapMemory = m_totalSwapMemory - vmusage.xsu_used;
    } else {
        // 默认值
        m_totalSwapMemory = 0;
        m_availableSwapMemory = 0;
    }
    
    return true;
}

bool MacMemoryInfo::GetMemoryDetails() const {
    // 获取内存详情信息
    vm_statistics64_data_t vm_stat;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, 
                         (host_info64_t)&vm_stat, &count) != KERN_SUCCESS) {
        return false;
    }
    
    uint64_t pageSize = sysconf(_SC_PAGESIZE);
    
    // 更详细的内存分类 (不影响可用内存计算)
    m_cachedMemory = vm_stat.inactive_count * pageSize;
    // 注意：不要将空闲内存重新分类为缓存，这会影响可用内存计算
    m_bufferedMemory = 0; // macOS中buffered内存概念不同，设为0避免混淆
    m_sharedMemory = vm_stat.wire_count * pageSize;
    
    // 添加压缩内存支持 (macOS的内存压缩功能)
    uint64_t compressedMemory = vm_stat.compressor_page_count * pageSize;
    if (compressedMemory > 0) {
        // 将压缩内存的一部分计入缓存，但不影响可用内存
        m_cachedMemory += compressedMemory * 0.5;
    }
    
    // 增强的内存压力计算
    double usage = GetPhysicalMemoryUsage();
    double swapUsage = GetSwapMemoryUsage();
    
    // 基于物理内存使用率的基础压力 (权重: 60%)
    double basePressure = usage * 0.6;
    
    // 考虑交换文件使用的影响 (权重: 20%)
    double swapPressure = std::min(swapUsage, 100.0) * 0.2;
    
    // 考虑非活跃内存的比例 (权重: 10%)
    uint64_t totalMemory = GetTotalPhysicalMemory();
    double inactiveRatio = (double)m_cachedMemory / totalMemory * 100.0;
    double inactivePressure = 0.0;
    if (inactiveRatio < 10.0 && usage > 70.0) {
        inactivePressure = 15.0; // 非活跃内存过少且使用率高时增加压力
    }
    inactivePressure *= 0.1;
    
    // 考虑有线内存（不可被换出的内存）的比例 (权重: 10%)
    double wiredRatio = (double)m_sharedMemory / totalMemory * 100.0;
    double wiredPressure = 0.0;
    if (wiredRatio > 30.0) {
        wiredPressure = 10.0; // 有线内存过多增加压力
    }
    wiredPressure *= 0.1;
    
    // 综合计算内存压力
    basePressure = basePressure + swapPressure + inactivePressure + wiredPressure;
    
    // 限制在合理范围内
    m_memoryPressure = std::max(0.0, std::min(100.0, basePressure));
    
    // 获取内存类型和速度信息 (Apple Silicon特有)
    // 对于Apple Silicon，使用统一内存架构
    m_memoryType = "Unified Memory";
    
    // 尝试获取硬件型号信息来确定内存类型
    char hw_model[256];
    size_t modelSize = sizeof(hw_model);
    if (sysctlbyname("hw.model", hw_model, &modelSize, NULL, 0) == 0) {
        std::string model(hw_model);
        if (model.find("MacBook") != std::string::npos || 
            model.find("Mac mini") != std::string::npos ||
            model.find("MacBookAir") != std::string::npos ||
            model.find("MacBookPro") != std::string::npos) {
            // 对于Apple Silicon Mac，使用LPDDR
            m_memoryType = "LPDDR5";
        }
    }
    
    // 对于Apple Silicon，使用统一内存架构
    // 内存速度通常与CPU性能相关
    size_t cpufreqSize = sizeof(uint64_t);
    uint64_t cpufreq = 0;
    if (sysctlbyname("hw.cpufrequency", &cpufreq, &cpufreqSize, NULL, 0) == 0) {
        // 简化的内存速度估算
        m_memorySpeed = cpufreq / 1000000.0 / 4.0; // 假设内存速度为CPU频率的1/4
    }
    
    // 内存通道数 (Apple Silicon统一内存)
    m_memoryChannels = 1;
    
    return true;
}

bool MacMemoryInfo::AnalyzeMemoryHealth() const {
    // 分析内存健康状态
    double physicalUsage = GetPhysicalMemoryUsage();
    double swapUsage = GetSwapMemoryUsage();
    double pressure = GetMemoryPressure();
    
    // 检查是否有内存压力过大
    if (pressure > 80.0) {
        return false;
    }
    
    // 检查是否有过多的交换文件使用
    if (swapUsage > 60.0) {
        return false;
    }
    
    // 检查是否有足够的可用内存
    if (physicalUsage > 85.0) {
        return false;
    }
    
    return true;
}

std::string MacMemoryInfo::GetMemoryStatusDescription() const {
    double usage = GetPhysicalMemoryUsage();
    double pressure = GetMemoryPressure();
    
    if (pressure > 80.0) {
        return "内存压力过大";
    } else if (pressure > 60.0) {
        return "内存使用较高";
    } else if (pressure > 40.0) {
        return "内存使用正常";
    } else {
        return "内存使用较低";
    }
}

double MacMemoryInfo::GetMemoryEfficiency() const {
    // 计算内存效率指标
    double usage = GetPhysicalMemoryUsage();
    uint64_t totalMemory = GetTotalPhysicalMemory();
    uint64_t availableMemory = GetAvailablePhysicalMemory();
    uint64_t cachedMemory = GetCachedMemory();
    
    // 效率 = 使用的内存 / (使用的内存 + 缓存内存)
    double efficiency = usage / (usage + (double)cachedMemory / totalMemory * 100.0) * 100.0;
    
    return std::max(0.0, std::min(100.0, efficiency));
}

#endif // PLATFORM_MACOS