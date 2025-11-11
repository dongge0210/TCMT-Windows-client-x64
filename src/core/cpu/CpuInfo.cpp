#include "CpuInfo.h"
#include "Logger.h"
#include <intrin.h>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <vector>
    #include <pdh.h>
    #include <algorithm>
    #include <powrprof.h> // CallNtPowerInformation
    #include <memory>
    #pragma comment(lib, "pdh.lib")
    #pragma comment(lib, "PowrProf.lib")
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <mach/mach.h>
    #include <mach/host_statistics.h>
    #include <mach/mach_host.h>
    #include <sys/utsname.h>
    #include <thread>
    #include <chrono>
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
    #include <thread>
    #include <chrono>
#endif

// Define PDH constants if not available
#ifndef PDH_CSTATUS_VALID_DATA
#define PDH_CSTATUS_VALID_DATA 0x00000000L
#endif
#ifndef PDH_CSTATUS_NEW_DATA
#define PDH_CSTATUS_NEW_DATA 0x00000001L
#endif

static bool QueryBaseAndCurrentFrequencyACPI(double& baseMHz, double& currentMHz)
{
    baseMHz = 0; currentMHz = 0;
    typedef struct _PROCESSOR_POWER_INFORMATION { ULONG Number; ULONG MaxMhz; ULONG CurrentMhz; ULONG MhzLimit; ULONG MaxIdleState; ULONG CurrentIdleState; } PROCESSOR_POWER_INFORMATION;
    SYSTEM_INFO si{}; GetSystemInfo(&si);
    ULONG count = si.dwNumberOfProcessors; if (count == 0 || count > 256) return false;
    std::vector<PROCESSOR_POWER_INFORMATION> buf(count);
    auto st = CallNtPowerInformation(ProcessorInformation, nullptr, 0, buf.data(), (ULONG)(buf.size()*sizeof(PROCESSOR_POWER_INFORMATION)));
    if (st == 0) { double sumBase=0,sumCur=0; int n=0; for (ULONG i=0;i<count;++i){ if (buf[i].MaxMhz>0){ sumBase+=buf[i].MaxMhz; sumCur+=buf[i].CurrentMhz; ++n; } } if (n>0){ baseMHz=sumBase/n; currentMHz=sumCur/n; return true; } }
    return false;
}

static void QueryBaseAndCurrentFrequencyFallback(double& baseMHz, double& currentMHz)
{
    baseMHz = currentMHz = 0;
    HKEY hKey; DWORD speed=0; DWORD size=sizeof(DWORD);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"~MHz", NULL, NULL, (LPBYTE)&speed, &size) == ERROR_SUCCESS) {
            baseMHz = currentMHz = (double)speed;
        }
        RegCloseKey(hKey);
    }
}

void CpuInfo::InitializeFrequencyCounter()
{
    if (freqCounterInitialized) return;
    PDH_STATUS s = PdhOpenQuery(NULL, 0, &freqQueryHandle);
    if (s != ERROR_SUCCESS) { Logger::Warn("无法打开PDH频率查询"); return; }
    // English name to avoid localization: Processor Information(_Total)\\Processor Frequency
    s = PdhAddEnglishCounter(freqQueryHandle, L"\\Processor Information(_Total)\\Processor Frequency", 0, &freqCounterHandle);
    if (s != ERROR_SUCCESS) { Logger::Warn("无法添加频率计数器"); PdhCloseQuery(freqQueryHandle); return; }
    s = PdhCollectQueryData(freqQueryHandle); // prime
    if (s != ERROR_SUCCESS) { Logger::Warn("无法读取频率计数器初值"); PdhCloseQuery(freqQueryHandle); return; }
    freqCounterInitialized = true;
}

void CpuInfo::CleanupFrequencyCounter()
{
    if (freqCounterInitialized) {
        PdhCloseQuery(freqQueryHandle);
        freqCounterInitialized = false;
        freqQueryHandle = nullptr; freqCounterHandle = nullptr;
    }
}

double CpuInfo::updateInstantFrequencyMHz()
{
    // 优先 ACPI
    double base=0, cur=0;
    if (QueryBaseAndCurrentFrequencyACPI(base, cur)) {
        cachedInstantMHz = cur; return cachedInstantMHz;
    }
    // 次选 PDH 频率计数器
    if (!freqCounterInitialized) InitializeFrequencyCounter();
    if (freqCounterInitialized) {
        DWORD now = GetTickCount();
        if (now - lastFreqTick >= 300) { // 300ms 刷新
            lastFreqTick = now;
            PDH_STATUS s = PdhCollectQueryData(freqQueryHandle);
            if (s == ERROR_SUCCESS) {
                PDH_FMT_COUNTERVALUE v{}; s = PdhGetFormattedCounterValue(freqCounterHandle, PDH_FMT_DOUBLE, NULL, &v);
                if (s == ERROR_SUCCESS && (v.CStatus == PDH_CSTATUS_VALID_DATA || v.CStatus == PDH_CSTATUS_NEW_DATA)) {
                    cachedInstantMHz = v.doubleValue; return cachedInstantMHz;
                }
            }
        }
        if (cachedInstantMHz > 0) return cachedInstantMHz;
    }
    // 最后回退注册表
    QueryBaseAndCurrentFrequencyFallback(base, cur);
    cachedInstantMHz = cur; return cachedInstantMHz;
}

