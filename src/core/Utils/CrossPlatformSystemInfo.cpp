#include "CrossPlatformSystemInfo.h"
#include "Logger.h"
#include <sstream>
#include <algorithm>
#include <cstring>

CrossPlatformSystemInfo& CrossPlatformSystemInfo::GetInstance() {
    static CrossPlatformSystemInfo instance;
    return instance;
}

CrossPlatformSystemInfo::CrossPlatformSystemInfo()
    : m_initialized(false)
#ifdef PLATFORM_WINDOWS
    , m_pLoc(nullptr)
    , m_pSvc(nullptr)
    , m_pEnumerator(nullptr)
#elif defined(__APPLE__)
    , m_serviceIterator(0)
#endif
{
}

CrossPlatformSystemInfo::~CrossPlatformSystemInfo() {
    Cleanup();
}

bool CrossPlatformSystemInfo::Initialize() {
    if (m_initialized) {
        return true;
    }
    
#ifdef PLATFORM_WINDOWS
    return InitializeWMI();
#elif defined(__APPLE__)
    return InitializeIOKit();
#elif defined(__linux__)
    // Linux 不需要特殊初始化
    m_initialized = true;
    return true;
#else
    Logger::Warning("CrossPlatformSystemInfo: Platform not supported");
    return false;
#endif
}

void CrossPlatformSystemInfo::Cleanup() {
    if (!m_initialized) {
        return;
    }
    
#ifdef PLATFORM_WINDOWS
    CleanupWMI();
#elif defined(__APPLE__)
    CleanupIOKit();
#endif
    
    m_initialized = false;
}

// Windows 实现
#ifdef PLATFORM_WINDOWS
bool CrossPlatformSystemInfo::InitializeWMI() {
    HRESULT hres;
    
    // 初始化 COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        Logger::Error("CrossPlatformSystemInfo: Failed to initialize COM library");
        return false;
    }
    
    // 设置 COM 安全级别
    hres = CoInitializeSecurity(
        nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE, nullptr
    );
    
    if (FAILED(hres) && hres != RPC_E_TOO_LATE) {
        Logger::Error("CrossPlatformSystemInfo: Failed to initialize security");
        CoUninitialize();
        return false;
    }
    
    // 获取 WMI 定位器
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&m_pLoc);
    if (FAILED(hres)) {
        Logger::Error("CrossPlatformSystemInfo: Failed to create IWbemLocator object");
        CoUninitialize();
        return false;
    }
    
    // 连接到 WMI
    hres = m_pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, nullptr, 0, 0, &m_pSvc
    );
    if (FAILED(hres)) {
        Logger::Error("CrossPlatformSystemInfo: Could not connect to WMI");
        m_pLoc->Release();
        CoUninitialize();
        return false;
    }
    
    // 设置安全级别
    hres = CoSetProxyBlanket(
        m_pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE
    );
    if (FAILED(hres)) {
        Logger::Error("CrossPlatformSystemInfo: Could not set proxy blanket");
        m_pSvc->Release();
        m_pLoc->Release();
        CoUninitialize();
        return false;
    }
    
    m_initialized = true;
    return true;
}

void CrossPlatformSystemInfo::CleanupWMI() {
    if (m_pSvc) {
        m_pSvc->Release();
        m_pSvc = nullptr;
    }
    if (m_pLoc) {
        m_pLoc->Release();
        m_pLoc = nullptr;
    }
    if (m_pEnumerator) {
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
    }
    CoUninitialize();
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::ExecuteWMIQuery(const std::wstring& query) {
    std::vector<SystemDeviceInfo> results;
    
    if (!m_initialized || !m_pSvc) {
        return results;
    }
    
    HRESULT hres = m_pSvc->ExecQuery(
        bstr_t("WQL"), bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &m_pEnumerator
    );
    
    if (FAILED(hres)) {
        Logger::Error("CrossPlatformSystemInfo: WMI query failed");
        return results;
    }
    
    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;
    
    while (m_pEnumerator) {
        HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }
        
        SystemDeviceInfo device;
        VARIANT vtProp;
        
        // 获取所有属性
        hr = pclsObj->BeginEnumeration(0);
        if (SUCCEEDED(hr)) {
            BSTR className;
            hr = pclsObj->GetObjectText(0, &className);
            if (SUCCEEDED(hr)) {
                // 解析 WMI 对象文本以提取属性
                std::wstring objText(className);
                // 这里可以添加更详细的属性解析逻辑
                
                device.description = std::string(objText.begin(), objText.end());
                SysFreeString(className);
            }
        }
        
        VariantClear(&vtProp);
        pclsObj->Release();
        results.push_back(device);
    }
    
    return results;
}

