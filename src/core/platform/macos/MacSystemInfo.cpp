#include "MacSystemInfo.h"
#include "../../Utils/Logger.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <sys/vmmeter.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <pwd.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <mach/mach.h>
#include <mach/host_info.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <net/route.h>
#include <thread>

#ifdef PLATFORM_MACOS

MacSystemInfo::MacSystemInfo() {
    m_initialized = false;
    m_lastUpdateTime = 0;
    m_uptimeSeconds = 0;
    m_processCount = 0;
    m_runningProcessCount = 0;
    m_sleepingProcessCount = 0;
    m_threadCount = 0;
    m_maxProcesses = 0;
    m_totalProcesses = 0;
    m_runningProcesses = 0;
    m_sleepingProcesses = 0;
    m_totalMemory = 0;
    m_availableMemory = 0;
    m_usedMemory = 0;
    m_cacheMemory = 0;
    m_swapMemory = 0;
    m_memoryPressure = 0.0;
    m_totalPhysicalMemory = 0;
    m_availablePhysicalMemory = 0;
    m_usedPhysicalMemory = 0;
    m_totalSwapMemory = 0;
    m_availableSwapMemory = 0;
    m_usedSwapMemory = 0;
    m_totalDiskSpace = 0;
    m_availableDiskSpace = 0;
    m_usedDiskSpace = 0;
    m_diskReadOps = 0;
    m_diskWriteOps = 0;
    m_diskReadBytes = 0;
    m_diskWriteBytes = 0;
    m_networkInterfaceCount = 0;
    m_totalBytesReceived = 0;
    m_totalBytesSent = 0;
    m_networkUtilization = 0.0;
    m_networkBytesReceived = 0;
    m_networkBytesSent = 0;
    m_networkPacketsReceived = 0;
    m_networkPacketsSent = 0;
    m_systemHealthScore = 0.0;
    m_isVirtualMachine = false;
    m_virtualCPUCount = 0;
    m_virtualMemory = 0;
    
    // 初始化负载平均值
    for (int i = 0; i < 3; ++i) {
        m_loadAverage[i] = 0.0;
    }
    
    // 初始化主网络接口
    m_primaryInterface = "en0";
    
    Initialize();
}

MacSystemInfo::~MacSystemInfo() {
    Cleanup();
}

bool MacSystemInfo::Initialize() {
    try {
        ClearErrorInternal();
        
        // 获取系统基本信息
        if (!UpdateOSInfo()) {
            SetError("Failed to get OS information");
            return false;
        }
        
        // 获取主机名和用户名
        if (!UpdateHostname()) {
            SetError("Failed to get hostname");
            return false;
        }
        
        if (!UpdateUsername()) {
            SetError("Failed to get username");
            return false;
        }
        
        // 获取初始系统信息
        if (!Update()) {
            SetError("Failed to update initial system information");
            return false;
        }
        
        m_initialized = true;
        Logger::Info("MacSystemInfo initialized successfully for: " + m_hostname);
        return true;
    }
    catch (const std::exception& e) {
        SetError("MacSystemInfo initialization failed: " + std::string(e.what()));
        return false;
    }
}

void MacSystemInfo::Cleanup() {
    m_initialized = false;
    ClearErrorInternal();
}

bool MacSystemInfo::IsInitialized() const {
    return m_initialized;
}

bool MacSystemInfo::Update() {
    if (!m_initialized) {
        SetError("MacSystemInfo not initialized");
        return false;
    }
    
    bool success = true;
    
    // 更新运行时间
    if (!UpdateUptime()) {
        success = false;
    }
    
    // 更新系统负载
    if (!GetLoadAverages()) {
        success = false;
    }
    
    // 更新进程信息
    if (!GetProcessInfo()) {
        success = false;
    }
    
    // 更新内存信息
    if (!GetMemoryInfo()) {
        success = false;
    }
    
    // 更新磁盘信息
    if (!GetDiskInfo()) {
        success = false;
    }
    
    // 更新网络信息
    if (!GetNetworkInfo()) {
        success = false;
    }
    
    m_lastUpdateTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    return success;
}

