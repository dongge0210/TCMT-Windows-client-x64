#include "TemperatureWrapper.h"
#include "../gpu/GpuInfo.h"
#include "../utils/Logger.h"
#include "../utils/WmiManager.h"
#include "../Utils/LibreHardwareMonitorBridge.h"
#include <algorithm>
#include <cwctype>

// 静态成员定义
bool TemperatureWrapper::initialized = false;
static GpuInfo* gpuInfo = nullptr;
static WmiManager* wmiManager = nullptr;
static int temperatureCallCount = 0; // 添加调用计数器

// 输出真实GPU名称列表（过滤虚拟GPU）- 只在详细日志时显示
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

void TemperatureWrapper::Initialize() {
    try {
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
    }
    catch (...) {
        initialized = false;
        throw;
    }
}

void TemperatureWrapper::Cleanup() {
    if (initialized) {
        LibreHardwareMonitorBridge::Cleanup();
        initialized = false;
    }
    if (gpuInfo) { delete gpuInfo; gpuInfo = nullptr; }
    if (wmiManager) { delete wmiManager; wmiManager = nullptr; }
}

std::vector<std::pair<std::string, double>> TemperatureWrapper::GetTemperatures() {
    std::vector<std::pair<std::string, double>> temps;
    
    // 增加调用计数器
    temperatureCallCount++;
    
    // 只在每5次调用时显示详细日志（与主循环的详细日志周期同步）
    bool isDetailedLogging = (temperatureCallCount % 5 == 1);
    
    // 1. 先获取libre的
    if (initialized) {
        try {
            auto libreTemps = LibreHardwareMonitorBridge::GetTemperatures();
            if (isDetailedLogging) {
            }
            temps.insert(temps.end(), libreTemps.begin(), libreTemps.end());
        } catch (...) {
            if (isDetailedLogging) {
                Logger::Warn("TemperatureWrapper: Exception while getting temperatures from libre");
            }
        }
    }
    
    // 2. 再获取GpuInfo的（过滤虚拟GPU）
    if (gpuInfo) {
        const auto& gpus = gpuInfo->GetGpuData();
        if (isDetailedLogging) {
            Logger::Debug("TemperatureWrapper: GpuInfo GPU count: " + std::to_string(gpus.size()));
            LogRealGpuNames(gpus, isDetailedLogging);
        }
        
        for (const auto& gpu : gpus) {
            if (gpu.isVirtual) {
                if (isDetailedLogging) {
                    Logger::Debug("TemperatureWrapper: Skipping virtual GPU: " + std::string(gpu.name.begin(), gpu.name.end()));
                }
                continue;
            }
            std::string gpuName(gpu.name.begin(), gpu.name.end());
            if (isDetailedLogging) {
                Logger::Debug("TemperatureWrapper: GpuInfo detected GPU: " + gpuName + ", Temperature: " + std::to_string(gpu.temperature));
            }
            temps.emplace_back("GPU: " + gpuName, static_cast<double>(gpu.temperature));
        }
    } else {
        if (isDetailedLogging) {
            Logger::Warn("TemperatureWrapper: GpuInfo not initialized");
        }
    }
    
    if (isDetailedLogging) {
        Logger::Debug("TemperatureWrapper: Total number of temperatures: " + std::to_string(temps.size()));
    }
    
    // 防止计数器溢出
    if (temperatureCallCount >= 100) temperatureCallCount = 0;
    
    return temps;
}

bool TemperatureWrapper::IsInitialized() {
    return initialized;
}
