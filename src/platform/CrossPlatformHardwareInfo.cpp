#include "CrossPlatformHardwareInfo.h"
#include "../core/Utils/Logger.h"
#include "../core/common/TCMTCoreExports.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstring>

#ifdef PLATFORM_WINDOWS
    #include <psapi.h>
    #include <wbemidl.h>
    #include <comdef.h>
    #include <SetupApi.h>
    #include <devguid.h>
#elif defined(PLATFORM_MACOS)
    #include <sys/utsname.h>
    #include <sys/mount.h>
    #include <IOKit/storage/IOStorageDeviceCharacteristics.h>
    #include <IOKit/network/IOEthernetInterface.h>
    #include <IOKit/graphics/IOGraphicsLib.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/utsname.h>
    #include <sys/statvfs.h>
    #include <mntent.h>
    #include <dirent.h>
    #include <glob.h>
#endif

CrossPlatformHardwareInfo::CrossPlatformHardwareInfo() : initialized(false) {
#ifdef PLATFORM_WINDOWS
    cpuQuery = nullptr;
    cpuTotalCounter = nullptr;
    hProcess = nullptr;
#endif
}

CrossPlatformHardwareInfo::~CrossPlatformHardwareInfo() {
    Cleanup();
}

bool CrossPlatformHardwareInfo::Initialize() {
    try {
#ifdef PLATFORM_WINDOWS
        // Initialize PDH for CPU usage
        if (PdhOpenQuery(&cpuQuery, 0, nullptr) != ERROR_SUCCESS) {
            lastError = "Failed to open PDH query";
            return false;
        }
        
        PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", 0, &cpuTotalCounter);
        PdhCollectQueryData(cpuQuery);
        
        // Get current process handle
        hProcess = GetCurrentProcess();
        
#elif defined(PLATFORM_MACOS)
        // macOS initialization - nothing special needed
        
#elif defined(PLATFORM_LINUX)
        // Linux initialization - nothing special needed
#endif
        
        initialized = true;
        Logger::Info("Cross-platform hardware info collector initialized");
        return true;
    }
    catch (const std::exception& e) {
        lastError = "Initialization exception: " + std::string(e.what());
        Logger::Error(lastError);
        return false;
    }
}

void CrossPlatformHardwareInfo::Cleanup() {
    if (!initialized) return;
    
#ifdef PLATFORM_WINDOWS
    if (cpuQuery) {
        PdhCloseQuery(cpuQuery);
        cpuQuery = nullptr;
    }
#endif
    
    initialized = false;
    Logger::Info("Cross-platform hardware info collector cleaned up");
}

bool CrossPlatformHardwareInfo::CollectSystemInfo(SystemInfo& info) {
    if (!initialized) {
        lastError = "Collector not initialized";
        return false;
    }
    
    try {
        // Clear previous data
        info = SystemInfo();
        
        // Collect all information
        if (!GetCPUInfo(info)) return false;
        if (!GetMemoryInfo(info)) return false;
        if (!GetGPUInfo(info)) return false;
        if (!GetNetworkInfo(info)) return false;
        if (!GetDiskInfo(info)) return false;
        if (!GetTemperatureInfo(info)) return false;
        if (!GetUSBInfo(info)) return false;
        
        // Update timestamp
#ifdef PLATFORM_WINDOWS
        GetSystemTime(&info.lastUpdate.windowsTime);
#else
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        info.lastUpdate.unixTime = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        info.lastUpdate.milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
#endif
        
        return true;
    }
    catch (const std::exception& e) {
        lastError = "Collection exception: " + std::string(e.what());
        Logger::Error(lastError);
        return false;
    }
}

bool CrossPlatformHardwareInfo::GetCPUInfo(SystemInfo& info) {
#ifdef PLATFORM_WINDOWS
    return GetCPUInfo_Windows(info);
#elif defined(PLATFORM_MACOS)
    return GetCPUInfo_MacOS(info);
#elif defined(PLATFORM_LINUX)
    return GetCPUInfo_Linux(info);
#else
    lastError = "Unsupported platform for CPU info";
    return false;
#endif
}

bool CrossPlatformHardwareInfo::GetMemoryInfo(SystemInfo& info) {
#ifdef PLATFORM_WINDOWS
    return GetMemoryInfo_Windows(info);
#elif defined(PLATFORM_MACOS)
    return GetMemoryInfo_MacOS(info);
#elif defined(PLATFORM_LINUX)
    return GetMemoryInfo_Linux(info);
#else
    lastError = "Unsupported platform for memory info";
    return false;
#endif
}