bool MacSystemInfo::IsDataValid() const {
    return m_initialized && (m_lastUpdateTime > 0);
}

std::string MacSystemInfo::GetOSName() const {
    return m_osName;
}

std::string MacSystemInfo::GetOSVersion() const {
    return m_osVersion;
}

std::string MacSystemInfo::GetOSBuild() const {
    return m_osBuild;
}

std::string MacSystemInfo::GetArchitecture() const {
    if (m_architecture.empty()) {
        struct utsname uname_info;
        if (uname(&uname_info) == 0) {
            m_architecture = uname_info.machine;
        }
    }
    return m_architecture;
}

std::string MacSystemInfo::GetHostname() const {
    return m_hostname;
}

std::string MacSystemInfo::GetDomain() const {
    if (m_domain.empty()) {
        // 简化实现，返回空字符串
        m_domain = "";
    }
    return m_domain;
}

std::string MacSystemInfo::GetUsername() const {
    return m_username;
}

uint64_t MacSystemInfo::GetUptimeSeconds() const {
    return m_uptimeSeconds;
}

std::string MacSystemInfo::GetBootTime() const {
    if (m_bootTime == std::chrono::system_clock::time_point{}) {
        return "Unknown";
    }
    
    auto tt = std::chrono::system_clock::to_time_t(m_bootTime);
    std::tm tm{};
    localtime_r(&tt, &tm);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string MacSystemInfo::GetBootTimeFormatted() const {
    if (m_bootTime == std::chrono::system_clock::time_point{}) {
        return "Unknown";
    }
    
    auto tt = std::chrono::system_clock::to_time_t(m_bootTime);
    std::tm tm{};
    localtime_r(&tt, &tm);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string MacSystemInfo::GetUptimeFormatted() const {
    uint64_t seconds = m_uptimeSeconds;
    uint64_t days = seconds / 86400;
    uint64_t hours = (seconds % 86400) / 3600;
    uint64_t minutes = (seconds % 3600) / 60;
    
    std::ostringstream oss;
    if (days > 0) {
        oss << days << "天 ";
    }
    if (hours > 0 || days > 0) {
        oss << hours << "小时 ";
    }
    oss << minutes << "分钟";
    
    return oss.str();
}

std::string MacSystemInfo::GetLocalTime() const {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&tt, &tm);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string MacSystemInfo::GetUTCTime() const {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&tt, &tm);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S UTC");
    return oss.str();
}

std::string MacSystemInfo::GetTimezone() const {
    std::time_t now = std::time(nullptr);
    std::tm local_tm{};
    localtime_r(&now, &local_tm);
    
    char tz[32];
    strftime(tz, sizeof(tz), "%Z", &local_tm);
    return std::string(tz);
}

double MacSystemInfo::GetLoadAverage1Min() const {
    return m_loadAverage[0];
}

double MacSystemInfo::GetLoadAverage5Min() const {
    return m_loadAverage[1];
}

double MacSystemInfo::GetLoadAverage15Min() const {
    return m_loadAverage[2];
}

double MacSystemInfo::GetCPULoadAverage() const {
    // 简化实现，返回1分钟负载平均值
    return m_loadAverage[0];
}

uint32_t MacSystemInfo::GetProcessCount() const {
    return m_processCount;
}

uint32_t MacSystemInfo::GetRunningProcessCount() const {
    return m_runningProcessCount;
}

uint32_t MacSystemInfo::GetSleepingProcessCount() const {
    return m_sleepingProcessCount;
}

uint32_t MacSystemInfo::GetThreadCount() const {
    return m_threadCount;
}

uint32_t MacSystemInfo::GetMaxProcesses() const {
    if (m_maxProcesses == 0) {
        size_t size = sizeof(m_maxProcesses);
        if (sysctlbyname("kern.maxproc", nullptr, &size, nullptr, 0) == 0) {
            sysctlbyname("kern.maxproc", nullptr, &size, &m_maxProcesses, size);
        }
    }
    return m_maxProcesses;
}

uint32_t MacSystemInfo::GetTotalProcesses() const {
    return m_totalProcesses;
}

uint32_t MacSystemInfo::GetRunningProcesses() const {
    return m_runningProcesses;
}

uint32_t MacSystemInfo::GetSleepingProcesses() const {
    return m_sleepingProcesses;
}

uint64_t MacSystemInfo::GetTotalPhysicalMemory() const {
    return m_totalPhysicalMemory;
}

uint64_t MacSystemInfo::GetAvailablePhysicalMemory() const {
    return m_availablePhysicalMemory;
}

uint64_t MacSystemInfo::GetUsedPhysicalMemory() const {
    return m_usedPhysicalMemory;
}

double MacSystemInfo::GetMemoryUsagePercentage() const {
    if (m_totalPhysicalMemory == 0) return 0.0;
    return (double)m_usedPhysicalMemory / m_totalPhysicalMemory * 100.0;
}

uint64_t MacSystemInfo::GetTotalMemory() const {
    return m_totalMemory;
}

uint64_t MacSystemInfo::GetAvailableMemory() const {
    return m_availableMemory;
}

uint64_t MacSystemInfo::GetUsedMemory() const {
    return m_usedMemory;
}

uint64_t MacSystemInfo::GetCacheMemory() const {
    return m_cacheMemory;
}

uint64_t MacSystemInfo::GetSwapMemory() const {
    return m_swapMemory;
}

double MacSystemInfo::GetMemoryPressure() const {
    return m_memoryPressure;
}

uint64_t MacSystemInfo::GetTotalDiskSpace() const {
    return m_totalDiskSpace;
}

uint64_t MacSystemInfo::GetAvailableDiskSpace() const {
    return m_availableDiskSpace;
}

uint64_t MacSystemInfo::GetUsedDiskSpace() const {
    return m_usedDiskSpace;
}

double MacSystemInfo::GetDiskUsagePercentage() const {
    if (m_totalDiskSpace == 0) return 0.0;
    return (double)m_usedDiskSpace / m_totalDiskSpace * 100.0;
}

uint32_t MacSystemInfo::GetDiskReadOps() const {
    return m_diskReadOps;
}

uint32_t MacSystemInfo::GetDiskWriteOps() const {
    return m_diskWriteOps;
}

uint64_t MacSystemInfo::GetDiskReadBytes() const {
    return m_diskReadBytes;
}

uint64_t MacSystemInfo::GetDiskWriteBytes() const {
    return m_diskWriteBytes;
}

std::string MacSystemInfo::GetPrimaryInterface() const {
    return m_primaryInterface;
}

uint64_t MacSystemInfo::GetNetworkBytesReceived() const {
    return m_networkBytesReceived;
}

uint64_t MacSystemInfo::GetNetworkBytesSent() const {
    return m_networkBytesSent;
}

uint64_t MacSystemInfo::GetNetworkPacketsReceived() const {
    return m_networkPacketsReceived;
}

uint64_t MacSystemInfo::GetNetworkPacketsSent() const {
    return m_networkPacketsSent;
}

uint32_t MacSystemInfo::GetNetworkInterfaceCount() const {
    return m_networkInterfaceCount;
}

uint64_t MacSystemInfo::GetTotalBytesReceived() const {
    return m_totalBytesReceived;
}

uint64_t MacSystemInfo::GetTotalBytesSent() const {
    return m_totalBytesSent;
}

double MacSystemInfo::GetNetworkUtilization() const {
    return m_networkUtilization;
}

bool MacSystemInfo::IsSystemHealthy() const {
    // 简单的健康检查逻辑
    double memoryUsage = GetMemoryUsagePercentage();
    double diskUsage = GetDiskUsagePercentage();
    double loadAvg = GetLoadAverage1Min();
    
    // 内存使用率超过90%认为不健康
    if (memoryUsage > 90.0) {
        return false;
    }
    
    // 磁盘使用率超过95%认为不健康
    if (diskUsage > 95.0) {
        return false;
    }
    
    // 负载平均值超过CPU核心数的2倍认为不健康
    uint32_t cores = std::thread::hardware_concurrency();
    if (cores > 0 && loadAvg > cores * 2.0) {
        return false;
    }
    
    return true;
}

std::string MacSystemInfo::GetSystemStatus() const {
    if (IsSystemHealthy()) {
        return "Healthy";
    } else {
        return "Warning";
    }
}

std::vector<std::string> MacSystemInfo::GetSystemWarnings() const {
    return m_systemWarnings;
}

std::vector<std::string> MacSystemInfo::GetSystemErrors() const {
    return m_systemErrors;
}

double MacSystemInfo::GetSystemHealthScore() const {
    if (m_systemHealthScore == 0.0) {
        // 简化健康评分计算
        double memoryScore = 100.0 - std::min(50.0, GetMemoryUsagePercentage() * 0.5);
        double diskScore = 100.0 - std::min(50.0, GetDiskUsagePercentage() * 0.5);
        double loadScore = 100.0 - std::min(50.0, GetLoadAverage1Min() * 10.0);
        m_systemHealthScore = (memoryScore + diskScore + loadScore) / 3.0;
    }
    return m_systemHealthScore;
}

bool MacSystemInfo::IsSecureBootEnabled() const {
    // macOS不使用传统安全启动，返回false
    return false;
}

bool MacSystemInfo::IsFirewallEnabled() const {
    // 简化实现
    return false;
}

bool MacSystemInfo::IsAntivirusRunning() const {
    // macOS不通常有传统杀毒软件
    return false;
}

std::string MacSystemInfo::GetSecurityStatus() const {
    return "Unknown";
}

std::string MacSystemInfo::GetMotherboardModel() const {
    if (m_motherboardModel.empty()) {
        // 简化实现
        m_motherboardModel = "Unknown";
    }
    return m_motherboardModel;
}

std::string MacSystemInfo::GetBIOSVersion() const {
    if (m_biosVersion.empty()) {
        // macOS使用固件版本
        size_t size = 0;
        if (sysctlbyname("kern.osversion", nullptr, &size, nullptr, 0) == 0 && size > 0) {
            std::vector<char> buffer(size);
            if (sysctlbyname("kern.osversion", buffer.data(), &size, nullptr, 0) == 0) {
                m_biosVersion = std::string(buffer.data(), size - 1);
            }
        }
    }
    return m_biosVersion;
}

std::string MacSystemInfo::GetFirmwareVersion() const {
    return GetBIOSVersion();
}

std::string MacSystemInfo::GetSerialNumber() const {
    if (m_serialNumber.empty()) {
        // 简化实现
        m_serialNumber = "Unknown";
    }
    return m_serialNumber;
}

bool MacSystemInfo::IsVirtualMachine() const {
    if (m_isVirtualMachine) {
        return m_isVirtualMachine;
    }
    
    // 简化检测
    size_t size = 0;
    if (sysctlbyname("kern.hv_support", nullptr, &size, nullptr, 0) == 0) {
        int hv_support = 0;
        if (sysctlbyname("kern.hv_support", nullptr, &size, &hv_support, size) == 0) {
            m_isVirtualMachine = (hv_support == 1);
        }
    }
    return m_isVirtualMachine;
}

std::string MacSystemInfo::GetVirtualizationPlatform() const {
    if (IsVirtualMachine()) {
        return "macOS Hypervisor";
    }
    return "None";
}

uint32_t MacSystemInfo::GetVirtualCPUCount() const {
    return m_virtualCPUCount;
}

uint64_t MacSystemInfo::GetVirtualMemory() const {
    return m_virtualMemory;
}

std::vector<std::string> MacSystemInfo::GetEnvironmentVariables() const {
    std::vector<std::string> envVars;
    // 简化实现，返回一些常见的环境变量
    extern char** environ;
    for (int i = 0; environ[i] != nullptr; ++i) {
        envVars.push_back(std::string(environ[i]));
        if (envVars.size() >= 20) break; // 限制数量
    }
    return envVars;
}

std::string MacSystemInfo::GetEnvironmentVariable(const std::string& name) const {
    const char* value = getenv(name.c_str());
    return value ? std::string(value) : "";
}

std::string MacSystemInfo::GetLastSystemUpdateTime() const {
    // 简化实现
    return "Unknown";
}

bool MacSystemInfo::UpdatesAvailable() const {
    // 简化实现
    return false;
}

std::vector<std::string> MacSystemInfo::GetPendingUpdates() const {
    return std::vector<std::string>();
}

std::string MacSystemInfo::GetLastError() const {
    return m_lastError;
}

void MacSystemInfo::ClearError() {
    ClearErrorInternal();
}

// 私有方法实现
bool MacSystemInfo::UpdateOSInfo() const {
    struct utsname uname_info;
    if (uname(&uname_info) != 0) {
        SetError("Failed to get system uname information");
        return false;
    }
    
    m_osName = uname_info.sysname;
    m_osVersion = uname_info.release;
    m_osBuild = uname_info.version;
    
    // 获取更详细的macOS版本信息
    size_t size = 0;
    if (sysctlbyname("kern.osproductversion", nullptr, &size, nullptr, 0) == 0 && size > 0) {
        std::vector<char> buffer(size);
        if (sysctlbyname("kern.osproductversion", buffer.data(), &size, nullptr, 0) == 0) {
            m_osVersion = std::string(buffer.data(), size - 1);
        }
    }
    
    return true;
}

bool MacSystemInfo::UpdateHostname() const {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        SetError("Failed to get hostname");
        return false;
    }
    m_hostname = hostname;
    return true;
}

