#include "TemperatureWrapper.h"
#include "../Utils/Logger.h"

#ifdef PLATFORM_WINDOWS
    #include "../gpu/GpuInfo.h"
    #include "../utils/WmiManager.h"
    #include "../Utils/LibreHardwareMonitorBridge.h"
    #include <algorithm>
    #include <cwctype>
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/graphics/IOGraphicsLib.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <unistd.h>
    #include <fstream>
    #include <sstream>
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
    #include <glob.h>
#endif

// 静态成员定义
bool TemperatureWrapper::initialized = false;

#ifdef PLATFORM_WINDOWS
    GpuInfo* TemperatureWrapper::gpuInfo = nullptr;
    WmiManager* TemperatureWrapper::wmiManager = nullptr;
    int TemperatureWrapper::temperatureCallCount = 0;
#endif

// macOS温度监控实现
#ifdef PLATFORM_MACOS
std::vector<std::pair<std::string, double>> TemperatureWrapper::GetMacTemperatures() {
    std::vector<std::pair<std::string, double>> temps;
    
    try {
        // 获取CPU温度
        double cpuTemp = GetMacCpuTemperature();
        if (cpuTemp > 0) {
            temps.emplace_back("CPU", cpuTemp);
            Logger::Debug("TemperatureWrapper: CPU temperature: " + std::to_string(cpuTemp) + "°C");
        }
        
        // 获取GPU温度
        double gpuTemp = GetMacGpuTemperature();
        if (gpuTemp > 0) {
            temps.emplace_back("GPU", gpuTemp);
            Logger::Debug("TemperatureWrapper: GPU temperature: " + std::to_string(gpuTemp) + "°C");
        }
        
        // 获取其他传感器温度
        auto sensorTemps = GetMacSensorTemperatures();
        temps.insert(temps.end(), sensorTemps.begin(), sensorTemps.end());
        
        LogMacSensorInfo(sensorTemps);
    }
    catch (const std::exception& e) {
        Logger::Error("GetMacTemperatures failed: " + std::string(e.what()));
    }
    
    return temps;
}

double TemperatureWrapper::GetMacCpuTemperature() {
    try {
        // 使用sysctl获取CPU温度
        size_t size = 0;
        sysctlbyname("hw.ncpu", nullptr, &size, nullptr, 0);
        
        // 尝试通过IOKit获取CPU温度传感器
        io_iterator_t iter;
        kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault,
            IOServiceMatching("IOPlatformSensorHWSensor"), &iter);
        
        if (kr == KERN_SUCCESS) {
            io_service_t service;
            while ((service = IOIteratorNext(iter)) != 0) {
                // 检查是否为CPU温度传感器
                CFStringRef sensorType = (CFStringRef)IORegistryEntryCreateCFProperty(
                    service, CFSTR("sensor-type"), kCFAllocatorDefault, 0);
                
                if (sensorType && CFStringCompare(sensorType, CFSTR("temperature"), 0) == kCFCompareEqualTo) {
                    double temp = GetMacSensorTemperature("CPU");
                    if (temp > 0) {
                        CFRelease(sensorType);
                        IOObjectRelease(service);
                        IOObjectRelease(iter);
                        return temp;
                    }
                }
                
                if (sensorType) CFRelease(sensorType);
                IOObjectRelease(service);
            }
            IOObjectRelease(iter);
        }
        
        // 备用方法：通过powermetrics获取
        FILE* pipe = popen("powermetrics --samplers cpu_power -i 1 -n 1 | grep \"CPU die temperature\"", "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                // 解析温度值
                std::string line(buffer);
                size_t pos = line.find("temperature:");
                if (pos != std::string::npos) {
                    std::string tempStr = line.substr(pos + 12);
                    double temp = std::stod(tempStr);
                    pclose(pipe);
                    return temp;
                }
            }
            pclose(pipe);
        }
    }
    catch (const std::exception& e) {
        Logger::Error("GetMacCpuTemperature failed: " + std::string(e.what()));
    }
    
    return 0.0;
}

double TemperatureWrapper::GetMacGpuTemperature() {
    try {
        // 通过IOKit获取GPU温度
        io_iterator_t iter;
        kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault,
            IOServiceMatching("IOAccelerator"), &iter);
        
        if (kr == KERN_SUCCESS) {
            io_service_t service;
            while ((service = IOIteratorNext(iter)) != 0) {
                double temp = GetMacSensorTemperature("GPU");
                if (temp > 0) {
                    IOObjectRelease(service);
                    IOObjectRelease(iter);
                    return temp;
                }
                IOObjectRelease(service);
            }
            IOObjectRelease(iter);
        }
        
        // 备用方法：通过system_profiler获取
        FILE* pipe = popen("system_profiler SPDisplaysDataType | grep -i temperature", "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                std::string line(buffer);
                size_t pos = line.find("Temperature:");
                if (pos != std::string::npos) {
                    std::string tempStr = line.substr(pos + 12);
                    double temp = std::stod(tempStr);
                    pclose(pipe);
                    return temp;
                }
            }
            pclose(pipe);
        }
    }
    catch (const std::exception& e) {
        Logger::Error("GetMacGpuTemperature failed: " + std::string(e.what()));
    }
    
    return 0.0;
}