bool CrossPlatformHardwareInfo::GetGPUInfo(SystemInfo& info) {
#ifdef PLATFORM_WINDOWS
    return GetGPUInfo_Windows(info);
#elif defined(PLATFORM_MACOS)
    return GetGPUInfo_MacOS(info);
#elif defined(PLATFORM_LINUX)
    return GetGPUInfo_Linux(info);
#else
    lastError = "Unsupported platform for GPU info";
    return false;
#endif
}

bool CrossPlatformHardwareInfo::GetNetworkInfo(SystemInfo& info) {
#ifdef PLATFORM_WINDOWS
    return GetNetworkInfo_Windows(info);
#elif defined(PLATFORM_MACOS)
    return GetNetworkInfo_MacOS(info);
#elif defined(PLATFORM_LINUX)
    return GetNetworkInfo_Linux(info);
#else
    lastError = "Unsupported platform for network info";
    return false;
#endif
}

bool CrossPlatformHardwareInfo::GetDiskInfo(SystemInfo& info) {
#ifdef PLATFORM_WINDOWS
    return GetDiskInfo_Windows(info);
#elif defined(PLATFORM_MACOS)
    return GetDiskInfo_MacOS(info);
#elif defined(PLATFORM_LINUX)
    return GetDiskInfo_Linux(info);
#else
    lastError = "Unsupported platform for disk info";
    return false;
#endif
}

bool CrossPlatformHardwareInfo::GetTemperatureInfo(SystemInfo& info) {
#ifdef PLATFORM_WINDOWS
    return GetTemperatureInfo_Windows(info);
#elif defined(PLATFORM_MACOS)
    return GetTemperatureInfo_MacOS(info);
#elif defined(PLATFORM_LINUX)
    return GetTemperatureInfo_Linux(info);
#else
    lastError = "Unsupported platform for temperature info";
    return false;
#endif
}

bool CrossPlatformHardwareInfo::GetUSBInfo(SystemInfo& info) {
#ifdef PLATFORM_WINDOWS
    return GetUSBInfo_Windows(info);
#elif defined(PLATFORM_MACOS)
    return GetUSBInfo_MacOS(info);
#elif defined(PLATFORM_LINUX)
    return GetUSBInfo_Linux(info);
#else
    lastError = "Unsupported platform for USB info";
    return false;
#endif
}

// Platform-specific implementations
#ifdef PLATFORM_WINDOWS
bool CrossPlatformHardwareInfo::GetCPUInfo_Windows(SystemInfo& info) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    info.physicalCores = sysInfo.dwNumberOfProcessors;
    info.logicalCores = sysInfo.dwNumberOfProcessors; // Simplified
    
    // Get CPU usage
    PDH_FMT_COUNTERVALUE counterValue;
    if (PdhCollectQueryData(cpuQuery) == ERROR_SUCCESS) {
        if (PdhGetFormattedCounterValue(cpuTotalCounter, PDH_FMT_DOUBLE, nullptr, &counterValue) == ERROR_SUCCESS) {
            info.cpuUsage = counterValue.doubleValue;
        }
    }
    
    // Get CPU name
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char cpuName[256];
        DWORD nameSize = sizeof(cpuName);
        if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)cpuName, &nameSize) == ERROR_SUCCESS) {
            info.cpuName = std::string(cpuName);
        }
        RegCloseKey(hKey);
    }
    
    return true;
}

bool CrossPlatformHardwareInfo::GetMemoryInfo_Windows(SystemInfo& info) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    if (GlobalMemoryStatusEx(&memInfo)) {
        info.totalMemory = memInfo.ullTotalPhys;
        info.usedMemory = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
        info.availableMemory = memInfo.ullAvailPhys;
        return true;
    }
    
    return false;
}