bool MacSystemInfo::UpdateUsername() const {
    struct passwd* pw = getpwuid(getuid());
    if (pw == nullptr) {
        SetError("Failed to get username");
        return false;
    }
    m_username = pw->pw_name;
    return true;
}

bool MacSystemInfo::UpdateUptime() const {
    struct timeval boottime;
    size_t size = sizeof(boottime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    
    if (sysctl(mib, 2, &boottime, &size, nullptr, 0) != 0) {
        SetError("Failed to get boot time");
        return false;
    }
    
    auto now = std::chrono::system_clock::now();
    auto bootTime = std::chrono::system_clock::from_time_t(boottime.tv_sec);
    m_bootTime = bootTime;
    
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - bootTime);
    m_uptimeSeconds = duration.count();
    
    return true;
}

bool MacSystemInfo::GetLoadAverages() const {
    if (getloadavg(m_loadAverage, 3) == -1) {
        SetError("Failed to get load averages");
        return false;
    }
    return true;
}

bool MacSystemInfo::GetProcessInfo() const {
    // 获取进程信息
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t size = 0;
    
    if (sysctl(mib, 4, nullptr, &size, nullptr, 0) != 0) {
        SetError("Failed to get process info size");
        return false;
    }
    
    if (size == 0) {
        m_totalProcesses = 0;
        m_runningProcesses = 0;
        m_sleepingProcesses = 0;
        return true;
    }
    
    // 简化处理，使用系统调用获取进程数量
    FILE* fp = popen("ps -eo stat | wc -l", "r");
    if (fp) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), fp)) {
            m_totalProcesses = std::stoi(buffer) - 1; // 减去标题行
            m_processCount = m_totalProcesses;
        }
        pclose(fp);
    }
    
    // 获取线程数
    fp = popen("ps -Tm -o pid | wc -l", "r");
    if (fp) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), fp)) {
            m_threadCount = std::stoi(buffer) - 1; // 减去标题行
        }
        pclose(fp);
    }
    
    // 简化处理，估算运行和休眠进程
    m_runningProcesses = m_totalProcesses / 3; // 简化估算
    m_sleepingProcesses = m_totalProcesses - m_runningProcesses;
    m_runningProcessCount = m_runningProcesses;
    m_sleepingProcessCount = m_sleepingProcesses;
    
    return true;
}

