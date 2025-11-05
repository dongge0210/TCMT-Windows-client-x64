#include "WinCpuInfo.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

// 包含Windows特定头文件
#include <winreg.h>
#include <intrin.h>
#include <wmiutils.h>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "advapi32.lib")

WinCpuInfo::WinCpuInfo()
    : m_queryHandle(nullptr)
    , m_totalCounter(nullptr)
    , m_freqQueryHandle(nullptr)
    , m_freqCounter(nullptr)
    , m_pdhInitialized(false)
    , m_freqInitialized(false)
    , m_tempInitialized(false)
    , m_powerInitialized(false)
    , m_totalCores(0)
    , m_logicalCores(0)
    , m_physicalCores(0)
    , m_performanceCores(0)
    , m_efficiencyCores(0)
    , m_hasHybridArchitecture(false)
    , m_totalUsage(0.0)
    , m_performanceCoreUsage(0.0)
    , m_efficiencyCoreUsage(0.0)
    , m_currentFrequency(0.0)
    , m_baseFrequency(0.0)
    , m_maxFrequency(0.0)
    , m_isHyperThreadingEnabled(false)
    , m_isVirtualizationEnabled(false)
    , m_supportsAVX(false)
    , m_supportsAVX2(false)
    , m_supportsAVX512(false)
    , m_temperature(0.0)
    , m_powerUsage(0.0)
    , m_packagePower(0.0)
    , m_corePower(0.0)
    , m_initialized(false)
    , m_dataValid(false)
    , m_lastUpdateTime(0) {
}

WinCpuInfo::~WinCpuInfo() {
    Cleanup();
}

bool WinCpuInfo::Initialize() {
    if (m_initialized) {
        return true;
    }

    try {
        // 初始化CPU基本信息
        if (!InitializeCpuInfo()) {
            SetError("Failed to initialize CPU basic information");
            return false;
        }

        // 初始化PDH计数器
        if (!InitializePdhCounters()) {
            SetError("Failed to initialize PDH counters");
            return false;
        }

        // 初始化频率计数器
        if (!InitializeFrequencyCounters()) {
            SetError("Failed to initialize frequency counters");
            return false;
        }

        // 初始化温度监控
        InitializeTemperatureMonitoring();

        // 初始化功耗监控
        InitializePowerMonitoring();

        m_initialized = true;
        ClearError();
        return true;
    }
    catch (const std::exception& e) {
        SetError(std::string("Initialization failed: ") + e.what());
        return false;
    }
}

void WinCpuInfo::Cleanup() {
    if (!m_initialized) {
        return;
    }

    CleanupPdhCounters();
    CleanupFrequencyCounters();
    CleanupTemperatureMonitoring();
    CleanupPowerMonitoring();

    m_initialized = false;
    m_dataValid = false;
}

bool WinCpuInfo::IsInitialized() const {
    return m_initialized;
}

bool WinCpuInfo::Update() {
    if (!m_initialized) {
        SetError("Not initialized");
        return false;
    }

    try {
        bool success = true;

        // 更新CPU使用率
        if (!UpdateCpuUsage()) {
            success = false;
        }

        // 更新CPU频率
        if (!UpdateCpuFrequency()) {
            success = false;
        }

        // 更新温度
        if (m_tempInitialized && !UpdateCpuTemperature()) {
            success = false;
        }

        // 更新功耗
        if (m_powerInitialized && !UpdateCpuPower()) {
            success = false;
        }

        if (success) {
            m_dataValid = true;
            m_lastUpdateTime = GetTickCount64();
            ClearError();
        }

        return success;
    }
    catch (const std::exception& e) {
        SetError(std::string("Update failed: ") + e.what());
        return false;
    }
}

bool WinCpuInfo::IsDataValid() const {
    return m_dataValid;
}

uint64_t WinCpuInfo::GetLastUpdateTime() const {
    return m_lastUpdateTime;
}

std::string WinCpuInfo::GetLastError() const {
    return m_lastError;
}

void WinCpuInfo::ClearError() {
    m_lastError.clear();
}

// ICpuInfo接口实现
std::string WinCpuInfo::GetName() const {
    return m_name;
}

std::string WinCpuInfo::GetVendor() const {
    return m_vendor;
}