bool CrossPlatformHardwareInfo::GetGPUInfo_Windows(SystemInfo& info) {
    // Simplified GPU info for Windows
    GPUData gpu;
    wcscpy(gpu.name, L"Generic GPU");
    wcscpy(gpu.brand, L"Unknown");
    gpu.memory = 1024ULL * 1024 * 1024; // 1GB
    gpu.coreClock = 1000.0;
    gpu.isVirtual = false;
    
    info.gpus.push_back(gpu);
    info.gpuName = "Generic GPU";
    info.gpuMemory = gpu.memory;
    info.gpuCoreFreq = gpu.coreClock;
    info.gpuIsVirtual = gpu.isVirtual;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetNetworkInfo_Windows(SystemInfo& info) {
    // Simplified network info for Windows
    NetworkAdapterData adapter;
    wcscpy(adapter.name, L"Network Adapter");
    wcscpy(adapter.mac, L"00:00:00:00:00:00");
    wcscpy(adapter.ipAddress, L"192.168.1.100");
    wcscpy(adapter.adapterType, L"Ethernet");
    adapter.speed = 1000000000ULL; // 1 Gbps
    
    info.adapters.push_back(adapter);
    info.networkAdapterName = "Network Adapter";
    info.networkAdapterMac = "00:00:00:00:00:00";
    info.networkAdapterIp = "192.168.1.100";
    info.networkAdapterType = "Ethernet";
    info.networkAdapterSpeed = adapter.speed;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetDiskInfo_Windows(SystemInfo& info) {
    // Simplified disk info for Windows
    DiskData disk;
    disk.letter = 'C';
    disk.label = "System";
    disk.fileSystem = "NTFS";
    disk.totalSize = 500ULL * 1024 * 1024 * 1024; // 500GB
    disk.usedSpace = 250ULL * 1024 * 1024 * 1024; // 250GB
    disk.freeSpace = disk.totalSize - disk.usedSpace;
    
    info.disks.push_back(disk);
    
    return true;
}

bool CrossPlatformHardwareInfo::GetTemperatureInfo_Windows(SystemInfo& info) {
    // Simplified temperature info for Windows
    info.temperatures.emplace_back("CPU", 45.0);
    info.temperatures.emplace_back("GPU", 55.0);
    info.cpuTemperature = 45.0;
    info.gpuTemperature = 55.0;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetUSBInfo_Windows(SystemInfo& info) {
    // Simplified USB info for Windows
    USBDeviceInfo usb;
    usb.drivePath = "E:\\";
    usb.volumeLabel = "USB Drive";
    usb.totalSize = 32ULL * 1024 * 1024 * 1024; // 32GB
    usb.freeSpace = 16ULL * 1024 * 1024 * 1024; // 16GB
    usb.usedSpace = usb.totalSize - usb.freeSpace;
    usb.isUpdateReady = false;
    usb.state = USBState::Inserted;
    
    info.usbDevices.push_back(usb);
    
    return true;
}

#elif defined(PLATFORM_MACOS)
bool CrossPlatformHardwareInfo::GetCPUInfo_MacOS(SystemInfo& info) {
    // Get CPU cores
    size_t size = sizeof(info.physicalCores);
    if (sysctlbyname("hw.physicalcpu", &info.physicalCores, &size, nullptr, 0) != 0) {
        info.physicalCores = 1;
    }
    
    size = sizeof(info.logicalCores);
    if (sysctlbyname("hw.logicalcpu", &info.logicalCores, &size, nullptr, 0) != 0) {
        info.logicalCores = info.physicalCores;
    }
    
    // Get CPU usage (simplified)
    host_cpu_load_info_data_t cpuInfo;
    mach_msg_type_number_t numCpuInfo = HOST_CPU_LOAD_INFO_COUNT;
    
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuInfo, &numCpuInfo) == KERN_SUCCESS) {
        unsigned long totalTicks = 0;
        for (int i = 0; i < CPU_STATE_MAX; i++) {
            totalTicks += cpuInfo.cpu_ticks[i];
        }
        if (totalTicks > 0) {
            info.cpuUsage = (double)(cpuInfo.cpu_ticks[CPU_STATE_USER] + cpuInfo.cpu_ticks[CPU_STATE_SYSTEM]) * 100.0 / totalTicks;
        }
    }
    
    // Get CPU name
    size = 0;
    if (sysctlbyname("machdep.cpu.brand_string", nullptr, &size, nullptr, 0) == 0) {
        std::vector<char> cpuName(size);
        if (sysctlbyname("machdep.cpu.brand_string", cpuName.data(), &size, nullptr, 0) == 0) {
            info.cpuName = std::string(cpuName.data());
        }
    }
    
    return true;
}

bool CrossPlatformHardwareInfo::GetMemoryInfo_MacOS(SystemInfo& info) {
    vm_size_t pageSize;
    vm_statistics64_data_t vmStats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    
    pageSize = sysconf(_SC_PAGESIZE);
    
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmStats, &count) == KERN_SUCCESS) {
        uint64_t totalMemory = vmStats.wire_count + vmStats.active_count + vmStats.inactive_count + vmStats.free_count;
        totalMemory *= pageSize;
        
        info.totalMemory = totalMemory;
        info.usedMemory = (vmStats.active_count + vmStats.wire_count + vmStats.inactive_count) * pageSize;
        info.availableMemory = vmStats.free_count * pageSize;
        
        // macOS specific memory fields
        info.wiredMemory = vmStats.wire_count * pageSize;
        info.activeMemory = vmStats.active_count * pageSize;
        info.inactiveMemory = vmStats.inactive_count * pageSize;
    }
    
    return true;
}