// macOS 实现
#elif defined(__APPLE__)
bool CrossPlatformSystemInfo::InitializeIOKit() {
    // 创建匹配所有服务的字典
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOServiceClass);
    if (!matchingDict) {
        Logger::Error("CrossPlatformSystemInfo: Could not create matching dictionary");
        return false;
    }
    
    // 获取服务迭代器
    kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &m_serviceIterator);
    if (kr != KERN_SUCCESS) {
        Logger::Error("CrossPlatformSystemInfo: Could not get matching services");
        return false;
    }
    
    m_initialized = true;
    return true;
}

void CrossPlatformSystemInfo::CleanupIOKit() {
    if (m_serviceIterator) {
        IOObjectRelease(m_serviceIterator);
        m_serviceIterator = 0;
    }
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::QueryIOKitDevices(const io_name_t className) {
    std::vector<SystemDeviceInfo> results;
    
    CFMutableDictionaryRef matchingDict = IOServiceMatching(className);
    if (!matchingDict) {
        return results;
    }
    
    io_iterator_t iterator;
    kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iterator);
    if (kr != KERN_SUCCESS) {
        return results;
    }
    
    io_registry_entry_t service;
    while ((service = IOIteratorNext(iterator)) != 0) {
        SystemDeviceInfo device;
        
        // 获取设备名称
        io_string_t path;
        kr = IORegistryEntryGetPath(service, kIOServicePlane, path);
        if (kr == KERN_SUCCESS) {
            device.name = path;
        }
        
        // 获取设备属性
        CFMutableDictionaryRef properties;
        kr = IORegistryEntryCreateCFProperties(service, &properties, kCFAllocatorDefault, kNilOptions);
        if (kr == KERN_SUCCESS) {
            // 提取常用属性
            CFStringRef nameRef = (CFStringRef)CFDictionaryGetValue(properties, CFSTR(kIOPropertyNameKey));
            if (nameRef) {
                device.name = CFStringToString(nameRef);
            }
            
            CFStringRef vendorRef = (CFStringRef)CFDictionaryGetValue(properties, CFSTR(kIOPropertyVendorNameKey));
            if (vendorRef) {
                device.vendor = CFStringToString(vendorRef);
            }
            
            CFStringRef modelRef = (CFStringRef)CFDictionaryGetValue(properties, CFSTR(kIOPropertyModelKey));
            if (modelRef) {
                device.description = CFStringToString(modelRef);
            }
            
            CFRelease(properties);
        }
        
        IOObjectRelease(service);
        results.push_back(device);
    }
    
    IOObjectRelease(iterator);
    return results;
}

std::string CrossPlatformSystemInfo::CFStringToString(CFStringRef cfString) {
    if (!cfString) {
        return "";
    }
    
    CFIndex length = CFStringGetLength(cfString);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    
    std::vector<char> buffer(maxSize);
    if (CFStringGetCString(cfString, buffer.data(), maxSize, kCFStringEncodingUTF8)) {
        return std::string(buffer.data());
    }
    
    return "";
}

// Linux 实现
#elif defined(__linux__)
std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::QueryProcDevices(const std::string& procPath) {
    std::vector<SystemDeviceInfo> results;
    
    std::ifstream file(procPath);
    if (!file.is_open()) {
        return results;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        SystemDeviceInfo device;
        std::map<std::string, std::string> props = ParseProcLine(line);
        
        auto nameIt = props.find("name");
        if (nameIt != props.end()) {
            device.name = nameIt->second;
        }
        
        device.properties = props;
        results.push_back(device);
    }
    
    return results;
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::QuerySysfsDevices(const std::string& sysfsPath) {
    std::vector<SystemDeviceInfo> results;
    
    DIR* dir = opendir(sysfsPath.c_str());
    if (!dir) {
        return results;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR || entry->d_type == DT_LNK) {
            std::string devicePath = sysfsPath + "/" + entry->d_name;
            
            SystemDeviceInfo device;
            device.name = entry->d_name;
            
            // 读取设备属性
            device.properties["vendor"] = ReadSysfsProperty(devicePath + "/vendor");
            device.properties["model"] = ReadSysfsProperty(devicePath + "/model");
            device.properties["device"] = ReadSysfsProperty(devicePath + "/device");
            
            results.push_back(device);
        }
    }
    
    closedir(dir);
    return results;
}

