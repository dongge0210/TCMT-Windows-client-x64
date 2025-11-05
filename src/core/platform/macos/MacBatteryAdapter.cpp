#include "MacBatteryAdapter.h"
#include "MacBatteryInfo.h"
#include "../../Utils/Logger.h"

#ifdef PLATFORM_MACOS

MacBatteryAdapter::MacBatteryAdapter() {
    m_macBatteryInfo = std::make_unique<MacBatteryInfo>();
    if (!m_macBatteryInfo || !m_macBatteryInfo->Initialize()) {
        Logger::Error("Failed to initialize MacBatteryInfo");
        m_macBatteryInfo.reset();
    }
}

MacBatteryAdapter::~MacBatteryAdapter() {
    Cleanup();
}

bool MacBatteryAdapter::Initialize() {
    return m_macBatteryInfo != nullptr && m_macBatteryInfo->IsInitialized();
}

void MacBatteryAdapter::Cleanup() {
    if (m_macBatteryInfo) {
        m_macBatteryInfo->Cleanup();
        m_macBatteryInfo.reset();
    }
}

bool MacBatteryAdapter::IsInitialized() const {
    return m_macBatteryInfo != nullptr && m_macBatteryInfo->IsInitialized();
}

bool MacBatteryAdapter::Update() {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->Update();
}

bool MacBatteryAdapter::IsDataValid() const {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->IsDataValid();
}

bool MacBatteryAdapter::IsBatteryPresent() const {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->IsBatteryPresent();
}

bool MacBatteryAdapter::IsCharging() const {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->IsCharging();
}

bool MacBatteryAdapter::IsACPowered() const {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->IsACPowered();
}

std::string MacBatteryAdapter::GetBatteryType() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetBatteryType();
}

std::string MacBatteryAdapter::GetBatteryModel() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetBatteryModel();
}

std::string MacBatteryAdapter::GetBatteryManufacturer() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetBatteryManufacturer();
}

std::string MacBatteryAdapter::GetBatterySerialNumber() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetBatterySerialNumber();
}

uint32_t MacBatteryAdapter::GetCurrentCapacity() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetCurrentCapacity();
}

uint32_t MacBatteryAdapter::GetMaxCapacity() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetMaxCapacity();
}

uint32_t MacBatteryAdapter::GetDesignCapacity() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetDesignCapacity();
}

uint32_t MacBatteryAdapter::GetNominalCapacity() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetNominalCapacity();
}

double MacBatteryAdapter::GetChargePercentage() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetChargePercentage();
}

double MacBatteryAdapter::GetHealthPercentage() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetHealthPercentage();
}

double MacBatteryAdapter::GetDesignHealthPercentage() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetDesignHealthPercentage();
}

double MacBatteryAdapter::GetVoltage() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetVoltage();
}

double MacBatteryAdapter::GetAmperage() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetAmperage();
}

double MacBatteryAdapter::GetWattage() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetWattage();
}

uint32_t MacBatteryAdapter::GetTimeToFullCharge() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetTimeToFullCharge();
}

uint32_t MacBatteryAdapter::GetTimeToEmpty() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetTimeToEmpty();
}

uint32_t MacBatteryAdapter::GetTimeRemaining() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetTimeRemaining();
}

uint32_t MacBatteryAdapter::GetCycleCount() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetCycleCount();
}

uint32_t MacBatteryAdapter::GetCycleCountLimit() const {
    if (!m_macBatteryInfo) return 0;
    return m_macBatteryInfo->GetCycleCountLimit();
}

double MacBatteryAdapter::GetCycleCountPercentage() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetCycleCountPercentage();
}

double MacBatteryAdapter::GetTemperature() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetTemperature();
}

std::vector<BatteryCell> MacBatteryAdapter::GetCellInfo() const {
    if (!m_macBatteryInfo) return std::vector<BatteryCell>();
    return m_macBatteryInfo->GetCellInfo();
}

std::string MacBatteryAdapter::GetPowerSourceState() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetPowerSourceState();
}

bool MacBatteryAdapter::IsPowerSavingMode() const {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->IsPowerSavingMode();
}

bool MacBatteryAdapter::IsOptimizedBatteryCharging() const {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->IsOptimizedBatteryCharging();
}

std::string MacBatteryAdapter::GetChargingState() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetChargingState();
}

bool MacBatteryAdapter::IsBatteryHealthy() const {
    if (!m_macBatteryInfo) return false;
    return m_macBatteryInfo->IsBatteryHealthy();
}

std::string MacBatteryAdapter::GetBatteryHealthStatus() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetBatteryHealthStatus();
}

std::vector<std::string> MacBatteryAdapter::GetWarnings() const {
    if (!m_macBatteryInfo) return std::vector<std::string>();
    return m_macBatteryInfo->GetWarnings();
}

std::vector<std::string> MacBatteryAdapter::GetErrors() const {
    if (!m_macBatteryInfo) return std::vector<std::string>();
    return m_macBatteryInfo->GetErrors();
}

double MacBatteryAdapter::GetEstimatedRuntime() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetEstimatedRuntime();
}

double MacBatteryAdapter::GetEstimatedChargingTime() const {
    if (!m_macBatteryInfo) return 0.0;
    return m_macBatteryInfo->GetEstimatedChargingTime();
}

std::string MacBatteryAdapter::GetLastError() const {
    if (!m_macBatteryInfo) return "";
    return m_macBatteryInfo->GetLastError();
}

void MacBatteryAdapter::ClearError() {
    if (m_macBatteryInfo) {
        m_macBatteryInfo->ClearError();
    }
}

#endif // PLATFORM_MACOS