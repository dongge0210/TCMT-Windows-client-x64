#include "MacCpuAdapter.h"
#include "MacCpuInfo.h"
#include "../../Utils/Logger.h"

#ifdef PLATFORM_MACOS

MacCpuAdapter::MacCpuAdapter() {
    m_macCpuInfo = std::make_unique<MacCpuInfo>();
    if (!m_macCpuInfo || !m_macCpuInfo->Initialize()) {
        Logger::Error("Failed to initialize MacCpuInfo");
        m_macCpuInfo.reset();
    }
}

MacCpuAdapter::~MacCpuAdapter() {
    Cleanup();
}

double MacCpuAdapter::GetUsage() {
    if (!m_macCpuInfo) return 0.0;
    return m_macCpuInfo->GetTotalUsage();
}

std::string MacCpuAdapter::GetName() {
    if (!m_macCpuInfo) return "";
    return m_macCpuInfo->GetName();
}

int MacCpuAdapter::GetTotalCores() const {
    if (!m_macCpuInfo) return 0;
    return static_cast<int>(m_macCpuInfo->GetTotalCores());
}

int MacCpuAdapter::GetSmallCores() const {
    if (!m_macCpuInfo) return 0;
    return static_cast<int>(m_macCpuInfo->GetEfficiencyCores());
}

int MacCpuAdapter::GetLargeCores() const {
    if (!m_macCpuInfo) return 0;
    return static_cast<int>(m_macCpuInfo->GetPerformanceCores());
}

double MacCpuAdapter::GetLargeCoreSpeed() const {
    if (!m_macCpuInfo) return 0.0;
    // 性能核心频率通常等于当前频率
    return m_macCpuInfo->GetCurrentFrequency();
}

double MacCpuAdapter::GetSmallCoreSpeed() const {
    if (!m_macCpuInfo) return 0.0;
    // 能效核心频率通常略低于性能核心
    return m_macCpuInfo->GetCurrentFrequency() * 0.8;
}

DWORD MacCpuAdapter::GetCurrentSpeed() const {
    if (!m_macCpuInfo) return 0;
    // macOS下返回当前频率，转换为MHz
    return static_cast<DWORD>(m_macCpuInfo->GetCurrentFrequency());
}

bool MacCpuAdapter::IsHyperThreadingEnabled() const {
    if (!m_macCpuInfo) return false;
    return m_macCpuInfo->IsHyperThreadingEnabled();
}

bool MacCpuAdapter::IsVirtualizationEnabled() const {
    if (!m_macCpuInfo) return false;
    return m_macCpuInfo->IsVirtualizationEnabled();
}

double MacCpuAdapter::GetLastSampleIntervalMs() const {
    if (!m_macCpuInfo) return 0.0;
    // 简化处理，返回默认采样间隔
    return 1000.0; // 1秒
}

double MacCpuAdapter::GetBaseFrequencyMHz() const {
    if (!m_macCpuInfo) return 0.0;
    return m_macCpuInfo->GetBaseFrequency();
}

double MacCpuAdapter::GetCurrentFrequencyMHz() const {
    if (!m_macCpuInfo) return 0.0;
    return m_macCpuInfo->GetCurrentFrequency();
}

bool MacCpuAdapter::Initialize() {
    return m_macCpuInfo != nullptr && m_macCpuInfo->IsInitialized();
}

void MacCpuAdapter::Cleanup() {
    if (m_macCpuInfo) {
        m_macCpuInfo->Cleanup();
        m_macCpuInfo.reset();
    }
}

bool MacCpuAdapter::Update() {
    if (!m_macCpuInfo) return false;
    return m_macCpuInfo->Update();
}

#endif // PLATFORM_MACOS