std::string CrossPlatformSystemInfo::ReadSysfsProperty(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content;
    std::getline(file, content);
    return TrimString(content);
}

std::map<std::string, std::string> CrossPlatformSystemInfo::ParseProcLine(const std::string& line) {
    std::map<std::string, std::string> result;
    
    std::istringstream iss(line);
    std::string token;
    int field = 0;
    
    while (iss >> token) {
        switch (field) {
            case 0: result["interface"] = token; break;
            case 1: result["bytes_received"] = token; break;
            case 9: result["bytes_sent"] = token; break;
            // 可以添加更多字段解析
        }
        field++;
    }
    
    return result;
}

#endif

// 通用实现
std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::QueryDevices(const std::string& deviceClass) {
    if (!Initialize()) {
        return {};
    }
    
#ifdef PLATFORM_WINDOWS
    std::wstring query = L"SELECT * FROM " + std::wstring(deviceClass.begin(), deviceClass.end());
    return ExecuteWMIQuery(query);
#elif defined(__APPLE__)
    if (deviceClass == "GPU") {
        return QueryIOKitDevices(kIOAcceleratorClassName);
    } else if (deviceClass == "Network") {
        return QueryIOKitDevices(kIONetworkInterfaceClass);
    } else if (deviceClass == "Storage") {
        return QueryIOKitDevices(kIOBlockStorageDeviceClass);
    } else if (deviceClass == "USB") {
        return QueryIOKitDevices(kIOUSBDeviceClassName);
    }
    return {};
#elif defined(__linux__)
    if (deviceClass == "Network") {
        return QueryProcDevices("/proc/net/dev");
    } else if (deviceClass.find("Storage") != std::string::npos) {
        return QuerySysfsDevices("/sys/block");
    } else if (deviceClass.find("USB") != std::string::npos) {
        return QuerySysfsDevices("/sys/bus/usb/devices");
    }
    return {};
#else
    return {};
#endif
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::GetGpuDevices() {
    return QueryDevices("GPU");
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::GetNetworkAdapters() {
    return QueryDevices("Network");
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::GetStorageDevices() {
    return QueryDevices("Storage");
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::GetUsbDevices() {
    return QueryDevices("USB");
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::GetTemperatureSensors() {
    return QueryDevices("Temperature");
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::GetMemoryModules() {
    return QueryDevices("Memory");
}

std::vector<SystemDeviceInfo> CrossPlatformSystemInfo::GetCpuInfo() {
    return QueryDevices("CPU");
}

CrossPlatformSystemInfo::PerformanceInfo CrossPlatformSystemInfo::GetPerformanceInfo() {
    PerformanceInfo info = {};
    
#ifdef PLATFORM_WINDOWS
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    info.totalMemory = memInfo.ullTotalPhys;
    info.availableMemory = memInfo.ullAvailPhys;
    
    // CPU 使用率可以通过 PDH 或 WMI 获取
    // 这里简化处理
    
#elif defined(__APPLE__)
    // macOS 内存信息
    vm_size_t pageSize;
    host_page_size(mach_host_self(), &pageSize);
    
    vm_statistics64_data_t vmStats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmStats, &count) == KERN_SUCCESS) {
        info.totalMemory = (vmStats.wire_count + vmStats.active_count + vmStats.inactive_count + vmStats.free_count) * pageSize;
        info.availableMemory = vmStats.free_count * pageSize;
    }
    
    // CPU 信息
    processor_cpu_load_info_t cpuLoad;
    mach_msg_type_number_t cpuCount;
    
    if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &cpuCount, (processor_info_array_t*)&cpuLoad, &cpuCount) == KERN_SUCCESS) {
        // 计算 CPU 使用率
        uint64_t totalTicks = 0;
        uint64_t idleTicks = 0;
        
        for (uint32_t i = 0; i < cpuCount; i++) {
            totalTicks += cpuLoad[i].cpu_ticks[CPU_STATE_USER] + 
                         cpuLoad[i].cpu_ticks[CPU_STATE_SYSTEM] + 
                         cpuLoad[i].cpu_ticks[CPU_STATE_IDLE] + 
                         cpuLoad[i].cpu_ticks[CPU_STATE_NICE];
            idleTicks += cpuLoad[i].cpu_ticks[CPU_STATE_IDLE];
        }
        
        if (totalTicks > 0) {
            info.cpuUsage = 100 - (idleTicks * 100 / totalTicks);
        }
    }
    
#elif defined(__linux__)
    // Linux 内存信息
    std::ifstream memFile("/proc/meminfo");
    if (memFile.is_open()) {
        std::string line;
        while (std::getline(memFile, line)) {
            if (line.find("MemTotal:") == 0) {
                std::istringstream iss(line);
                std::string label, value, unit;
                iss >> label >> value >> unit;
                info.totalMemory = std::stoull(value) * 1024; // KB to bytes
            } else if (line.find("MemAvailable:") == 0) {
                std::istringstream iss(line);
                std::string label, value, unit;
                iss >> label >> value >> unit;
                info.availableMemory = std::stoull(value) * 1024; // KB to bytes
            }
        }
    }
    
    // CPU 信息
    std::ifstream cpuFile("/proc/stat");
    if (cpuFile.is_open()) {
        std::string line;
        if (std::getline(cpuFile, line) && line.find("cpu ") == 0) {
            std::istringstream iss(line);
            std::string label;
            uint64_t user, nice, system, idle, iowait, irq, softirq;
            iss >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
            
            uint64_t totalTicks = user + nice + system + idle + iowait + irq + softirq;
            uint64_t idleTicks = idle + iowait;
            
            if (totalTicks > 0) {
                info.cpuUsage = 100 - (idleTicks * 100 / totalTicks);
            }
        }
    }
#endif
    
    return info;
}

std::vector<CrossPlatformSystemInfo::SpecialPath> CrossPlatformSystemInfo::GetSpecialPaths() {
    std::vector<SpecialPath> paths;
    
#ifdef PLATFORM_WINDOWS
    // Windows 特殊路径
    char programFiles[MAX_PATH];
    if (GetEnvironmentVariableA("ProgramFiles", programFiles, MAX_PATH)) {
        paths.push_back({programFiles, "Program Files", true});
    }
    
    char systemRoot[MAX_PATH];
    if (GetEnvironmentVariableA("SystemRoot", systemRoot, MAX_PATH)) {
        paths.push_back({systemRoot, "Windows System", true});
    }
    
    char appData[MAX_PATH];
    if (GetEnvironmentVariableA("APPDATA", appData, MAX_PATH)) {
        paths.push_back({appData, "Application Data", true});
    }
    
#elif defined(__APPLE__)
    // macOS 特殊路径
    paths.push_back({"/System/Library", "System Library", true});
    paths.push_back({"/Library", "Library", true});
    paths.push_back({"/Applications", "Applications", true});
    paths.push_back({"/usr/local", "User Applications", true});
    
    // 用户特定路径
    const char* home = getenv("HOME");
    if (home) {
        paths.push_back({std::string(home) + "/Library", "User Library", true});
        paths.push_back({std::string(home) + "/Applications", "User Applications", true});
    }
    
#elif defined(__linux__)
    // Linux 特殊路径
    paths.push_back({"/usr/bin", "User Binaries", true});
    paths.push_back({"/usr/lib", "User Libraries", true});
    paths.push_back({"/usr/local", "Local Software", true});
    paths.push_back({"/etc", "Configuration", true});
    paths.push_back({"/var/log", "Log Files", true});
    
    // 用户特定路径
    const char* home = getenv("HOME");
    if (home) {
        paths.push_back({std::string(home) + "/.local", "User Local", true});
        paths.push_back({std::string(home) + "/.config", "User Config", true});
    }
#endif
    
    return paths;
}

std::vector<std::string> CrossPlatformSystemInfo::GetConfigFiles(const std::string& pattern) {
    std::vector<std::string> configFiles;
    
#ifdef PLATFORM_WINDOWS
    std::string appData = getenv("APPDATA") ? getenv("APPDATA") : "";
    if (!appData.empty()) {
        // 搜索配置文件
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA((appData + "\\*" + pattern).c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    configFiles.push_back(appData + "\\" + findData.cFileName);
                }
            } while (FindNextFileA(hFind, &findData));
            
            FindClose(hFind);
        }
    }
    
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (home) {
        std::string configDir = std::string(home) + "/Library/Preferences";
        DIR* dir = opendir(configDir.c_str());
        
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string filename = entry->d_name;
                if (filename.find(pattern) != std::string::npos) {
                    configFiles.push_back(configDir + "/" + filename);
                }
            }
            closedir(dir);
        }
    }
    