std::vector<std::pair<std::string, double>> TemperatureWrapper::GetMacSensorTemperatures() {
    std::vector<std::pair<std::string, double>> sensors;
    
    try {
        // 获取所有温度传感器
        io_iterator_t iter;
        kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault,
            IOServiceMatching("IOPlatformSensorHWSensor"), &iter);
        
        if (kr == KERN_SUCCESS) {
            io_service_t service;
            int sensorCount = 0;
            
            while ((service = IOIteratorNext(iter)) != 0 && sensorCount < 10) { // 限制传感器数量
                CFStringRef sensorName = (CFStringRef)IORegistryEntryCreateCFProperty(
                    service, CFSTR("name"), kCFAllocatorDefault, 0);
                
                if (sensorName) {
                    char nameBuffer[256];
                    CFStringGetCString(sensorName, nameBuffer, sizeof(nameBuffer), kCFStringEncodingUTF8);
                    
                    // 尝试获取温度值
                    double temp = GetMacSensorTemperature(nameBuffer);
                    if (temp > 0 && temp < 150) { // 合理的温度范围
                        sensors.emplace_back(nameBuffer, temp);
                        sensorCount++;
                    }
                    
                    CFRelease(sensorName);
                }
                
                IOObjectRelease(service);
            }
            IOObjectRelease(iter);
        }
    }
    catch (const std::exception& e) {
        Logger::Error("GetMacSensorTemperatures failed: " + std::string(e.what()));
    }
    
    return sensors;
}

double TemperatureWrapper::GetMacSensorTemperature(const std::string& sensorPath) {
    try {
        // 通过IOKit获取传感器温度值
        io_iterator_t iter;
        kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault,
            IOServiceMatching("IOPlatformSensorHWSensor"), &iter);
        
        if (kr == KERN_SUCCESS) {
            io_service_t service;
            while ((service = IOIteratorNext(iter)) != 0) {
                CFStringRef name = (CFStringRef)IORegistryEntryCreateCFProperty(
                    service, CFSTR("name"), kCFAllocatorDefault, 0);
                
                if (name) {
                    char nameBuffer[256];
                    CFStringGetCString(name, nameBuffer, sizeof(nameBuffer), kCFStringEncodingUTF8);
                    
                    if (std::string(nameBuffer) == sensorPath) {
                        // 获取温度值
                        CFNumberRef temperature = (CFNumberRef)IORegistryEntryCreateCFProperty(
                            service, CFSTR("current-value"), kCFAllocatorDefault, 0);
                        
                        if (temperature) {
                            double temp;
                            if (CFNumberGetValue(temperature, kCFNumberDoubleType, &temp)) {
                                CFRelease(temperature);
                                CFRelease(name);
                                IOObjectRelease(service);
                                IOObjectRelease(iter);
                                return temp;
                            }
                            CFRelease(temperature);
                        }
                    }
                    CFRelease(name);
                }
                IOObjectRelease(service);
            }
            IOObjectRelease(iter);
        }
    }
    catch (const std::exception& e) {
        Logger::Error("GetMacSensorTemperature failed: " + std::string(e.what()));
    }
    
    return 0.0;
}

void TemperatureWrapper::LogMacSensorInfo(const std::vector<std::pair<std::string, double>>& sensors) {
    if (sensors.empty()) {
        Logger::Debug("TemperatureWrapper: No macOS sensors found");
        return;
    }
    
    std::string msg = "TemperatureWrapper: macOS sensors (" + std::to_string(sensors.size()) + "): ";
    for (size_t i = 0; i < sensors.size(); ++i) {
        msg += sensors[i].first + "=" + std::to_string(sensors[i].second) + "°C";
        if (i + 1 < sensors.size()) msg += ", ";
    }
    Logger::Debug(msg);
}
#endif