std::string WinCpuInfo::GetArchitecture() const {
    return m_architecture;
}

uint32_t WinCpuInfo::GetTotalCores() const {
    return m_totalCores;
}

uint32_t WinCpuInfo::GetLogicalCores() const {
    return m_logicalCores;
}

uint32_t WinCpuInfo::GetPhysicalCores() const {
    return m_physicalCores;
}

uint32_t WinCpuInfo::GetPerformanceCores() const {
    return m_performanceCores;
}

uint32_t WinCpuInfo::GetEfficiencyCores() const {
    return m_efficiencyCores;
}

bool WinCpuInfo::HasHybridArchitecture() const {
    return m_hasHybridArchitecture;
}

double WinCpuInfo::GetTotalUsage() const {
    return m_totalUsage;
}

std::vector<double> WinCpuInfo::GetPerCoreUsage() const {
    return m_perCoreUsage;
}

double WinCpuInfo::GetPerformanceCoreUsage() const {
    return m_performanceCoreUsage;
}

double WinCpuInfo::GetEfficiencyCoreUsage() const {
    return m_efficiencyCoreUsage;
}

double WinCpuInfo::GetCurrentFrequency() const {
    return m_currentFrequency;
}

double WinCpuInfo::GetBaseFrequency() const {
    return m_baseFrequency;
}

double WinCpuInfo::GetMaxFrequency() const {
    return m_maxFrequency;
}

std::vector<double> WinCpuInfo::GetPerCoreFrequencies() const {
    return m_perCoreFrequencies;
}

bool WinCpuInfo::IsHyperThreadingEnabled() const {
    return m_isHyperThreadingEnabled;
}

bool WinCpuInfo::IsVirtualizationEnabled() const {
    return m_isVirtualizationEnabled;
}

bool WinCpuInfo::SupportsAVX() const {
    return m_supportsAVX;
}

bool WinCpuInfo::SupportsAVX2() const {
    return m_supportsAVX2;
}

bool WinCpuInfo::SupportsAVX512() const {
    return m_supportsAVX512;
}

double WinCpuInfo::GetTemperature() const {
    return m_temperature;
}

std::vector<double> WinCpuInfo::GetPerCoreTemperatures() const {
    return m_perCoreTemperatures;
}

double WinCpuInfo::GetPowerUsage() const {
    return m_powerUsage;
}

double WinCpuInfo::GetPackagePower() const {
    return m_packagePower;
}

double WinCpuInfo::GetCorePower() const {
    return m_corePower;
}

// 私有方法实现
bool WinCpuInfo::InitializeCpuInfo() {
    // 获取CPU名称
    m_name = GetNameFromRegistry();
    if (m_name.empty()) {
        m_name = GetCpuBrandString();
    }

    // 获取供应商信息
    m_vendor = GetVendorFromCpuid();

    // 获取架构信息
    m_architecture = GetArchitectureFromSystem();

    // 检测核心信息
    if (!DetectCores()) {
        return false;
    }

    // 检测混合架构
    DetectHybridArchitecture();

    // 检测指令集支持
    DetectInstructionSets();

    // 检测虚拟化支持
    DetectVirtualizationSupport();

    return true;
}

bool WinCpuInfo::InitializePdhCounters() {
    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &m_queryHandle);
    if (status != ERROR_SUCCESS) {
        SetError("Failed to open PDH query");
        return false;
    }

    // 添加总CPU使用率计数器
    status = PdhAddCounter(m_queryHandle, L"\\Processor(_Total)\\% Processor Time", 0, &m_totalCounter);
    if (status != ERROR_SUCCESS) {
        PdhCloseQuery(m_queryHandle);
        m_queryHandle = nullptr;
        SetError("Failed to add total CPU counter");
        return false;
    }

    // 添加每个核心的使用率计数器
    m_perCoreCounters.resize(m_logicalCores, nullptr);
    for (uint32_t i = 0; i < m_logicalCores; ++i) {
        std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
        status = PdhAddCounter(m_queryHandle, counterPath.c_str(), 0, &m_perCoreCounters[i]);
        if (status != ERROR_SUCCESS) {
            // 如果某个核心计数器添加失败，记录警告但继续
            m_perCoreCounters[i] = nullptr;
        }
    }

    // 第一次查询以初始化数据
    PdhCollectQueryData(m_queryHandle);

    m_pdhInitialized = true;
    return true;
}