#elif defined(__linux__)
    const char* home = getenv("HOME");
    if (home) {
        std::string configDir = std::string(home) + "/.config";
        DIR* dir = opendir(configDir.c_str());
        
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string filename = entry->d_name;
                if (filename.find(pattern) != std::string::npos) {
                    configFiles.push_back(configDir + "/" + filename);
                }
            }
            closedir(dir);
        }
    }
    
    // 系统配置文件
    DIR* etcDir = opendir("/etc");
    if (etcDir) {
        struct dirent* entry;
        while ((entry = readdir(etcDir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename.find(pattern) != std::string::npos) {
                configFiles.push_back("/etc/" + filename);
            }
        }
        closedir(etcDir);
    }
#endif
    
    return configFiles;
}

std::vector<std::string> CrossPlatformSystemInfo::GetDriverFiles() {
    std::vector<std::string> driverFiles;
    
#ifdef PLATFORM_WINDOWS
    char systemRoot[MAX_PATH];
    if (GetEnvironmentVariableA("SystemRoot", systemRoot, MAX_PATH)) {
        std::string driverDir = std::string(systemRoot) + "\\System32\\drivers";
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA((driverDir + "\\*.sys").c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    driverFiles.push_back(driverDir + "\\" + findData.cFileName);
                }
            } while (FindNextFileA(hFind, &findData));
            
            FindClose(hFind);
        }
    }
    