bool MacSystemInfo::GetMemoryInfo() const {
    size_t size = sizeof(m_totalPhysicalMemory);
    if (sysctlbyname("hw.memsize", nullptr, &size, nullptr, 0) != 0 ||
        sysctlbyname("hw.memsize", nullptr, &size, &m_totalPhysicalMemory, size) != 0) {
        SetError("Failed to get total physical memory");
        return false;
    }
    
    // 获取可用内存
    vm_statistics64_data_t vm_stat;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    kern_return_t kr = host_statistics64(mach_host_self(), HOST_VM_INFO64,
                                           (host_info64_t)&vm_stat, &count);
    
    if (kr != KERN_SUCCESS) {
        SetError("Failed to get virtual memory statistics");
        return false;
    }
    
    uint64_t free_memory = vm_stat.free_count * vm_page_size;
    uint64_t inactive_memory = vm_stat.inactive_count * vm_page_size;
    uint64_t active_memory = vm_stat.active_count * vm_page_size;
    uint64_t wired_memory = vm_stat.wire_count * vm_page_size;
    uint64_t cache_memory = vm_stat.inactive_count * vm_page_size;
    
    m_availablePhysicalMemory = free_memory + inactive_memory;
    m_usedPhysicalMemory = m_totalPhysicalMemory - m_availablePhysicalMemory;
    
    // 设置统一的内存信息
    m_totalMemory = m_totalPhysicalMemory;
    m_availableMemory = m_availablePhysicalMemory;
    m_usedMemory = m_usedPhysicalMemory;
    m_cacheMemory = cache_memory;
    
    // 计算内存压力
    double memoryUsage = (double)m_usedPhysicalMemory / m_totalPhysicalMemory;
    m_memoryPressure = memoryUsage * 100.0;
    
    return true;
}