bool WinCpuInfo::InitializeFrequencyCounters() {
    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &m_freqQueryHandle);
    if (status != ERROR_SUCCESS) {
        SetError("Failed to open frequency query");
        return false;
    }

    // 添加CPU频率计数器
    status = PdhAddCounter(m_freqQueryHandle, L"\\Processor Information(_Total)\\Processor Frequency", 0, &m_freqCounter);
    if (status != ERROR_SUCCESS) {
        // 尝试使用注册表方法
        PdhCloseQuery(m_freqQueryHandle);
        m_freqQueryHandle = nullptr;
        if (!GetFrequencyFromRegistry(m_baseFrequency)) {
            SetError("Failed to initialize frequency monitoring");
            return false;
        }
        m_maxFrequency = m_baseFrequency;
        return true;
    }

    // 添加每个核心的频率计数器
    m_perCoreFreqCounters.resize(m_logicalCores, nullptr);
    for (uint32_t i = 0; i < m_logicalCores; ++i) {
        std::wstring counterPath = L"\\Processor Information(" + std::to_wstring(i) + L")\\Processor Frequency";
        status = PdhAddCounter(m_freqQueryHandle, counterPath.c_str(), 0, &m_perCoreFreqCounters[i]);
        if (status != ERROR_SUCCESS) {
            m_perCoreFreqCounters[i] = nullptr;
        }
    }

    // 第一次查询以初始化数据
    PdhCollectQueryData(m_freqQueryHandle);

    m_freqInitialized = true;
    return true;
}

void WinCpuInfo::CleanupPdhCounters() {
    if (m_queryHandle) {
        PdhCloseQuery(m_queryHandle);
        m_queryHandle = nullptr;
    }
    m_totalCounter = nullptr;
    m_perCoreCounters.clear();
    m_pdhInitialized = false;
}

void WinCpuInfo::CleanupFrequencyCounters() {
    if (m_freqQueryHandle) {
        PdhCloseQuery(m_freqQueryHandle);
        m_freqQueryHandle = nullptr;
    }
    m_freqCounter = nullptr;
    m_perCoreFreqCounters.clear();
    m_freqInitialized = false;
}

void WinCpuInfo::CleanupTemperatureMonitoring() {
    m_tempInitialized = false;
}

void WinCpuInfo::CleanupPowerMonitoring() {
    m_powerInitialized = false;
}

bool WinCpuInfo::UpdateCpuUsage() {
    if (!m_pdhInitialized) {
        return false;
    }

    PDH_STATUS status = PdhCollectQueryData(m_queryHandle);
    if (status != ERROR_SUCCESS) {
        return false;
    }

    // 更新总CPU使用率
    if (m_totalCounter) {
        m_totalUsage = GetCounterValue(m_totalCounter);
    }

    // 更新每个核心的使用率
    m_perCoreUsage.resize(m_logicalCores, 0.0);
    for (uint32_t i = 0; i < m_logicalCores; ++i) {
        if (m_perCoreCounters[i]) {
            m_perCoreUsage[i] = GetCounterValue(m_perCoreCounters[i]);
        }
    }

    // 计算性能核心和能效核心的使用率
    if (m_hasHybridArchitecture) {
        double perfSum = 0.0, effSum = 0.0;
        uint32_t perfCount = 0, effCount = 0;

        for (uint32_t i = 0; i < m_logicalCores; ++i) {
            if (i < m_performanceCores) {
                perfSum += m_perCoreUsage[i];
                perfCount++;
            } else {
                effSum += m_perCoreUsage[i];
                effCount++;
            }
        }

        m_performanceCoreUsage = (perfCount > 0) ? (perfSum / perfCount) : 0.0;
        m_efficiencyCoreUsage = (effCount > 0) ? (effSum / effCount) : 0.0;
    }

    return true;
}