#elif defined(__APPLE__)
    // macOS 驱动文件
    std::vector<std::string> driverDirs = {
        "/System/Library/Extensions",
        "/Library/Extensions"
    };
    
    for (const auto& dir : driverDirs) {
        DIR* dirPtr = opendir(dir.c_str());
        if (dirPtr) {
            struct dirent* entry;
            while ((entry = readdir(dirPtr)) != nullptr) {
                std::string filename = entry->d_name;
                if (filename.find(".kext") != std::string::npos) {
                    driverFiles.push_back(dir + "/" + filename);
                }
            }
            closedir(dirPtr);
        }
    }
    
#elif defined(__linux__)
    // Linux 驱动文件
    DIR* modulesDir = opendir("/sys/module");
    if (modulesDir) {
        struct dirent* entry;
        while ((entry = readdir(modulesDir)) != nullptr) {
            if (entry->d_name[0] != '.' && entry->d_type == DT_DIR) {
                driverFiles.push_back("/sys/module/" + std::string(entry->d_name));
            }
        }
        closedir(modulesDir);
    }
    
    // 加载的内核模块
    std::ifstream modulesFile("/proc/modules");
    if (modulesFile.is_open()) {
        std::string line;
        while (std::getline(modulesFile, line)) {
            std::istringstream iss(line);
            std::string moduleName;
            iss >> moduleName;
            driverFiles.push_back("/lib/modules/" + moduleName + ".ko");
        }
    }
#endif
    
    return driverFiles;
}

bool CrossPlatformSystemInfo::IsVirtualMachine() {
    std::string hypervisor = GetHypervisorType();
    return !hypervisor.empty() && hypervisor != "None";
}