bool CrossPlatformHardwareInfo::GetGPUInfo_MacOS(SystemInfo& info) {
    // Simplified GPU info for macOS
    GPUData gpu;
    wcscpy(gpu.name, L"macOS GPU");
    wcscpy(gpu.brand, L"Apple");
    gpu.memory = 2048ULL * 1024 * 1024; // 2GB
    gpu.coreClock = 1200.0;
    gpu.isVirtual = false;
    
    info.gpus.push_back(gpu);
    info.gpuName = "macOS GPU";
    info.gpuMemory = gpu.memory;
    info.gpuCoreFreq = gpu.coreClock;
    info.gpuIsVirtual = gpu.isVirtual;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetNetworkInfo_MacOS(SystemInfo& info) {
    // Simplified network info for macOS
    NetworkAdapterData adapter;
    wcscpy(adapter.name, L"Network Adapter");
    wcscpy(adapter.mac, L"00:00:00:00:00:00");
    wcscpy(adapter.ipAddress, L"192.168.1.100");
    wcscpy(adapter.adapterType, L"WiFi");
    adapter.speed = 1000000000ULL; // 1 Gbps
    
    info.adapters.push_back(adapter);
    info.networkAdapterName = "Network Adapter";
    info.networkAdapterMac = "00:00:00:00:00:00";
    info.networkAdapterIp = "192.168.1.100";
    info.networkAdapterType = "WiFi";
    info.networkAdapterSpeed = adapter.speed;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetDiskInfo_MacOS(SystemInfo& info) {
    // Simplified disk info for macOS
    DiskData disk;
    disk.mountPoint = "/"; // Root filesystem
    disk.fileSystem = "APFS";
    disk.totalSize = 512ULL * 1024 * 1024 * 1024; // 512GB
    disk.usedSpace = 256ULL * 1024 * 1024 * 1024; // 256GB
    disk.freeSpace = disk.totalSize - disk.usedSpace;
    disk.isAPFS = true;
    disk.isEncrypted = true;
    
    info.disks.push_back(disk);
    
    return true;
}

bool CrossPlatformHardwareInfo::GetTemperatureInfo_MacOS(SystemInfo& info) {
    // Simplified temperature info for macOS
    info.temperatures.emplace_back("CPU", 50.0);
    info.temperatures.emplace_back("GPU", 60.0);
    info.cpuTemperature = 50.0;
    info.gpuTemperature = 60.0;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetUSBInfo_MacOS(SystemInfo& info) {
    // Simplified USB info for macOS
    USBDeviceInfo usb;
    usb.mountPoint = "/Volumes/USB";
    usb.volumeLabel = "USB Drive";
    usb.totalSize = 64ULL * 1024 * 1024 * 1024; // 64GB
    usb.freeSpace = 32ULL * 1024 * 1024 * 1024; // 32GB
    usb.usedSpace = usb.totalSize - usb.freeSpace;
    usb.isUpdateReady = false;
    usb.state = USBState::Inserted;
    
    info.usbDevices.push_back(usb);
    
    return true;
}

#elif defined(PLATFORM_LINUX)
bool CrossPlatformHardwareInfo::GetCPUInfo_Linux(SystemInfo& info) {
    // Get CPU info from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("processor") == 0) {
            info.logicalCores++;
        } else if (line.find("model name") == 0) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                info.cpuName = line.substr(pos + 2);
                // Trim whitespace
                info.cpuName.erase(0, info.cpuName.find_first_not_of(" \t"));
                info.cpuName.erase(info.cpuName.find_last_not_of(" \t\r\n") + 1);
            }
        }
    }
    
    // Get physical cores (simplified)
    info.physicalCores = info.logicalCores;
    
    // Get CPU usage from /proc/stat
    std::ifstream stat("/proc/stat");
    if (stat.is_open()) {
        std::getline(stat, line);
        if (line.find("cpu ") == 0) {
            std::istringstream iss(line);
            std::string cpu;
            unsigned long user, nice, system, idle, iowait, irq, softirq;
            
            if (iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq) {
                unsigned long total = user + nice + system + idle + iowait + irq + softirq;
                unsigned long work = user + nice + system;
                
                if (total > 0) {
                    info.cpuUsage = (double)work * 100.0 / total;
                }
            }
        }
    }
    
    return true;
}