bool WinCpuInfo::UpdateCpuFrequency() {
    if (!m_freqInitialized) {
        return false;
    }

    PDH_STATUS status = PdhCollectQueryData(m_freqQueryHandle);
    if (status != ERROR_SUCCESS) {
        return false;
    }

    // 更新CPU频率
    if (m_freqCounter) {
        m_currentFrequency = GetCounterValue(m_freqCounter);
    }

    // 更新每个核心的频率
    m_perCoreFrequencies.resize(m_logicalCores, 0.0);
    for (uint32_t i = 0; i < m_logicalCores; ++i) {
        if (m_perCoreFreqCounters[i]) {
            m_perCoreFrequencies[i] = GetCounterValue(m_perCoreFreqCounters[i]);
        }
    }

    return true;
}

bool WinCpuInfo::UpdateCpuTemperature() {
    return GetTemperatureFromWmi(m_temperature);
}

bool WinCpuInfo::UpdateCpuPower() {
    return GetPowerFromWmi(m_powerUsage, m_packagePower, m_corePower);
}

std::string WinCpuInfo::GetNameFromRegistry() {
    std::string result;
    ReadRegistryValue(
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        L"ProcessorNameString",
        result
    );
    return result;
}

std::string WinCpuInfo::GetVendorFromCpuid() {
    uint32_t eax, ebx, ecx, edx;
    
    if (GetCpuidInfo(eax, ebx, ecx, edx, 0)) {
        // 检查供应商字符串
        char vendor[13] = {0};
        memcpy(&vendor[0], &ebx, 4);
        memcpy(&vendor[4], &edx, 4);
        memcpy(&vendor[8], &ecx, 4);
        
        std::string vendorStr(vendor);
        
        if (vendorStr == "GenuineIntel") {
            return "Intel";
        } else if (vendorStr == "AuthenticAMD") {
            return "AMD";
        } else if (vendorStr == "CentaurHauls") {
            return "Centaur";
        } else {
            return vendorStr;
        }
    }
    
    return "Unknown";
}

std::string WinCpuInfo::GetArchitectureFromSystem() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            return "x64";
        case PROCESSOR_ARCHITECTURE_INTEL:
            return "x86";
        case PROCESSOR_ARCHITECTURE_ARM64:
            return "ARM64";
        case PROCESSOR_ARCHITECTURE_ARM:
            return "ARM";
        default:
            return "Unknown";
    }
}

bool WinCpuInfo::DetectCores() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    m_logicalCores = si.dwNumberOfProcessors;
    
    // 获取物理核心数
    uint32_t physicalCores = 0;
    if (ReadRegistryDword(
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        L"~MHz",
        physicalCores)) {
        // 这里需要更复杂的逻辑来获取物理核心数
        // 暂时使用逻辑核心数的一半作为物理核心数的估计
        m_physicalCores = (m_logicalCores + 1) / 2;
    } else {
        m_physicalCores = m_logicalCores;
    }
    
    m_totalCores = m_logicalCores;
    
    return true;
}

bool WinCpuInfo::DetectHybridArchitecture() {
    // 检查是否支持混合架构（Intel大小核）
    // 这里可以通过检查特定的CPU型号或使用WMI来检测
    // 暂时简化实现
    m_hasHybridArchitecture = false;
    
    // 可以通过检查CPU型号来判断
    if (m_name.find("Intel") != std::string::npos) {
        // Intel 12代及以后支持大小核架构
        if (m_name.find("12000") != std::string::npos ||
            m_name.find("13000") != std::string::npos ||
            m_name.find("14000") != std::string::npos) {
            m_hasHybridArchitecture = true;
            m_performanceCores = m_physicalCores / 2; // 简化假设
            m_efficiencyCores = m_physicalCores - m_performanceCores;
        }
    }
    
    return true;
}

bool WinCpuInfo::DetectInstructionSets() {
    uint32_t eax, ebx, ecx, edx;
    
    // 检查AVX支持
    if (GetCpuidInfo(eax, ebx, ecx, edx, 1)) {
        m_supportsAVX = (ecx & (1 << 28)) != 0;
    }
    
    // 检查AVX2支持
    if (GetCpuidInfo(eax, ebx, ecx, edx, 7)) {
        m_supportsAVX2 = (ebx & (1 << 5)) != 0;
    }
    
    // 检查AVX512支持
    if (GetCpuidInfo(eax, ebx, ecx, edx, 7)) {
        m_supportsAVX512 = (ebx & (1 << 16)) != 0;
    }
    
    return true;
}