CpuInfo::CpuInfo() :
    totalCores(0),
    largeCores(0),
    smallCores(0),
    cpuUsage(0.0),
    counterInitialized(false),
    lastUpdateTime(0),
    lastSampleTick(0),
    prevSampleTick(0),
    lastSampleIntervalMs(0.0) {

    try {
#ifdef PLATFORM_WINDOWS
        DetectCores();
        cpuName = GetNameFromRegistry();
        InitializeCounter();
        UpdateCoreSpeeds();  // 初始化频率信息
        InitializeFrequencyCounter();     // 初始化即时频率计数器
#elif defined(PLATFORM_MACOS)
        DetectCores();
        cpuName = GetNameFromSysctl();
        InitializeCounter();
        UpdateCoreSpeeds();  // 初始化频率信息
        InitializeFrequencyCounter();     // 初始化即时频率计数器
#elif defined(PLATFORM_LINUX)
        DetectCores();
        cpuName = GetNameFromProcCpuinfo();
        InitializeCounter();
        UpdateCoreSpeeds();  // 初始化频率信息
        InitializeFrequencyCounter();     // 初始化即时频率计数器
#endif
    }
    catch (const std::exception& e) {
        Logger::Error("CPU信息初始化失败: " + std::string(e.what()));
    }
}

CpuInfo::~CpuInfo() {
    CleanupFrequencyCounter();
    CleanupCounter();
}

void CpuInfo::InitializeCounter() {
    PDH_STATUS status = PdhOpenQuery(NULL, 0, &queryHandle);
    if (status != ERROR_SUCCESS) {
        Logger::Error("无法打开性能计数器查询");
        return;
    }

    // 使用英文计数器名称以避免本地化问题
    status = PdhAddEnglishCounter(queryHandle,
        L"\\Processor(_Total)\\% Processor Time",
        0,
        &counterHandle);

    if (status != ERROR_SUCCESS) {
        Logger::Error("无法添加CPU使用率计数器");
        PdhCloseQuery(queryHandle);
        return;
    }

    // 首次查询以初始化计数器
    status = PdhCollectQueryData(queryHandle);
    if (status != ERROR_SUCCESS) {
        Logger::Error("无法收集性能计数器数据");
        PdhCloseQuery(queryHandle);
        return;
    }

    counterInitialized = true;
    Logger::Debug("CPU性能计数器初始化完成");
}

double CpuInfo::GetLargeCoreSpeed() const {
    const_cast<CpuInfo*>(this)->UpdateCoreSpeeds();
    if (largeCoresSpeeds.empty()) {
        return GetCurrentSpeed();
    }
    // 计算平均频率
    double total = 0;
    for (DWORD speed : largeCoresSpeeds) {
        total += speed;
    }
    return total / largeCoresSpeeds.size();
}

double CpuInfo::GetSmallCoreSpeed() const {
    const_cast<CpuInfo*>(this)->UpdateCoreSpeeds();
    if (smallCoresSpeeds.empty()) {
        return GetCurrentSpeed();
    }
    // 计算平均频率
    double total = 0;
    for (DWORD speed : smallCoresSpeeds) {
        total += speed;
    }
    return total / smallCoresSpeeds.size();
}

void CpuInfo::CleanupCounter() {
    if (counterInitialized) {
        PdhCloseQuery(queryHandle);
        counterInitialized = false;
    }
}

void CpuInfo::DetectCores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    totalCores = sysInfo.dwNumberOfProcessors;

    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(nullptr, &bufferSize);
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));

    if (GetLogicalProcessorInformation(buffer.data(), &bufferSize)) {
        for (const auto& info : buffer) {
            if (info.Relationship == RelationProcessorCore) {
                (info.ProcessorCore.Flags == 1) ? largeCores++ : smallCores++;
            }
        }
    }
}