bool MacSystemInfo::GetDiskInfo() const {
    struct statvfs vfs;
    if (statvfs("/", &vfs) != 0) {
        SetError("Failed to get disk statistics");
        return false;
    }
    
    m_totalDiskSpace = vfs.f_blocks * vfs.f_frsize;
    m_availableDiskSpace = vfs.f_bavail * vfs.f_frsize;
    m_usedDiskSpace = m_totalDiskSpace - m_availableDiskSpace;
    
    return true;
}

bool MacSystemInfo::GetNetworkInfo() const {
    struct ifaddrs* ifaddrs_ptr = nullptr;
    if (getifaddrs(&ifaddrs_ptr) != 0) {
        SetError("Failed to get network interface information");
        return false;
    }
    
    // 计算网络接口数量
    m_networkInterfaceCount = 0;
    for (struct ifaddrs* ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)) {
            m_networkInterfaceCount++;
        }
    }
    
    // 查找主网络接口
    m_primaryInterface = "en0"; // 默认使用en0
    
    // 简化处理，获取网络统计信息
    FILE* fp = popen("netstat -ib | grep en0 | head -1", "r");
    if (fp) {
        char buffer[1024];
        if (fgets(buffer, sizeof(buffer), fp)) {
            std::string line(buffer);
            // 简化解析网络统计信息
            // 这里应该解析Ibytes和Obytes等字段
        }
        pclose(fp);
    }
    
    // 设置统一的网络信息
    m_totalBytesReceived = m_networkBytesReceived;
    m_totalBytesSent = m_networkBytesSent;
    m_networkUtilization = 0.0; // 简化实现
    
    freeifaddrs(ifaddrs_ptr);
    return true;
}

void MacSystemInfo::ClearErrorInternal() const {
    m_lastError.clear();
}

void MacSystemInfo::SetError(const std::string& error) const {
    m_lastError = error;
}

uint64_t MacSystemInfo::GetLastUpdateTime() const {
    return m_lastUpdateTime;
}

#endif // PLATFORM_MACOS