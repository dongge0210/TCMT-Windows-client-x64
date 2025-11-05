#include "MacGpuAdapter.h"
#include "../../Utils/Logger.h"

#ifdef PLATFORM_MACOS

MacGpuAdapter::MacGpuAdapter() {
    m_macGpuInfo = std::make_unique<MacGpuInfo>();
    if (!m_macGpuInfo || !m_macGpuInfo->Initialize()) {
        Logger::Error("Failed to initialize MacGpuInfo");
        SetError("Failed to initialize MacGpuInfo");
        m_macGpuInfo.reset();
    } else {
        UpdateGpuData();
    }
}

MacGpuAdapter::~MacGpuAdapter() {
    Cleanup();
}

const std::vector<GpuInfo::GpuData>& MacGpuAdapter::GetGpuData() const {
    return m_gpuData;
}

double MacGpuAdapter::GetGpuUsage() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetGpuUsage();
}

double MacGpuAdapter::GetMemoryUsage() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetMemoryUsage();
}

double MacGpuAdapter::GetVideoDecoderUsage() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetVideoDecoderUsage();
}

double MacGpuAdapter::GetVideoEncoderUsage() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetVideoEncoderUsage();
}

double MacGpuAdapter::GetCurrentFrequency() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetCurrentFrequency();
}

double MacGpuAdapter::GetBaseFrequency() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetBaseFrequency();
}

double MacGpuAdapter::GetMaxFrequency() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetMaxFrequency();
}

double MacGpuAdapter::GetTemperature() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetTemperature();
}

double MacGpuAdapter::GetHotspotTemperature() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetHotspotTemperature();
}

double MacGpuAdapter::GetMemoryTemperature() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetMemoryTemperature();
}

double MacGpuAdapter::GetPowerUsage() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetPowerUsage();
}

double MacGpuAdapter::GetBoardPower() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetBoardPower();
}

double MacGpuAdapter::GetMaxPowerLimit() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetMaxPowerLimit();
}

double MacGpuAdapter::GetFanSpeed() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetFanSpeed();
}

double MacGpuAdapter::GetFanSpeedPercent() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetFanSpeedPercent();
}

uint64_t MacGpuAdapter::GetComputeUnits() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0;
    }
    return m_macGpuInfo->GetComputeUnits();
}

std::string MacGpuAdapter::GetArchitecture() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return "Unknown";
    }
    return m_macGpuInfo->GetArchitecture();
}

double MacGpuAdapter::GetPerformanceRating() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0.0;
    }
    return m_macGpuInfo->GetPerformanceRating();
}

uint64_t MacGpuAdapter::GetDedicatedMemory() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0;
    }
    return m_macGpuInfo->GetDedicatedMemory();
}

uint64_t MacGpuAdapter::GetSharedMemory() const {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return 0;
    }
    return m_macGpuInfo->GetSharedMemory();
}

uint64_t MacGpuAdapter::GetTotalMemory() const {
    return GetDedicatedMemory() + GetSharedMemory();
}

bool MacGpuAdapter::Initialize() {
    if (!m_macGpuInfo) {
        m_macGpuInfo = std::make_unique<MacGpuInfo>();
    }
    
    if (m_macGpuInfo && m_macGpuInfo->Initialize()) {
        UpdateGpuData();
        m_lastError.clear();
        return true;
    } else {
        SetError("Failed to initialize MacGpuInfo");
        return false;
    }
}

void MacGpuAdapter::Cleanup() {
    if (m_macGpuInfo) {
        m_macGpuInfo->Cleanup();
        m_macGpuInfo.reset();
    }
    m_gpuData.clear();
}

bool MacGpuAdapter::Update() {
    if (!m_macGpuInfo) {
        SetError("MacGpuInfo not initialized");
        return false;
    }
    
    if (m_macGpuInfo->Update()) {
        UpdateGpuData();
        m_lastError.clear();
        return true;
    } else {
        SetError("Failed to update MacGpuInfo");
        return false;
    }
}

std::string MacGpuAdapter::GetLastError() const {
    return m_lastError;
}

void MacGpuAdapter::UpdateGpuData() {
    if (!m_macGpuInfo) {
        return;
    }
    
    // 清空现有数据
    m_gpuData.clear();
    
    // 创建兼容的GpuData结构
    GpuInfo::GpuData gpuData;
    
    // 填充基本信息
    std::string name = m_macGpuInfo->GetName();
    gpuData.name = std::wstring(name.begin(), name.end());
    gpuData.deviceId = L"Apple_Silicon_GPU";
    gpuData.dedicatedMemory = m_macGpuInfo->GetDedicatedMemory();
    gpuData.coreClock = static_cast<uint64_t>(m_macGpuInfo->GetCurrentFrequency() * 1000000); // 转换为Hz
    gpuData.isNvidia = false;
    gpuData.isIntegrated = true;
    gpuData.isVirtual = false;
    gpuData.computeCapabilityMajor = 1; // Apple Silicon GPU计算能力
    gpuData.computeCapabilityMinor = 0;
    gpuData.temperature = static_cast<unsigned int>(m_macGpuInfo->GetTemperature());
    
    // 添加到列表
    m_gpuData.push_back(gpuData);
}

#endif // PLATFORM_MACOS