void CpuInfo::UpdateCoreSpeeds() {
    // 检查更新间隔
    DWORD currentTime = GetTickCount();
    if (currentTime - lastUpdateTime < 1000) { // 1秒更新一次
        return;
    }
    lastUpdateTime = currentTime;

    HKEY hKey;
    DWORD speed;
    DWORD size = sizeof(DWORD);

    // 清空旧数据
    largeCoresSpeeds.clear();
    smallCoresSpeeds.clear();

    // 遍历所有核心
    for (int i = 0; i < totalCores; ++i) {
        std::wstring keyPath = L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\" + std::to_wstring(i);
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExW(hKey, L"~MHz", NULL, NULL, (LPBYTE)&speed, &size) == ERROR_SUCCESS) {
                // 根据核心类型分类存储频率
                if (i < largeCores * 2) { // 考虑超线程，每个物理核心有两个逻辑核心
                    largeCoresSpeeds.push_back(speed);
                }
                else {
                    smallCoresSpeeds.push_back(speed);
                }
            }
            RegCloseKey(hKey);
        }
    }
}

double CpuInfo::updateUsage() {
    if (!counterInitialized) {
        Logger::Warn("CPU性能计数器未初始化");
        return cpuUsage;
    }

    // 检查时间间隔，确保两次采样之间有足够间隔
    static DWORD lastCollectTime = 0;
    DWORD currentTime = GetTickCount();
    DWORD delta = currentTime - lastCollectTime;
    if (delta < 500) { // 放宽至 500ms
        return cpuUsage; // 返回上次的值
    }

    lastCollectTime = currentTime;

    PDH_STATUS status = PdhCollectQueryData(queryHandle);
    if (status != ERROR_SUCCESS) {
        Logger::Error("无法收集CPU使用率数据，错误代码: " + std::to_string(status));
        return cpuUsage;
    }

    PDH_FMT_COUNTERVALUE counterValue;
    status = PdhGetFormattedCounterValue(counterHandle,
        PDH_FMT_DOUBLE,
        NULL,
        &counterValue);

    if (status != ERROR_SUCCESS) {
        Logger::Error("无法格式化CPU使用率数据，错误代码: " + std::to_string(status));
        return cpuUsage;
    }

    // 记录采样间隔
    prevSampleTick = lastSampleTick;
    lastSampleTick = currentTime;
    if (prevSampleTick != 0) {
        lastSampleIntervalMs = static_cast<double>(lastSampleTick - prevSampleTick);
    }

    // 验证数据有效性
    if (counterValue.CStatus == PDH_CSTATUS_VALID_DATA || counterValue.CStatus == PDH_CSTATUS_NEW_DATA) {
        double newUsage = counterValue.doubleValue;
        if (newUsage < 0.0) newUsage = 0.0;
        if (newUsage > 100.0) newUsage = 100.0;
        if (cpuUsage > 0.0) cpuUsage = (cpuUsage * 0.8) + (newUsage * 0.2); else cpuUsage = newUsage;
        static int updateCounter = 0;
        if (++updateCounter % 60 == 0) {
            Logger::Debug("CPU使用率更新: " + std::to_string(cpuUsage) + "% (采样间隔=" + std::to_string(lastSampleIntervalMs) + "ms)");
        }
    } else {
        Logger::Warn("CPU使用率数据无效，状态: " + std::to_string(counterValue.CStatus));
    }
    return cpuUsage;
}

double CpuInfo::GetUsage() {
    double currentUsage = updateUsage();
    
    // 减少调试信息的频率
    static int debugCounter = 0;
    if (++debugCounter % 30 == 0) { // 每30次调用记录一次（约30秒）
        Logger::Info("CPU使用率: " + std::to_string(currentUsage) + "%");
    }
    
    return currentUsage;
}

std::string CpuInfo::GetNameFromRegistry() {
    HKEY hKey;
    char buffer[128];
    DWORD size = sizeof(buffer);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(buffer, size - 1);
        }
        RegCloseKey(hKey);
    }
    return "Unknown CPU";
}

std::string CpuInfo::GetNameFromSysctl() {
    size_t len = 0;
    sysctlbyname("machdep.cpu.brand_string", nullptr, &len, nullptr, 0);
    if (len > 0) {
        std::string cpuModel;
        cpuModel.resize(len);
        sysctlbyname("machdep.cpu.brand_string", &cpuModel[0], &len, nullptr, 0);
        cpuModel.resize(len - 1); // 移除null终止符
        return cpuModel;
    }
    return "Unknown CPU";
}

std::string CpuInfo::GetNameFromProcCpuinfo() {
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) {
        return "Unknown CPU";
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string cpuModel = line.substr(pos + 1);
                // 移除前后空格
                cpuModel.erase(0, cpuModel.find_first_not_of(' '));
                cpuModel.erase(cpuModel.find_last_not_of(' ') + 1);
                return cpuModel;
            }
        }
    }
    return "Unknown CPU";
}

