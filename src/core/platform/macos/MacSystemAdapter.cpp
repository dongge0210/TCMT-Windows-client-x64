#include "MacSystemAdapter.h"
#include "MacSystemInfo.h"
#include "../../Utils/Logger.h"

#ifdef PLATFORM_MACOS

MacSystemAdapter::MacSystemAdapter() {
    m_macSystemInfo = std::make_unique<MacSystemInfo>();
    if (!m_macSystemInfo || !m_macSystemInfo->Initialize()) {
        Logger::Error("Failed to initialize MacSystemInfo");
        m_macSystemInfo.reset();
    }
}

MacSystemAdapter::~MacSystemAdapter() {
    Cleanup();
}

bool MacSystemAdapter::Initialize() {
    return m_macSystemInfo != nullptr && m_macSystemInfo->IsInitialized();
}

void MacSystemAdapter::Cleanup() {
    if (m_macSystemInfo) {
        m_macSystemInfo->Cleanup();
        m_macSystemInfo.reset();
    }
}

bool MacSystemAdapter::IsInitialized() const {
    return m_macSystemInfo != nullptr && m_macSystemInfo->IsInitialized();
}

bool MacSystemAdapter::Update() {
    if (!m_macSystemInfo) return false;
    return m_macSystemInfo->Update();
}

bool MacSystemAdapter::IsDataValid() const {
    if (!m_macSystemInfo) return false;
    return m_macSystemInfo->IsDataValid();
}

std::string MacSystemAdapter::GetOSName() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetOSName();
}

std::string MacSystemAdapter::GetOSVersion() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetOSVersion();
}

std::string MacSystemAdapter::GetOSBuild() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetOSBuild();
}

std::string MacSystemAdapter::GetHostname() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetHostname();
}

std::string MacSystemAdapter::GetUsername() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetUsername();
}

uint64_t MacSystemAdapter::GetUptimeSeconds() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetUptimeSeconds();
}

std::string MacSystemAdapter::GetBootTime() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetBootTime();
}

std::string MacSystemAdapter::GetUptimeFormatted() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetUptimeFormatted();
}

double MacSystemAdapter::GetLoadAverage1Min() const {
    if (!m_macSystemInfo) return 0.0;
    return m_macSystemInfo->GetLoadAverage1Min();
}

double MacSystemAdapter::GetLoadAverage5Min() const {
    if (!m_macSystemInfo) return 0.0;
    return m_macSystemInfo->GetLoadAverage5Min();
}

double MacSystemAdapter::GetLoadAverage15Min() const {
    if (!m_macSystemInfo) return 0.0;
    return m_macSystemInfo->GetLoadAverage15Min();
}

uint32_t MacSystemAdapter::GetTotalProcesses() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetTotalProcesses();
}

uint32_t MacSystemAdapter::GetRunningProcesses() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetRunningProcesses();
}

uint32_t MacSystemAdapter::GetSleepingProcesses() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetSleepingProcesses();
}

uint32_t MacSystemAdapter::GetThreadCount() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetThreadCount();
}

uint64_t MacSystemAdapter::GetTotalPhysicalMemory() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetTotalPhysicalMemory();
}

uint64_t MacSystemAdapter::GetAvailablePhysicalMemory() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetAvailablePhysicalMemory();
}

uint64_t MacSystemAdapter::GetUsedPhysicalMemory() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetUsedPhysicalMemory();
}

double MacSystemAdapter::GetMemoryUsagePercentage() const {
    if (!m_macSystemInfo) return 0.0;
    return m_macSystemInfo->GetMemoryUsagePercentage();
}

uint64_t MacSystemAdapter::GetTotalDiskSpace() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetTotalDiskSpace();
}

uint64_t MacSystemAdapter::GetAvailableDiskSpace() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetAvailableDiskSpace();
}

uint64_t MacSystemAdapter::GetUsedDiskSpace() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetUsedDiskSpace();
}

double MacSystemAdapter::GetDiskUsagePercentage() const {
    if (!m_macSystemInfo) return 0.0;
    return m_macSystemInfo->GetDiskUsagePercentage();
}

std::string MacSystemAdapter::GetPrimaryInterface() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetPrimaryInterface();
}

uint64_t MacSystemAdapter::GetNetworkBytesReceived() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetNetworkBytesReceived();
}

uint64_t MacSystemAdapter::GetNetworkBytesSent() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetNetworkBytesSent();
}

uint64_t MacSystemAdapter::GetNetworkPacketsReceived() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetNetworkPacketsReceived();
}

uint64_t MacSystemAdapter::GetNetworkPacketsSent() const {
    if (!m_macSystemInfo) return 0;
    return m_macSystemInfo->GetNetworkPacketsSent();
}

bool MacSystemAdapter::IsSystemHealthy() const {
    if (!m_macSystemInfo) return false;
    return m_macSystemInfo->IsSystemHealthy();
}

std::string MacSystemAdapter::GetSystemStatus() const {
    if (!m_macSystemInfo) return "Unknown";
    return m_macSystemInfo->GetSystemStatus();
}

std::string MacSystemAdapter::GetLastError() const {
    if (!m_macSystemInfo) return "";
    return m_macSystemInfo->GetLastError();
}

void MacSystemAdapter::ClearError() {
    if (m_macSystemInfo) {
        m_macSystemInfo->ClearError();
    }
}

#endif // PLATFORM_MACOS