std::string CrossPlatformSystemInfo::GetHypervisorType() {
#ifdef PLATFORM_WINDOWS
    // 检查 WMI 中的虚拟化信息
    std::vector<SystemDeviceInfo> systems = QueryDevices("Win32_ComputerSystem");
    for (const auto& system : systems) {
        auto modelIt = system.properties.find("Model");
        if (modelIt != system.properties.end()) {
            std::string model = ToLower(modelIt->second);
            if (model.find("vmware") != std::string::npos) return "VMware";
            if (model.find("virtualbox") != std::string::npos) return "VirtualBox";
            if (model.find("hyper-v") != std::string::npos) return "Hyper-V";
            if (model.find("kvm") != std::string::npos) return "KVM";
            if (model.find("qemu") != std::string::npos) return "QEMU";
        }
    }
    
#elif defined(__APPLE__)
    // 检查 macOS 虚拟化
    std::ifstream cpuInfo("/proc/cpuinfo");
    if (cpuInfo.is_open()) {
        std::string line;
        while (std::getline(cpuInfo, line)) {
            if (line.find("Hypervisor vendor:") != std::string::npos) {
                if (line.find("VMware") != std::string::npos) return "VMware";
                if (line.find("Microsoft") != std::string::npos) return "Hyper-V";
            }
        }
    }
    
#elif defined(__linux__)
    // 检查 Linux 虚拟化
    std::ifstream dmiFile("/sys/class/dmi/id/product_name");
    if (dmiFile.is_open()) {
        std::string productName;
        std::getline(dmiFile, productName);
        std::string lowerName = ToLower(productName);
        
        if (lowerName.find("vmware") != std::string::npos) return "VMware";
        if (lowerName.find("virtualbox") != std::string::npos) return "VirtualBox";
        if (lowerName.find("kvm") != std::string::npos) return "KVM";
        if (lowerName.find("qemu") != std::string::npos) return "QEMU";
    }
    
    // 检查 hypervisor 文件
    if (access("/sys/hypervisor/type", F_OK) == 0) {
        std::ifstream hypervisorFile("/sys/hypervisor/type");
        if (hypervisorFile.is_open()) {
            std::string hypervisorType;
            std::getline(hypervisorFile, hypervisorType);
            return hypervisorType;
        }
    }
#endif
    
    return "None";
}

std::vector<std::string> CrossPlatformSystemInfo::GetVirtualGpuNames() {
    std::vector<std::string> virtualGpuNames;
    
#ifdef PLATFORM_WINDOWS
    std::vector<SystemDeviceInfo> gpus = GetGpuDevices();
    for (const auto& gpu : gpus) {
        std::string name = ToLower(gpu.name);
        if (name.find("microsoft basic") != std::string::npos ||
            name.find("hyper-v") != std::string::npos ||
            name.find("vmware") != std::string::npos ||
            name.find("virtualbox") != std::string::npos) {
            virtualGpuNames.push_back(gpu.name);
        }
    }
    
#elif defined(__APPLE__)
    // macOS 虚拟 GPU 检测
    std::vector<SystemDeviceInfo> gpus = GetGpuDevices();
    for (const auto& gpu : gpus) {
        std::string name = ToLower(gpu.name);
        if (name.find("virtual") != std::string::npos ||
            name.find("vmware") != std::string::npos) {
            virtualGpuNames.push_back(gpu.name);
        }
    }
    
#elif defined(__linux__)
    // Linux 虚拟 GPU 检测
    std::vector<SystemDeviceInfo> gpus = GetGpuDevices();
    for (const auto& gpu : gpus) {
        std::string name = ToLower(gpu.name);
        if (name.find("virtual") != std::string::npos ||
            name.find("vmware") != std::string::npos ||
            name.find("cirrus") != std::string::npos ||
            name.find("qxl") != std::string::npos) {
            virtualGpuNames.push_back(gpu.name);
        }
    }
#endif
    
    return virtualGpuNames;
}

// 辅助函数实现
std::string CrossPlatformSystemInfo::TrimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> CrossPlatformSystemInfo::SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;
    
    while (std::getline(iss, token, delimiter)) {
        result.push_back(TrimString(token));
    }
    
    return result;
}

std::string CrossPlatformSystemInfo::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool CrossPlatformSystemInfo::StringContains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}