// Cross-platform implementations
void TemperatureWrapper::Initialize() {
    try {
#ifdef PLATFORM_WINDOWS
        LibreHardwareMonitorBridge::Initialize();
        initialized = true;
        // 初始化GpuInfo
        if (!wmiManager) wmiManager = new WmiManager();
        if (!gpuInfo && wmiManager && wmiManager->IsInitialized()) {
            gpuInfo = new GpuInfo(*wmiManager);
            LogRealGpuNames(gpuInfo->GetGpuData(), true); // 初始化时总是显示
        } else if (!wmiManager || !wmiManager->IsInitialized()) {
            Logger::Warn("TemperatureWrapper: WmiManager initialization failed, cannot get local GPU temperature");
        }
#elif defined(PLATFORM_MACOS)
        // macOS温度监控初始化
        initialized = true;
        Logger::Debug("TemperatureWrapper: macOS temperature monitoring initialized");
#elif defined(PLATFORM_LINUX)
        // Linux温度监控初始化
        initialized = true;
        Logger::Debug("TemperatureWrapper: Linux temperature monitoring initialized");
#endif
    }
    catch (...) {
        initialized = false;
        throw;
    }
}

void TemperatureWrapper::Cleanup() {
    if (initialized) {
#ifdef PLATFORM_WINDOWS
        LibreHardwareMonitorBridge::Cleanup();
#endif
        initialized = false;
    }
#ifdef PLATFORM_WINDOWS
    if (gpuInfo) { delete gpuInfo; gpuInfo = nullptr; }
    if (wmiManager) { delete wmiManager; wmiManager = nullptr; }
#endif
}

std::vector<std::pair<std::string, double>> TemperatureWrapper::GetTemperatures() {
    std::vector<std::pair<std::string, double>> temps;
    
    if (!initialized) {
        Logger::Warn("TemperatureWrapper: Not initialized");
        return temps;
    }
    
    try {
#ifdef PLATFORM_WINDOWS
        // 增加调用计数器
        temperatureCallCount++;
        bool isDetailedLogging = (temperatureCallCount % 5 == 1);
        
        // 1. 先获取libre的
        auto libreTemps = LibreHardwareMonitorBridge::GetTemperatures();
        temps.insert(temps.end(), libreTemps.begin(), libreTemps.end());
        
        // 2. 再获取GpuInfo的（过滤虚拟GPU）
        if (gpuInfo) {
            const auto& gpus = gpuInfo->GetGpuData();
            
            for (const auto& gpu : gpus) {
                if (gpu.isVirtual) continue;
                std::string gpuName(gpu.name.begin(), gpu.name.end());
                temps.emplace_back("GPU: " + gpuName, static_cast<double>(gpu.temperature));
            }
        }
        
        // 防止计数器溢出
        if (temperatureCallCount >= 100) temperatureCallCount = 0;
        
#elif defined(PLATFORM_MACOS)
        temps = GetMacTemperatures();
#elif defined(PLATFORM_LINUX)
        temps = GetLinuxTemperatures();
#endif
    }
    catch (const std::exception& e) {
        Logger::Error("TemperatureWrapper: Exception while getting temperatures: " + std::string(e.what()));
    }
    
    return temps;
}

// Windows-specific helper function
#ifdef PLATFORM_WINDOWS
static void LogRealGpuNames(const std::vector<GpuInfo::GpuData>& gpus, bool isDetailedLogging) {
    if (!isDetailedLogging) return; // 只在详细日志周期显示
    
    std::vector<std::string> realGpuNames;
    for (const auto& gpu : gpus) {
        if (!gpu.isVirtual) {
            realGpuNames.emplace_back(gpu.name.begin(), gpu.name.end());
        }
    }
    if (!realGpuNames.empty()) {
        std::string msg = "TemperatureWrapper: Real GPU name list: ";
        for (size_t i = 0; i < realGpuNames.size(); ++i) {
            msg += realGpuNames[i];
            if (i + 1 < realGpuNames.size()) msg += ", ";
        }
        Logger::Debug(msg);
    } else {
        Logger::Debug("TemperatureWrapper: No real GPU detected");
    }
}
#endif // PLATFORM_WINDOWS

// Linux温度监控实现
#ifdef PLATFORM_LINUX
std::vector<std::pair<std::string, double>> TemperatureWrapper::GetLinuxTemperatures() {
    std::vector<std::pair<std::string, double>> temps;
    
    try {
        // 获取CPU温度
        double cpuTemp = GetLinuxCpuTemperature();
        if (cpuTemp > 0) {
            temps.emplace_back("CPU", cpuTemp);
            Logger::Debug("TemperatureWrapper: CPU temperature: " + std::to_string(cpuTemp) + "°C");
        }
        
        // 获取其他传感器温度
        auto sensorTemps = GetLinuxSensorTemperatures();
        temps.insert(temps.end(), sensorTemps.begin(), sensorTemps.end());
    }
    catch (const std::exception& e) {
        Logger::Error("GetLinuxTemperatures failed: " + std::string(e.what()));
    }
    
    return temps;
}