int CpuInfo::GetTotalCores() const {
    return totalCores;
}

int CpuInfo::GetSmallCores() const {
    return smallCores;
}

int CpuInfo::GetLargeCores() const {
    return largeCores;
}

DWORD CpuInfo::GetCurrentSpeed() const {
    HKEY hKey;
    DWORD speed = 0;
    DWORD size = sizeof(DWORD);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"~MHz", NULL, NULL, (LPBYTE)&speed, &size);
        RegCloseKey(hKey);
    }
    return speed;
}

double CpuInfo::GetBaseFrequencyMHz() const {
    double base=0, cur=0;
    if (!QueryBaseAndCurrentFrequencyACPI(base, cur)) {
        QueryBaseAndCurrentFrequencyFallback(base, cur);
    }
    return base;
}

double CpuInfo::GetCurrentFrequencyMHz() const {
    return const_cast<CpuInfo*>(this)->updateInstantFrequencyMHz();
}

std::string CpuInfo::GetName() {
    return cpuName;
}

bool CpuInfo::IsHyperThreadingEnabled() const {
    return (totalCores > (largeCores + smallCores));
}

bool CpuInfo::IsVirtualizationEnabled() const {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    bool hasVMX = (cpuInfo[2] & (1 << 5)) != 0;

    if (!hasVMX) return false;

    bool isVMXEnabled = false;
    __try {
        unsigned __int64 msrValue = __readmsr(0x3A);
        isVMXEnabled = (msrValue & 0x5) != 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        isVMXEnabled = false;
    }

    return isVMXEnabled;
}

// WinCpuAdapter 实现
WinCpuAdapter::WinCpuAdapter() {
    m_originalCpuInfo = std::make_unique<CpuInfo>();
}

WinCpuAdapter::~WinCpuAdapter() {
    Cleanup();
}

double WinCpuAdapter::GetUsage() {
    if (!m_originalCpuInfo) return 0.0;
    return m_originalCpuInfo->GetUsage();
}

std::string WinCpuAdapter::GetName() {
    if (!m_originalCpuInfo) return "";
    return m_originalCpuInfo->GetName();
}

int WinCpuAdapter::GetTotalCores() const {
    if (!m_originalCpuInfo) return 0;
    return m_originalCpuInfo->GetTotalCores();
}

int WinCpuAdapter::GetSmallCores() const {
    if (!m_originalCpuInfo) return 0;
    return m_originalCpuInfo->GetSmallCores();
}

int WinCpuAdapter::GetLargeCores() const {
    if (!m_originalCpuInfo) return 0;
    return m_originalCpuInfo->GetLargeCores();
}

double WinCpuAdapter::GetLargeCoreSpeed() const {
    if (!m_originalCpuInfo) return 0.0;
    return m_originalCpuInfo->GetLargeCoreSpeed();
}

double WinCpuAdapter::GetSmallCoreSpeed() const {
    if (!m_originalCpuInfo) return 0.0;
    return m_originalCpuInfo->GetSmallCoreSpeed();
}

DWORD WinCpuAdapter::GetCurrentSpeed() const {
    if (!m_originalCpuInfo) return 0;
    return m_originalCpuInfo->GetCurrentSpeed();
}

bool WinCpuAdapter::IsHyperThreadingEnabled() const {
    if (!m_originalCpuInfo) return false;
    return m_originalCpuInfo->IsHyperThreadingEnabled();
}

bool WinCpuAdapter::IsVirtualizationEnabled() const {
    if (!m_originalCpuInfo) return false;
    return m_originalCpuInfo->IsVirtualizationEnabled();
}

double WinCpuAdapter::GetLastSampleIntervalMs() const {
    if (!m_originalCpuInfo) return 0.0;
    return m_originalCpuInfo->GetLastSampleIntervalMs();
}

double WinCpuAdapter::GetBaseFrequencyMHz() const {
    if (!m_originalCpuInfo) return 0.0;
    return m_originalCpuInfo->GetBaseFrequencyMHz();
}

double WinCpuAdapter::GetCurrentFrequencyMHz() const {
    if (!m_originalCpuInfo) return 0.0;
    return m_originalCpuInfo->GetCurrentFrequencyMHz();
}

bool WinCpuAdapter::Initialize() {
    // CpuInfo类在构造时自动初始化，这里只需要确认对象存在
    return m_originalCpuInfo != nullptr;
}

void WinCpuAdapter::Cleanup() {
    // CpuInfo类在析构时自动清理，这里不需要额外操作
}

bool WinCpuAdapter::Update() {
    // CpuInfo类在每次调用Get方法时自动更新数据
    // 这里可以添加额外的更新逻辑，如果需要的话
    return m_originalCpuInfo != nullptr;
}
