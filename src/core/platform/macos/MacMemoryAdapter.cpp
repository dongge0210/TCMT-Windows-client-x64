#include "MacMemoryAdapter.h"
#include "../../Utils/Logger.h"

#ifdef PLATFORM_MACOS

MacMemoryAdapter::MacMemoryAdapter() {
    m_macMemoryInfo = std::make_unique<MacMemoryInfo>();
    if (!m_macMemoryInfo || !m_macMemoryInfo->Initialize()) {
        Logger::Error("Failed to initialize MacMemoryInfo");
        SetError("Failed to initialize MacMemoryInfo");
        m_macMemoryInfo.reset();
    }
}

MacMemoryAdapter::~MacMemoryAdapter() {
    Cleanup();
}

uint64_t MacMemoryAdapter::GetTotalPhysical() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetTotalPhysicalMemory();
}

uint64_t MacMemoryAdapter::GetAvailablePhysical() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetAvailablePhysicalMemory();
}

uint64_t MacMemoryAdapter::GetTotalVirtual() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetTotalVirtualMemory();
}

uint64_t MacMemoryAdapter::GetAvailableVirtual() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetAvailableVirtualMemory();
}

double MacMemoryAdapter::GetPhysicalUsagePercentage() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0.0;
    }
    return m_macMemoryInfo->GetPhysicalMemoryUsage();
}

double MacMemoryAdapter::GetVirtualUsagePercentage() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0.0;
    }
    return m_macMemoryInfo->GetVirtualMemoryUsage();
}

uint64_t MacMemoryAdapter::GetUsedPhysical() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetUsedPhysicalMemory();
}

uint64_t MacMemoryAdapter::GetUsedVirtual() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetUsedVirtualMemory();
}

uint64_t MacMemoryAdapter::GetTotalSwap() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetTotalSwapMemory();
}

uint64_t MacMemoryAdapter::GetAvailableSwap() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetAvailableSwapMemory();
}

uint64_t MacMemoryAdapter::GetUsedSwap() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetUsedSwapMemory();
}

double MacMemoryAdapter::GetSwapUsagePercentage() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0.0;
    }
    return m_macMemoryInfo->GetSwapMemoryUsage();
}

double MacMemoryAdapter::GetMemorySpeed() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0.0;
    }
    return m_macMemoryInfo->GetMemorySpeed();
}

std::string MacMemoryAdapter::GetMemoryType() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return "Unknown";
    }
    return m_macMemoryInfo->GetMemoryType();
}

uint32_t MacMemoryAdapter::GetMemoryChannels() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetMemoryChannels();
}

uint64_t MacMemoryAdapter::GetCachedMemory() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetCachedMemory();
}

uint64_t MacMemoryAdapter::GetBufferedMemory() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetBufferedMemory();
}

uint64_t MacMemoryAdapter::GetSharedMemory() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0;
    }
    return m_macMemoryInfo->GetSharedMemory();
}

double MacMemoryAdapter::GetMemoryPressure() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return 0.0;
    }
    return m_macMemoryInfo->GetMemoryPressure();
}

bool MacMemoryAdapter::IsMemoryLow() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return false;
    }
    return m_macMemoryInfo->IsMemoryLow();
}

bool MacMemoryAdapter::IsMemoryCritical() const {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return false;
    }
    return m_macMemoryInfo->IsMemoryCritical();
}

bool MacMemoryAdapter::Initialize() {
    if (!m_macMemoryInfo) {
        m_macMemoryInfo = std::make_unique<MacMemoryInfo>();
    }
    
    if (m_macMemoryInfo && m_macMemoryInfo->Initialize()) {
        m_lastError.clear();
        return true;
    } else {
        SetError("Failed to initialize MacMemoryInfo");
        return false;
    }
}

void MacMemoryAdapter::Cleanup() {
    if (m_macMemoryInfo) {
        m_macMemoryInfo->Cleanup();
        m_macMemoryInfo.reset();
    }
}

bool MacMemoryAdapter::Update() {
    if (!m_macMemoryInfo) {
        SetError("MacMemoryInfo not initialized");
        return false;
    }
    
    if (m_macMemoryInfo->Update()) {
        m_lastError.clear();
        return true;
    } else {
        SetError("Failed to update MacMemoryInfo");
        return false;
    }
}

std::string MacMemoryAdapter::GetLastError() const {
    return m_lastError;
}

void MacMemoryAdapter::SetError(const std::string& error) const {
    m_lastError = error;
}

#endif // PLATFORM_MACOS