double TemperatureWrapper::GetLinuxCpuTemperature() {
    try {
        // 尝试从 /sys/class/thermal/thermal_zone*/temp 获取CPU温度
        glob_t globResult;
        if (glob("/sys/class/thermal/thermal_zone*/temp", GLOB_NOSORT, nullptr, &globResult) == 0) {
            for (size_t i = 0; i < globResult.gl_pathc; i++) {
                std::string tempPath = globResult.gl_pathv[i];
                std::ifstream tempFile(tempPath);
                if (tempFile.is_open()) {
                    int tempMilliCelsius;
                    tempFile >> tempMilliCelsius;
                    tempFile.close();
                    
                    // 转换为摄氏度
                    double tempCelsius = tempMilliCelsius / 1000.0;
                    
                    // 检查是否为CPU温度（通常thermal_zone0是CPU）
                    if (tempCelsius > 0 && tempCelsius < 150) {
                        globfree(&globResult);
                        return tempCelsius;
                    }
                }
            }
            globfree(&globResult);
        }
        
        // 备用方法：从 /sys/devices/platform/coretemp.0/hwmon/hwmon*/temp1_input
        glob_t hwmonGlob;
        if (glob("/sys/devices/platform/coretemp.0/hwmon/hwmon*/temp1_input", GLOB_NOSORT, nullptr, &hwmonGlob) == 0) {
            for (size_t i = 0; i < hwmonGlob.gl_pathc; i++) {
                std::string tempPath = hwmonGlob.gl_pathv[i];
                std::ifstream tempFile(tempPath);
                if (tempFile.is_open()) {
                    int tempMilliCelsius;
                    tempFile >> tempMilliCelsius;
                    tempFile.close();
                    
                    double tempCelsius = tempMilliCelsius / 1000.0;
                    if (tempCelsius > 0 && tempCelsius < 150) {
                        globfree(&hwmonGlob);
                        return tempCelsius;
                    }
                }
            }
            globfree(&hwmonGlob);
        }
    }
    catch (const std::exception& e) {
        Logger::Error("GetLinuxCpuTemperature failed: " + std::string(e.what()));
    }
    
    return 0.0;
}

std::vector<std::pair<std::string, double>> TemperatureWrapper::GetLinuxSensorTemperatures() {
    std::vector<std::pair<std::string, double>> sensors;
    
    try {
        // 扫描 /sys/class/hwmon/hwmon*/ 获取所有硬件监控传感器
        glob_t globResult;
        if (glob("/sys/class/hwmon/hwmon*/", GLOB_NOSORT, nullptr, &globResult) == 0) {
            for (size_t i = 0; i < globResult.gl_pathc; i++) {
                std::string hwmonPath = globResult.gl_pathv[i];
                
                // 获取hwmon名称
                std::string namePath = hwmonPath + "/name";
                std::ifstream nameFile(namePath);
                if (!nameFile.is_open()) continue;
                
                std::string sensorName;
                std::getline(nameFile, sensorName);
                nameFile.close();
                
                // 扫描所有温度输入
                std::string tempPattern = hwmonPath + "/temp*_input";
                glob_t tempGlob;
                if (glob(tempPattern.c_str(), GLOB_NOSORT, nullptr, &tempGlob) == 0) {
                    for (size_t j = 0; j < tempGlob.gl_pathc; j++) {
                        std::string tempPath = tempGlob.gl_pathv[j];
                        double temp = GetLinuxSensorTemperature(tempPath);
                        
                        if (temp > 0 && temp < 150) {
                            // 提取温度编号
                            size_t pos = tempPath.find_last_of('/');
                            std::string tempFileName = (pos != std::string::npos) ? 
                                tempPath.substr(pos + 1) : tempPath;
                            
                            sensors.emplace_back(sensorName + " - " + tempFileName, temp);
                        }
                    }
                    globfree(&tempGlob);
                }
            }
            globfree(&globResult);
        }
    }
    catch (const std::exception& e) {
        Logger::Error("GetLinuxSensorTemperatures failed: " + std::string(e.what()));
    }
    
    return sensors;
}

double TemperatureWrapper::GetLinuxSensorTemperature(const std::string& sensorPath) {
    try {
        std::ifstream tempFile(sensorPath);
        if (!tempFile.is_open()) {
            return 0.0;
        }
        
        int tempMilliCelsius;
        tempFile >> tempMilliCelsius;
        tempFile.close();
        
        // 转换为摄氏度
        double tempCelsius = tempMilliCelsius / 1000.0;
        
        // 验证温度的合理性
        if (tempCelsius < -50 || tempCelsius > 200) {
            return 0.0;
        }
        
        return tempCelsius;
    }
    catch (const std::exception& e) {
        Logger::Error("GetLinuxSensorTemperature failed for " + sensorPath + ": " + std::string(e.what()));
        return 0.0;
    }
}
#endif // PLATFORM_LINUX

bool TemperatureWrapper::IsInitialized() {
    return initialized;
}