bool WinCpuInfo::DetectVirtualizationSupport() {
    uint32_t eax, ebx, ecx, edx;
    
    if (GetCpuidInfo(eax, ebx, ecx, edx, 1)) {
        m_isVirtualizationEnabled = (ecx & (1 << 5)) != 0; // VMX bit
    }
    
    return true;
}

void WinCpuInfo::SetError(const std::string& error) {
    m_lastError = error;
}

bool WinCpuInfo::ReadRegistryValue(const std::string& keyPath, const std::string& valueName, std::string& result) {
    HKEY hKey;
    LONG status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey);
    if (status != ERROR_SUCCESS) {
        return false;
    }
    
    DWORD dataType;
    DWORD dataSize = 0;
    status = RegQueryValueExA(hKey, valueName.c_str(), nullptr, &dataType, nullptr, &dataSize);
    if (status != ERROR_SUCCESS || dataType != REG_SZ) {
        RegCloseKey(hKey);
        return false;
    }
    
    std::vector<char> buffer(dataSize);
    status = RegQueryValueExA(hKey, valueName.c_str(), nullptr, &dataType, 
                           reinterpret_cast<LPBYTE>(buffer.data()), &dataSize);
    RegCloseKey(hKey);
    
    if (status == ERROR_SUCCESS) {
        result = std::string(buffer.data());
        return true;
    }
    
    return false;
}

bool WinCpuInfo::ReadRegistryDword(const std::string& keyPath, const std::string& valueName, uint32_t& result) {
    HKEY hKey;
    LONG status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey);
    if (status != ERROR_SUCCESS) {
        return false;
    }
    
    DWORD dataType;
    DWORD dataSize = sizeof(DWORD);
    DWORD value = 0;
    status = RegQueryValueExA(hKey, valueName.c_str(), nullptr, &dataType, 
                           reinterpret_cast<LPBYTE>(&value), &dataSize);
    RegCloseKey(hKey);
    
    if (status == ERROR_SUCCESS && dataType == REG_DWORD) {
        result = value;
        return true;
    }
    
    return false;
}

bool WinCpuInfo::GetCpuidInfo(uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx, uint32_t leaf) {
#ifdef _MSC_VER
    int cpuInfo[4];
    __cpuid(cpuInfo, leaf);
    eax = cpuInfo[0];
    ebx = cpuInfo[1];
    ecx = cpuInfo[2];
    edx = cpuInfo[3];
    return true;
#else
    // 对于其他编译器，可能需要不同的实现
    return false;
#endif
}

double WinCpuInfo::GetCounterValue(PDH_HCOUNTER counter) {
    if (!counter) {
        return 0.0;
    }
    
    PDH_FMT_COUNTERVALUE counterValue;
    PDH_STATUS status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counterValue);
    if (status == ERROR_SUCCESS) {
        return counterValue.doubleValue;
    }
    
    return 0.0;
}

bool WinCpuInfo::GetTemperatureFromWmi(double& temperature) {
    // 这里需要实现WMI查询来获取CPU温度
    // 暂时返回false，表示不可用
    return false;
}

bool WinCpuInfo::GetPowerFromWmi(double& power, double& packagePower, double& corePower) {
    // 这里需要实现WMI查询来获取CPU功耗
    // 暂时返回false，表示不可用
    return false;
}

bool WinCpuInfo::GetFrequencyFromRegistry(double& frequency) {
    // 从注册表获取CPU基础频率
    uint32_t mhz = 0;
    if (ReadRegistryDword(
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        L"~MHz",
        mhz)) {
        frequency = static_cast<double>(mhz);
        return true;
    }
    
    return false;
}

std::string WinCpuInfo::GetCpuBrandString() {
    uint32_t eax, ebx, ecx, edx;
    std::string brand;
    
    // 获取扩展CPU信息
    for (uint32_t i = 0x80000002; i <= 0x80000004; ++i) {
        if (GetCpuidInfo(eax, ebx, ecx, edx, i)) {
            char part[17] = {0};
            memcpy(part, &eax, 4);
            memcpy(part + 4, &ebx, 4);
            memcpy(part + 8, &ecx, 4);
            memcpy(part + 12, &edx, 4);
            brand += part;
        }
    }
    
    // 移除末尾的空格
    while (!brand.empty() && brand.back() == ' ') {
        brand.pop_back();
    }
    
    return brand;
}