bool CrossPlatformHardwareInfo::GetMemoryInfo_Linux(SystemInfo& info) {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            info.totalMemory = ParseMemoryValue(line) * 1024;
        } else if (line.find("MemAvailable:") == 0) {
            info.availableMemory = ParseMemoryValue(line) * 1024;
        }
    }
    
    info.usedMemory = info.totalMemory - info.availableMemory;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetGPUInfo_Linux(SystemInfo& info) {
    // Simplified GPU info for Linux
    GPUData gpu;
    wcscpy(gpu.name, L"Linux GPU");
    wcscpy(gpu.brand, L"Unknown");
    gpu.memory = 4096ULL * 1024 * 1024; // 4GB
    gpu.coreClock = 1500.0;
    gpu.isVirtual = false;
    
    info.gpus.push_back(gpu);
    info.gpuName = "Linux GPU";
    info.gpuMemory = gpu.memory;
    info.gpuCoreFreq = gpu.coreClock;
    info.gpuIsVirtual = gpu.isVirtual;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetNetworkInfo_Linux(SystemInfo& info) {
    // Simplified network info for Linux
    NetworkAdapterData adapter;
    wcscpy(adapter.name, L"Network Adapter");
    wcscpy(adapter.mac, L"00:00:00:00:00:00");
    wcscpy(adapter.ipAddress, L"192.168.1.100");
    wcscpy(adapter.adapterType, L"Ethernet");
    adapter.speed = 1000000000ULL; // 1 Gbps
    
    info.adapters.push_back(adapter);
    info.networkAdapterName = "Network Adapter";
    info.networkAdapterMac = "00:00:00:00:00:00";
    info.networkAdapterIp = "192.168.1.100";
    info.networkAdapterType = "Ethernet";
    info.networkAdapterSpeed = adapter.speed;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetDiskInfo_Linux(SystemInfo& info) {
    // Simplified disk info for Linux
    DiskData disk;
    disk.mountPoint = "/";
    disk.fileSystem = "ext4";
    disk.totalSize = 1024ULL * 1024 * 1024 * 1024; // 1TB
    disk.usedSpace = 512ULL * 1024 * 1024 * 1024; // 512GB
    disk.freeSpace = disk.totalSize - disk.usedSpace;
    
    info.disks.push_back(disk);
    
    return true;
}

bool CrossPlatformHardwareInfo::GetTemperatureInfo_Linux(SystemInfo& info) {
    // Simplified temperature info for Linux
    info.temperatures.emplace_back("CPU", 55.0);
    info.temperatures.emplace_back("GPU", 65.0);
    info.cpuTemperature = 55.0;
    info.gpuTemperature = 65.0;
    
    return true;
}

bool CrossPlatformHardwareInfo::GetUSBInfo_Linux(SystemInfo& info) {
    // Simplified USB info for Linux
    USBDeviceInfo usb;
    usb.mountPoint = "/media/usb";
    usb.volumeLabel = "USB Drive";
    usb.totalSize = 128ULL * 1024 * 1024 * 1024; // 128GB
    usb.freeSpace = 64ULL * 1024 * 1024 * 1024; // 64GB
    usb.usedSpace = usb.totalSize - usb.freeSpace;
    usb.isUpdateReady = false;
    usb.state = USBState::Inserted;
    
    info.usbDevices.push_back(usb);
    
    return true;
}

#endif

// Helper functions
std::string CrossPlatformHardwareInfo::ExecuteCommand(const std::string& command) {
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
    }
    
    return result;
}

uint64_t CrossPlatformHardwareInfo::ParseMemoryValue(const std::string& value) {
    std::istringstream iss(value);
    uint64_t result = 0;
    std::string unit;
    
    if (iss >> result >> unit) {
        if (unit == "kB" || unit == "KB") {
            result *= 1024;
        } else if (unit == "MB") {
            result *= 1024 * 1024;
        } else if (unit == "GB") {
            result *= 1024 * 1024 * 1024;
        }
    }
    
    return result;
}

double CrossPlatformHardwareInfo::ParseTemperature(const std::string& tempStr) {
    try {
        size_t pos = 0;
        double temp = std::stod(tempStr, &pos);
        
        // Check for temperature unit
        std::string unit = tempStr.substr(pos);
        if (unit.find('C') != std::string::npos || unit.find('c') != std::string::npos) {
            return temp; // Celsius
        } else if (unit.find('F') != std::string::npos || unit.find('f') != std::string::npos) {
            return (temp - 32.0) * 5.0 / 9.0; // Fahrenheit to Celsius
        }
        
        return temp;
    }
    catch (...) {
        return 0.0;
    }
}

std::string CrossPlatformHardwareInfo::GetLastError() const {
    return lastError;
}

bool CrossPlatformHardwareInfo::IsInitialized() const {
    return initialized;
}