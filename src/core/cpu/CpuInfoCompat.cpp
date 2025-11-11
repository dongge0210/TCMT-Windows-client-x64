#include "CpuInfoCompat.h"
#include "Logger.h"

// 包含原有CpuInfo实现
#include "CpuInfo.h"

CpuInfoCompat::CpuInfoCompat(ImplementationMode mode) : m_mode(mode) {
    Initialize();
}

CpuInfoCompat::~CpuInfoCompat() {
    Cleanup();
}

double CpuInfoCompat::GetUsage() {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetUsage();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetUsage();
        }
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get CPU usage: ") + e.what());
    }
    return 0.0;
}

std::string CpuInfoCompat::GetName() {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetName();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetName();
        }
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get CPU name: ") + e.what());
    }
    return "";
}

int CpuInfoCompat::GetTotalCores() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetTotalCores();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetTotalCores();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0;
}

int CpuInfoCompat::GetSmallCores() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetSmallCores();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetSmallCores();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0;
}

int CpuInfoCompat::GetLargeCores() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetLargeCores();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetLargeCores();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0;
}

double CpuCompat::GetLargeCoreSpeed() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetLargeCoreSpeed();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetLargeCoreSpeed();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0.0;
}

double CpuCompat::GetSmallCoreSpeed() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetSmallCoreSpeed();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetSmallCoreSpeed();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0.0;
}

DWORD CpuInfoCompat::GetCurrentSpeed() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetCurrentSpeed();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return static_cast<DWORD>(m_adapterCpuInfo->GetCurrentSpeed());
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0;
}

bool CpuCompat::IsHyperThreadingEnabled() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->IsHyperThreadingEnabled();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->IsHyperThreadingEnabled();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return false;
}

bool CpuCompat::IsVirtualizationEnabled() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->IsVirtualizationEnabled();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->IsVirtualizationEnabled();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return false;
}

double CpuCompat::GetLastSampleIntervalMs() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetLastSampleIntervalMs();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetLastSampleIntervalMs();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0.0;
}

double CpuCompat::GetBaseFrequencyMHz() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetBaseFrequencyMHz();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetBaseFrequencyMHz();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0.0;
}

double CpuCompat::GetCurrentFrequencyMHz() const {
    try {
        if (m_mode == ImplementationMode::Legacy && m_legacyCpuInfo) {
            return m_legacyCpuInfo->GetCurrentFrequencyMHz();
        } else if (m_mode == ImplementationMode::Adapter && m_adapterCpuInfo) {
            return m_adapterCpuInfo->GetCurrentFrequencyMHz();
        }
    } catch (const std::exception& e) {
        // 静默错误，因为这是const方法
    }
    return 0.0;
}

// 跨平台扩展方法
bool CpuInfoCompat::SwitchToAdapter() {
    if (!IsAdapterAvailable()) {
        SetError("Adapter not available on this platform");
        return false;
    }

    try {
        // 尝试创建适配器
        auto testAdapter = InfoFactory::CreateCpuAdapter();
        if (testAdapter && testAdapter->Initialize()) {
            Cleanup();
            m_mode = ImplementationMode::Adapter;
            m_adapterCpuInfo = std::move(testAdapter);
            Logger::Info("Switched to adapter mode for CPU monitoring");
            ClearError();
            return true;
        }
    } catch (const std::exception& e) {
        SetError(std::string("Failed to switch to adapter: ") + e.what());
    }
    
    return false;
}

bool CpuInfoCompat::SwitchToLegacy() {
    try {
        Cleanup();
        m_mode = ImplementationMode::Legacy;
        m_legacyCpuInfo = std::make_unique<CpuInfo>();
        Logger::Info("Switched to legacy mode for CPU monitoring");
        ClearError();
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to switch to legacy: ") + e.what());
    }
    return false;
}

CpuInfoCompat::ImplementationMode CpuInfoCompat::GetCurrentMode() const {
    return m_mode;
}

bool CpuInfoCompat::IsAdapterAvailable() const {
    // 检查当前平台是否支持适配器
    #ifdef PLATFORM_WINDOWS
        return true;  // Windows支持适配器
    #elif defined(PLATFORM_MACOS)
        return false; // macOS适配器待实现
    #elif defined(PLATFORM_LINUX)
        return false; // Linux适配器待实现
    #else
        return false;
    #endif
}

std::string CpuInfoCompat::GetLastError() const {
    return m_lastError;
}

void CpuInfoCompat::ClearError() {
    m_lastError.clear();
}

void CpuInfoCompat::Initialize() {
    ClearError();
    
    try {
        if (m_mode == ImplementationMode::Legacy) {
            m_legacyCpuInfo = std::make_unique<CpuInfo>();
            Logger::Info("Initialized CPU info in legacy mode");
        } else if (m_mode == ImplementationMode::Adapter) {
            if (IsAdapterAvailable()) {
                m_adapterCpuInfo = InfoFactory::CreateCpuAdapter();
                if (m_adapterCpuInfo && m_adapterCpuInfo->Initialize()) {
                    Logger::Info("Initialized CPU info in adapter mode");
                } else {
                    SetError("Failed to initialize adapter");
                    // 回退到legacy模式
                    m_mode = ImplementationMode::Legacy;
                    m_legacyCpuInfo = std::make_unique<CpuInfo>();
                    Logger::Warning("Fallback to legacy mode due to adapter initialization failure");
                }
            } else {
                SetError("Adapter not available on this platform");
                // 回退到legacy模式
                m_mode = ImplementationMode::Legacy;
                m_legacyCpuInfo = std::make_unique<CpuInfo>();
                Logger::Warning("Fallback to legacy mode due to platform limitations");
            }
        }
    } catch (const std::exception& e) {
        SetError(std::string("Initialization failed: ") + e.what());
        // 确保有可用的实现
        if (!m_legacyCpuInfo) {
            try {
                m_legacyCpuInfo = std::make_unique<CpuInfo>();
                m_mode = ImplementationMode::Legacy;
            } catch (...) {
                // 最后的保护
                SetError("Critical: Failed to initialize any CPU info implementation");
            }
        }
    }
}

void CpuInfoCompat::Cleanup() {
    m_legacyCpuInfo.reset();
    m_adapterCpuInfo.reset();
}

void CpuCompat::SetError(const std::string& error) {
    m_lastError = error;
    Logger::Error("CpuInfoCompat error: " + error);
}