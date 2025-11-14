// 跨平台主程序文件
// 支持 Windows、macOS 和 Linux 平台

#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <thread>
#include <algorithm>
#include <vector>
#include <mutex>
#include <atomic>
#include <locale>
#include <new>
#include <stdexcept>
#include <csignal>
#include <cstring>

// 平台特定头文件
#ifdef PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <shellapi.h>
    #include <sddl.h>
    #include <Aclapi.h>
    #include <conio.h>
    #include <eh.h>
    #include <io.h>
    #include <fcntl.h>
#elif defined(PLATFORM_MACOS) || defined(PLATFORM_LINUX)
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <signal.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <locale.h>
#endif

// 项目头文件
#include "core/common/PlatformDefs.h"
#include "core/cpu/CpuInfo.h"
#include "core/gpu/GpuInfo.h"
#include "core/memory/MemoryInfo.h"
#include "core/network/NetworkAdapter.h"
#include "core/os/OSInfo.h"
#include "core/utils/Logger.h"
#include "core/utils/TimeUtils.h"
#include "core/DataStruct/DataStruct.h"
#include "core/DataStruct/SharedMemoryManager.h"
#include "core/temperature/TemperatureWrapper.h"
#include "core/tpm/TpmInfo.h"
#include "core/tpm/TpmInfoEnhanced.h"
#include "core/tpm/CrossPlatformTpmInfo.h"
#include "core/usb/USBInfo.h"

#ifdef PLATFORM_WINDOWS
    #include "core/utils/WinUtils.h"
    #include "core/Utils/WMIManager.h"
    #include "core/utils/MotherboardInfo.h"
    #pragma comment(lib, "kernel32.lib")
    #pragma comment(lib, "user32.lib")
#endif

// 全局变量
std::atomic<bool> g_shouldExit{false};
static std::atomic<bool> g_monitoringStarted{false};
static std::atomic<bool> g_comInitialized{false};
static std::mutex g_consoleMutex;

// 缓存静态系统信息
static std::atomic<bool> systemInfoCached{false};
static std::string cachedOsVersion;
static std::string cachedCpuName;
static uint32_t cachedPhysicalCores = 0;
static uint32_t cachedLogicalCores = 0;
static uint32_t cachedPerformanceCores = 0;
static uint32_t cachedEfficiencyCores = 0;
static bool cachedHyperThreading = false;
static bool cachedVirtualization = false;

// 函数声明
bool CheckForKeyPress();
char GetKeyPress();
void SafeExit(int exitCode);
void SafeConsoleOutput(const std::string& message);
void SafeConsoleOutput(const std::string& message, int color);

// 平台特定函数声明
#ifdef PLATFORM_WINDOWS
    void SEHTranslator(unsigned int u, EXCEPTION_POINTERS* pExp);
    std::string GetSEHExceptionName(DWORD exceptionCode);
    BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
    bool IsRunAsAdmin();
#elif defined(PLATFORM_MACOS) || defined(PLATFORM_LINUX)
    void SignalHandler(int signal);
    bool IsRunAsRoot();
#endif// 线程安全的GPU信息缓存类
class ThreadSafeGpuCache {
private:
    mutable std::mutex mtx_;
    bool initialized_ = false;
    std::string cachedGpuName_ = "GPU not detected";
    std::string cachedGpuBrand_ = "Unknown";
    uint64_t cachedGpuMemory_ = 0;
    uint32_t cachedGpuCoreFreq_ = 0;
    bool cachedGpuIsVirtual_ = false;

public:
    void Initialize() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (initialized_) return;
        
        try {
            Logger::Info("Initializing GPU information");
            
#ifdef PLATFORM_WINDOWS
            // Windows 平台使用 WMI
            std::unique_ptr<WmiManager> wmiManager;
            try {
                wmiManager = std::make_unique<WmiManager>();
                if (wmiManager->IsInitialized()) {
                    GpuInfo gpuInfo(*wmiManager);
                    const auto& gpus = gpuInfo.GetGpuData();
                    
                    // 优先选择非虚拟GPU
                    const GpuInfo::GpuData* selectedGpu = nullptr;
                    for (const auto& gpu : gpus) {
                        if (!gpu.isVirtual) {
                            selectedGpu = &gpu;
                            break;
                        }
                    }
                    
                    if (!selectedGpu && !gpus.empty()) {
                        selectedGpu = &gpus[0];
                    }
                    
                    if (selectedGpu) {
                        cachedGpuName_ = WinUtils::WstringToString(selectedGpu->name);
                        cachedGpuMemory_ = selectedGpu->dedicatedMemory;
                        cachedGpuCoreFreq_ = static_cast<uint32_t>(selectedGpu->coreClock);
                        cachedGpuIsVirtual_ = selectedGpu->isVirtual;
                        
                        // 检测品牌
                        if (cachedGpuName_.find("NVIDIA") != std::string::npos) {
                            cachedGpuBrand_ = "NVIDIA";
                        } else if (cachedGpuName_.find("AMD") != std::string::npos) {
                            cachedGpuBrand_ = "AMD";
                        } else if (cachedGpuName_.find("Intel") != std::string::npos) {
                            cachedGpuBrand_ = "Intel";
                        }
                        
                        Logger::Info("Selected main GPU: " + cachedGpuName_);
                    }
                }
            } catch (const std::exception& e) {
                Logger::Error("GPU initialization failed: " + std::string(e.what()));
            }
#else
            // macOS/Linux 平台实现
            // TODO: 实现跨平台GPU检测
            Logger::Info("Cross-platform GPU detection not yet implemented");
#endif
            
            initialized_ = true;
            Logger::Info("GPU information initialization complete");
        }
        catch (const std::exception& e) {
            Logger::Error("GPU information initialization failed: " + std::string(e.what()));
            initialized_ = true;
        }
    }
    
    void GetCachedInfo(std::string& name, std::string& brand, uint64_t& memory, 
                       uint32_t& coreFreq, bool& isVirtual) const {
        std::lock_guard<std::mutex> lock(mtx_);
        name = cachedGpuName_;
        brand = cachedGpuBrand_;
        memory = cachedGpuMemory_;
        coreFreq = cachedGpuCoreFreq_;
        isVirtual = cachedGpuIsVirtual_;
    }
    
    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return initialized_;
    }
};// 平台特定实现
#ifdef PLATFORM_WINDOWS

// 结构化异常处理
std::string GetSEHExceptionName(DWORD exceptionCode) {
    switch (exceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION: return "Access Violation";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array Bounds Exceeded";
        case EXCEPTION_BREAKPOINT: return "Breakpoint";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "Datatype Misalignment";
        case EXCEPTION_FLT_DENORMAL_OPERAND: return "Floating-Point Denormal Operand";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Floating-Point Divide by Zero";
        case EXCEPTION_FLT_INEXACT_RESULT: return "Floating-Point Inexact Result";
        case EXCEPTION_FLT_INVALID_OPERATION: return "Floating-Point Invalid Operation";
        case EXCEPTION_FLT_OVERFLOW: return "Floating-Point Overflow";
        case EXCEPTION_FLT_STACK_CHECK: return "Floating-Point Stack Check";
        case EXCEPTION_FLT_UNDERFLOW: return "Floating-Point Underflow";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal Instruction";
        case EXCEPTION_IN_PAGE_ERROR: return "In Page Error";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer Divide by Zero";
        case EXCEPTION_INT_OVERFLOW: return "Integer Overflow";
        case EXCEPTION_INVALID_DISPOSITION: return "Invalid Disposition";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Non-continuable Exception";
        case EXCEPTION_PRIV_INSTRUCTION: return "Privileged Instruction";
        case EXCEPTION_SINGLE_STEP: return "Single Step";
        case EXCEPTION_STACK_OVERFLOW: return "Stack Overflow";
        default: return "Unknown System Exception (0x" + std::to_string(exceptionCode) + ")";
    }
}

void SEHTranslator(unsigned int u, EXCEPTION_POINTERS* pExp) {
    std::string exceptionName = GetSEHExceptionName(u);
    std::stringstream ss;
    ss << "System-level exception: " << exceptionName << " (0x" << std::hex << u << ")";
    if (pExp && pExp->ExceptionRecord) {
        ss << " Address: 0x" << std::hex << pExp->ExceptionRecord->ExceptionAddress;
    }
    
    try {
        if (Logger::IsInitialized()) {
            Logger::Fatal(ss.str());
        } else {
            SafeConsoleOutput("FATAL: " + ss.str() + "\n");
        }
    } catch (...) {
        SafeConsoleOutput("FATAL: " + ss.str() + "\n");
    }
    
    throw std::runtime_error(ss.str());
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        Logger::Info("Received system shutdown signal, exiting safely...");
        g_shouldExit = true;
        SafeConsoleOutput("Exiting program...\n", 14);
        return TRUE;
    }
    return FALSE;
}

bool IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin == TRUE;
}

#elif defined(PLATFORM_MACOS) || defined(PLATFORM_LINUX)

void SignalHandler(int signal) {
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            Logger::Info("Received signal " + std::to_string(signal) + ", exiting safely...");
            g_shouldExit = true;
            SafeConsoleOutput("Exiting program...\n");
            break;
    }
}

bool IsRunAsRoot() {
    return getuid() == 0;
}

#endif// 跨平台控制台输出函数
void SafeConsoleOutput(const std::string& message) {
    std::lock_guard<std::mutex> lock(g_consoleMutex);
    try {
        if (!message.empty()) {
#ifdef PLATFORM_WINDOWS
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole != INVALID_HANDLE_VALUE) {
                // 转换UTF-8到UTF-16
                int wideLength = MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, nullptr, 0);
                if (wideLength > 0) {
                    std::vector<wchar_t> wideMessage(wideLength);
                    if (MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, wideMessage.data(), wideLength)) {
                        DWORD written;
                        WriteConsoleW(hConsole, wideMessage.data(), static_cast<DWORD>(wideLength - 1), &written, NULL);
                        return;
                    }
                }
                // 回退到ASCII
                DWORD written;
                WriteConsoleA(hConsole, message.c_str(), static_cast<DWORD>(message.length()), &written, NULL);
            }
#else
            // Unix-like系统直接输出
            std::cout << message;
            std::cout.flush();
#endif
        }
    } catch (...) {
        // 忽略控制台输出错误
    }
}

void SafeConsoleOutput(const std::string& message, int color) {
    std::lock_guard<std::mutex> lock(g_consoleMutex);
    try {
        if (!message.empty()) {
#ifdef PLATFORM_WINDOWS
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole != INVALID_HANDLE_VALUE) {
                // 保存原始颜色
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(hConsole, &csbi);
                WORD originalColor = csbi.wAttributes;
                
                // 设置新颜色
                SetConsoleTextAttribute(hConsole, color);
                
                // 输出消息
                int wideLength = MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, nullptr, 0);
                if (wideLength > 0) {
                    std::vector<wchar_t> wideMessage(wideLength);
                    if (MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, wideMessage.data(), wideLength)) {
                        DWORD written;
                        WriteConsoleW(hConsole, wideMessage.data(), static_cast<DWORD>(wideLength - 1), &written, NULL);
                    }
                }
                
                // 恢复原始颜色
                SetConsoleTextAttribute(hConsole, originalColor);
            }
#else
            // Unix-like系统使用ANSI颜色代码
            std::string colorCode = "\033[0m"; // 重置
            switch (color) {
                case 14: // 黄色
                    colorCode = "\033[33m";
                    break;
                case 12: // 红色
                    colorCode = "\033[31m";
                    break;
                case 10: // 绿色
                    colorCode = "\033[32m";
                    break;
                default:
                    colorCode = "\033[0m";
                    break;
            }
            std::cout << colorCode << message << "\033[0m";
            std::cout.flush();
#endif
        }
    } catch (...) {
        // 忽略控制台输出错误
    }
}// 安全退出函数
void SafeExit(int exitCode) {
    try {
        Logger::Info("Starting program cleanup process");
        
        g_shouldExit = true;
        
        // 清理硬件监控桥接
        try {
            TemperatureWrapper::Cleanup();
        }
        catch (const std::exception& e) {
            Logger::Error("Error during hardware monitor bridge cleanup: " + std::string(e.what()));
        }
        
        // 清理共享内存
        try {
            SharedMemoryManager::CleanupSharedMemory();
            Logger::Debug("Shared memory cleanup complete");
        }
        catch (const std::exception& e) {
            Logger::Error("Error during shared memory cleanup: " + std::string(e.what()));
        }
        
#ifdef PLATFORM_WINDOWS
        // 清理COM
        if (g_comInitialized.load()) {
            try {
                CoUninitialize();
                g_comInitialized = false;
                Logger::Debug("COM cleanup complete");
            }
            catch (...) {
                Logger::Error("Unknown error during COM cleanup");
            }
        }
#endif
        
        Logger::Info("Program cleanup complete, exit code: " + std::to_string(exitCode));
        
        // 给日志系统一些时间完成写入
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    catch (...) {
        // 最终异常处理器，避免清理期间崩溃
    }
    
    exit(exitCode);
}

// 检查按键输入（非阻塞）
bool CheckForKeyPress() {
    try {
#ifdef PLATFORM_WINDOWS
        return _kbhit() != 0;
#else
        // Unix-like系统使用非阻塞读取
        struct termios oldt, newt;
        int ch;
        int oldf;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        if (ch != EOF) {
            ungetc(ch, stdin);
            return true;
        }
        return false;
#endif
    }
    catch (const std::exception& e) {
        Logger::Warn("Error checking for key press: " + std::string(e.what()));
        return false;
    }
}

// 获取按键（非阻塞）
char GetKeyPress() {
    try {
#ifdef PLATFORM_WINDOWS
        if (_kbhit()) {
            char key = _getch();
            if (key >= 0 && key <= 127) {
                return key;
            }
        }
#else
        // Unix-like系统
        struct termios oldt, newt;
        int ch;
        int oldf;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        if (ch != EOF && ch >= 0 && ch <= 127) {
            return static_cast<char>(ch);
        }
#endif
    }
    catch (const std::exception& e) {
        Logger::Warn("Error getting key press: " + std::string(e.what()));
    }
    return 0;
}// 辅助函数
std::string FormatSize(uint64_t bytes, bool useBinary = true) {
    try {
        const double kb = useBinary ? 1024.0 : 1000.0;
        const double mb = kb * kb;
        const double gb = mb * kb;
        const double tb = gb * kb;

        if (bytes == UINT64_MAX) {
            Logger::Warn("Byte count is at max value, may indicate an error state");
            return "N/A";
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);

        if (bytes >= tb) ss << (bytes / tb) << " TB";
        else if (bytes >= gb) ss << (bytes / gb) << " GB";
        else if (bytes >= mb) ss << (bytes / mb) << " MB";
        else if (bytes >= kb) ss << (bytes / kb) << " KB";
        else ss << bytes << " B";

        return ss.str();
    }
    catch (const std::exception& e) {
        Logger::Error("Exception occurred while formatting size: " + std::string(e.what()));
        return "Formatting failed";
    }
}// 主函数
int main(int argc, char* argv[]) {
    // 设置内存分配失败处理
    std::set_new_handler([]() {
        Logger::Fatal("Memory allocation failed - system out of memory");
        throw std::bad_alloc();
    });
    
    // 平台特定初始化
#ifdef PLATFORM_WINDOWS
    // 设置结构化异常处理
    _set_se_translator(SEHTranslator);
    
    // 设置控制台编码为UTF-8
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    // 设置控制台信号处理器
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    
    // 设置本地化支持UTF-8
    setlocale(LC_ALL, "C.UTF-8");
#else
    // Unix-like系统设置信号处理器
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
    // 设置本地化
    setlocale(LC_ALL, "C.UTF-8");
#endif
    
    try {
        // 初始化日志系统
        try {
            Logger::Initialize("system_monitor.log", false);
            Logger::SetLogLevel(LL_DEBUG);
            Logger::Info("Program starting");
        }
        catch (const std::exception& e) {
            printf("Log system initialization failed: %s\n", e.what());
            return 1;
        }

        // 检查权限
#ifdef PLATFORM_WINDOWS
        if (!IsRunAsAdmin()) {
            wchar_t szPath[MAX_PATH];
            GetModuleFileNameW(NULL, szPath, MAX_PATH);

            // 以管理员权限重启
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;

            if (ShellExecuteExW(&sei)) {
                exit(0);
            } else {
                MessageBoxW(NULL, L"Automatic elevation failed, please run as administrator.", L"Insufficient privileges", MB_OK | MB_ICONERROR);
                SafeExit(1);
            }
        }
#else
        if (!IsRunAsRoot()) {
            SafeConsoleOutput("Warning: Not running as root. Some hardware information may not be accessible.\n");
            SafeConsoleOutput("On macOS/Linux, certain hardware monitoring requires root privileges.\n");
        }
#endif

        // 平台特定初始化
#ifdef PLATFORM_WINDOWS
        // 初始化COM
        try {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (FAILED(hr)) {
                if (hr == RPC_E_CHANGED_MODE) {
                    Logger::Warn("COM initialization mode conflict, trying single-threaded mode");
                    hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
                    if (FAILED(hr)) {
                        Logger::Error("COM initialization failed: 0x" + std::to_string(hr));
                        return -1;
                    }
                }
                else {
                    Logger::Error("COM initialization failed: 0x" + std::to_string(hr));
                    return -1;
                }
            }
            g_comInitialized = true;
            Logger::Debug("COM initialization successful");
        }
        catch (const std::exception& e) {
            Logger::Error("Exception during COM initialization: " + std::string(e.what()));
            return -1;
        }
#endif

        // 初始化共享内存
        try {
            if (!SharedMemoryManager::InitSharedMemory()) {
                std::string error = SharedMemoryManager::GetLastError();
                Logger::Error("Shared memory initialization failed: " + error);
                
                Logger::Info("Attempting to reinitialize shared memory...");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                
                if (!SharedMemoryManager::InitSharedMemory()) {
                    Logger::Critical("Shared memory reinitialization failed, program cannot continue");
                    SafeExit(1);
                }
            }
            Logger::Info("Shared memory initialization successful");
        }
        catch (const std::exception& e) {
            Logger::Error("Exception during shared memory initialization: " + std::string(e.what()));
            SafeExit(1);
        }

#ifdef PLATFORM_WINDOWS
        // 创建WMI管理器
        std::unique_ptr<WmiManager> wmiManager;
        try {
            wmiManager = std::make_unique<WmiManager>();
            if (!wmiManager) {
                Logger::Fatal("WMI manager object creation failed - memory allocation returned null");
                SafeExit(1);
            }
            if (!wmiManager->IsInitialized()) {
                Logger::Error("WMI initialization failed");
                MessageBoxA(NULL, "WMI initialization failed, cannot get system information.", "Error", MB_OK | MB_ICONERROR);
                SafeExit(1);
            }
            Logger::Debug("WMI manager initialization successful");
        }
        catch (const std::bad_alloc& e) {
            Logger::Fatal("WMI manager creation failed - memory allocation failed: " + std::string(e.what()));
            SafeExit(1);
        }
        catch (const std::exception& e) {
            Logger::Error("WMI manager creation failed: " + std::string(e.what()));
            SafeExit(1);
        }
        catch (...) {
            Logger::Fatal("WMI manager creation failed - unknown exception");
            SafeExit(1);
        }
#endif

        // 初始化硬件监控桥接
        try {
            TemperatureWrapper::Initialize();
        }
        catch (const std::exception& e) {
            Logger::Error("Hardware monitor bridge initialization failed: " + std::string(e.what()));
            // 继续运行但温度数据可能不可用
        }

        Logger::Info("Program startup complete");
        
        // 初始化循环计数器
        int loopCounter = 1;
        bool isFirstRun = true;
        
        // 创建CPU对象
        std::unique_ptr<CpuInfo> cpuInfo;
        try {
            cpuInfo = std::make_unique<CpuInfo>();
            if (!cpuInfo) {
                Logger::Fatal("CPU information object creation failed - memory allocation returned null");
                SafeExit(1);
            }
            Logger::Debug("CPU information object creation successful");
        }
        catch (const std::bad_alloc& e) {
            Logger::Fatal("CPU information object creation failed - memory allocation failed: " + std::string(e.what()));
            SafeExit(1);
        }
        catch (const std::exception& e) {
            Logger::Error("CPU information object creation failed: " + std::string(e.what()));
            SafeExit(1);
        }
        catch (...) {
            Logger::Fatal("CPU information object creation failed - unknown exception");
            SafeExit(1);
        }
        
        // 线程安全的GPU缓存
        ThreadSafeGpuCache gpuCache;
        
        // 主监控循环
        while (!g_shouldExit.load()) {
            try {
                auto loopStart = std::chrono::high_resolution_clock::now();
                
                bool isDetailedLogging = (loopCounter % 5 == 1);
                
                if (isDetailedLogging) {
                    Logger::Debug("Starting main monitoring loop iteration #" + std::to_string(loopCounter));
                }
                
                if (loopCounter == 5) {
                    g_monitoringStarted = true;
                    Logger::Info("Program is running stably");
                }
                
                // 获取系统信息
                SystemInfo sysInfo;
                
                // 初始化所有字段
                try {
                    sysInfo.cpuUsage = 0.0;
                    sysInfo.performanceCoreFreq = 0.0;
                    sysInfo.efficiencyCoreFreq = 0.0;
                    sysInfo.totalMemory = 0;
                    sysInfo.usedMemory = 0;
                    sysInfo.availableMemory = 0;
                    sysInfo.gpuMemory = 0;
                    sysInfo.gpuCoreFreq = 0.0;
                    sysInfo.gpuIsVirtual = false;
                    sysInfo.networkAdapterSpeed = 0;
                    
#ifdef PLATFORM_WINDOWS
                    // Windows设置系统时间
                    ZeroMemory(&sysInfo.lastUpdate, sizeof(sysInfo.lastUpdate));
                    GetSystemTime(&sysInfo.lastUpdate);
#else
                    // Unix-like系统设置时间
                    auto now = std::chrono::system_clock::now();
                    auto time_t = std::chrono::system_clock::to_time_t(now);
                    sysInfo.lastUpdate.unixTime = static_cast<uint64_t>(time_t);
                    sysInfo.lastUpdate.milliseconds = static_cast<uint32_t>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()
                        ).count() % 1000
                    );
#endif
                    
                    // 跨平台TPM检测
                    try {
                        static std::unique_ptr<CrossPlatformTpmInfo> tpmInfo;
                        if (!tpmInfo) {
                            tpmInfo = std::make_unique<CrossPlatformTpmInfo>();
                            if (!tpmInfo->Initialize()) {
                                Logger::Warn("Failed to initialize cross-platform TPM detection");
                            }
                        }
                        
                        if (tpmInfo && tpmInfo->HasTpm()) {
                            const CrossPlatformTpmData& tpmData = tpmInfo->GetTpmData();
                            
                            sysInfo.hasTpm = tpmData.tpmPresent;
                            sysInfo.tpmManufacturer = tpmData.manufacturer;
                            sysInfo.tpmManufacturerId = tpmData.manufacturerId;
                            sysInfo.tpmVersion = tpmData.version;
                            sysInfo.tpmFirmwareVersion = tpmData.firmwareVersion;
                            sysInfo.tpmStatus = tpmData.status;
                            sysInfo.tpmEnabled = tpmData.isEnabled;
                            sysInfo.tpmIsActivated = tpmData.isActivated;
                            sysInfo.tpmIsOwned = tpmData.isOwned;
                            sysInfo.tpmReady = tpmData.isReady;
                            sysInfo.tpmSpecVersion = tpmData.specVersion;
                            sysInfo.tpmSpecVersion = tpmData.specRevision;
                            sysInfo.tpmErrorMessage = tpmData.errorMessage;
                            sysInfo.tmpDetectionMethod = tpmData.detectionMethod;
                            sysInfo.tmpWmiDetectionWorked = tpmData.wmiDetectionWorked;
                            sysInfo.tmpTbsDetectionWorked = tpmData.tpm2TssDetectionWorked;
                            
                            if (sysInfo.hasTpm) {
                                Logger::Info("Cross-platform TPM detection successful: " + tpmData.manufacturer);
                            }
                        } else {
                            sysInfo.hasTpm = false;
                        }
                    }
                    catch (const std::exception& e) {
                        Logger::Error("Cross-platform TPM detection exception: " + std::string(e.what()));
                        sysInfo.hasTpm = false;
                    }
                }
                catch (const std::exception& e) {
                    Logger::Error("SystemInfo initialization failed: " + std::string(e.what()));
                    continue;
                }
                catch (...) {
                    Logger::Error("SystemInfo initialization failed - unknown exception");
                    continue;
                }

                // 静态系统信息（只在首次获取）
                if (!systemInfoCached.load()) {
                    try {
                        Logger::Info("Initializing system information");
                        
                        // 操作系统信息
                        OSInfo os;
                        cachedOsVersion = os.GetVersion();

                        // CPU基本信息
                        if (cpuInfo) {
                            cachedCpuName = cpuInfo->GetName();
                            cachedPhysicalCores = cpuInfo->GetLargeCores() + cpuInfo->GetSmallCores();
                            cachedLogicalCores = cpuInfo->GetTotalCores();
                            cachedPerformanceCores = cpuInfo->GetLargeCores();
                            cachedEfficiencyCores = cpuInfo->GetSmallCores();
                            cachedHyperThreading = cpuInfo->IsHyperThreadingEnabled();
                            cachedVirtualization = cpuInfo->IsVirtualizationEnabled();
                        }
                        
                        systemInfoCached = true;
                        Logger::Info("System information initialization complete");
                    }
                    catch (const std::exception& e) {
                        Logger::Error("System information initialization failed: " + std::string(e.what()));
                        cachedOsVersion = "Unknown";
                        cachedCpuName = "Unknown";
                        systemInfoCached = true;
                    }
                }
                
                // 使用缓存的静态信息
                sysInfo.osVersion = cachedOsVersion;
                sysInfo.cpuName = cachedCpuName;
                sysInfo.physicalCores = cachedPhysicalCores;
                sysInfo.logicalCores = cachedLogicalCores;
                sysInfo.performanceCores = cachedPerformanceCores;
                sysInfo.efficiencyCores = cachedEfficiencyCores;
                sysInfo.hyperThreading = cachedHyperThreading;
                sysInfo.virtualization = cachedVirtualization;

                // 动态CPU信息
                try {
                    if (cpuInfo) {
                        sysInfo.cpuUsage = cpuInfo->GetUsage();
                        sysInfo.performanceCoreFreq = cpuInfo->GetLargeCoreSpeed();
                        sysInfo.efficiencyCoreFreq = cpuInfo->GetSmallCoreSpeed() * 0.8;
                        sysInfo.cpuBaseFrequencyMHz = cpuInfo->GetBaseFrequencyMHz();
                        sysInfo.cpuCurrentFrequencyMHz = cpuInfo->GetCurrentFrequencyMHz();
                        sysInfo.cpuUsageSampleIntervalMs = cpuInfo->GetLastSampleIntervalMs();
                    }
                }
                catch (const std::exception& e) {
                    Logger::Error("Failed to get CPU dynamic information: " + std::string(e.what()));
                }

                // 内存信息
                try {
                    MemoryInfo mem;
                    sysInfo.totalMemory = mem.GetTotalPhysical();
                    sysInfo.usedMemory = mem.GetTotalPhysical() - mem.GetAvailablePhysical();
                    sysInfo.availableMemory = mem.GetAvailablePhysical();
                }
                catch (const std::exception& e) {
                    Logger::Error("Failed to get memory information: " + std::string(e.what()));
                }

                // GPU信息
                if (!gpuCache.IsInitialized()) {
                    try {
                        gpuCache.Initialize();
                    }
                    catch (const std::exception& e) {
                        Logger::Error("GPU cache initialization failed: " + std::string(e.what()));
                    }
                }
                
                try {
                    std::string cachedGpuName, cachedGpuBrand;
                    uint64_t cachedGpuMemory;
                    uint32_t cachedGpuCoreFreq;
                    bool cachedGpuIsVirtual;
                    
                    gpuCache.GetCachedInfo(cachedGpuName, cachedGpuBrand, cachedGpuMemory, 
                                          cachedGpuCoreFreq, cachedGpuIsVirtual);
                    
                    sysInfo.gpuName = cachedGpuName;
                    sysInfo.gpuBrand = cachedGpuBrand;
                    sysInfo.gpuMemory = cachedGpuMemory;
                    sysInfo.gpuCoreFreq = cachedGpuCoreFreq;
                    sysInfo.gpuIsVirtual = cachedGpuIsVirtual;

                    sysInfo.gpus.clear();
                    if (!cachedGpuName.empty() && cachedGpuName != "GPU not detected") {
                        GPUData gpu;
                        memset(&gpu, 0, sizeof(GPUData));
                        
#ifdef PLATFORM_WINDOWS
                        std::wstring gpuNameW = WinUtils::StringToWstring(cachedGpuName);
                        std::wstring gpuBrandW = WinUtils::StringToWstring(cachedGpuBrand);
                        
                        if (gpuNameW.length() >= sizeof(gpu.name)/sizeof(wchar_t)) {
                            gpuNameW = gpuNameW.substr(0, sizeof(gpu.name)/sizeof(wchar_t) - 1);
                        }
                        if (gpuBrandW.length() >= sizeof(gpu.brand)/sizeof(wchar_t)) {
                            gpuBrandW = gpuBrandW.substr(0, sizeof(gpu.brand)/sizeof(wchar_t) - 1);
                        }
                        
                        wcsncpy_s(gpu.name, sizeof(gpu.name)/sizeof(wchar_t), gpuNameW.c_str(), _TRUNCATE);
                        wcsncpy_s(gpu.brand, sizeof(gpu.brand)/sizeof(wchar_t), gpuBrandW.c_str(), _TRUNCATE);
#else
                        // Unix-like系统处理
                        std::wstring gpuNameW(cachedGpuName.begin(), cachedGpuName.end());
                        std::wstring gpuBrandW(cachedGpuBrand.begin(), cachedGpuBrand.end());
                        
                        wcsncpy(gpu.name, gpuNameW.c_str(), sizeof(gpu.name)/sizeof(wchar_t) - 1);
                        gpu.name[sizeof(gpu.name)/sizeof(wchar_t) - 1] = L'\0';
                        wcsncpy(gpu.brand, gpuBrandW.c_str(), sizeof(gpu.brand)/sizeof(wchar_t) - 1);
                        gpu.brand[sizeof(gpu.brand)/sizeof(wchar_t) - 1] = L'\0';
#endif
                        
                        gpu.memory = (cachedGpuMemory > 0 && cachedGpuMemory < UINT64_MAX) ? cachedGpuMemory : 0;
                        
                        if (cachedGpuCoreFreq > 0 && cachedGpuCoreFreq < 10000) {
                            gpu.coreClock = cachedGpuCoreFreq;
                        } else {
                            gpu.coreClock = 0;
                        }
                        
                        gpu.isVirtual = cachedGpuIsVirtual;
                        sysInfo.gpus.push_back(gpu);
                    }
                }
                catch (const std::bad_alloc& e) {
                    Logger::Error("GPU cache information processing failed - out of memory: " + std::string(e.what()));
                    sysInfo.gpus.clear();
                    sysInfo.gpuName = "Out of memory";
                    sysInfo.gpuBrand = "Unknown";
                    sysInfo.gpuMemory = 0;
                    sysInfo.gpuCoreFreq = 0;
                    sysInfo.gpuIsVirtual = false;
                }
                catch (const std::exception& e) {
                    Logger::Error("Failed to get GPU cache information: " + std::string(e.what()));
                    sysInfo.gpus.clear();
                    sysInfo.gpuName = "GPU information retrieval failed";
                    sysInfo.gpuBrand = "Unknown";
                    sysInfo.gpuMemory = 0;
                    sysInfo.gpuCoreFreq = 0;
                    sysInfo.gpuIsVirtual = false;
                }
                catch (...) {
                    Logger::Error("Failed to get GPU cache information - unknown exception");
                    sysInfo.gpus.clear();
                    sysInfo.gpuName = "Unknown exception";
                    sysInfo.gpuBrand = "Unknown";
                    sysInfo.gpuMemory = 0;
                    sysInfo.gpuCoreFreq = 0;
                    sysInfo.gpuIsVirtual = false;
                }

                // 网络适配器信息
                sysInfo.networkAdapterName = "No network adapter detected";
                sysInfo.networkAdapterMac = "00-00-00-00-00-00";
                sysInfo.networkAdapterSpeed = 0;
                sysInfo.networkAdapterIp = "N/A";
                sysInfo.networkAdapterType = "Unknown";

                try {
                    sysInfo.adapters.clear();
#ifdef PLATFORM_WINDOWS
                    if (wmiManager && wmiManager->IsInitialized()) {
                        NetworkAdapter netAdapter(*wmiManager);
                        const auto& adapters = netAdapter.GetAdapters();
                        if (!adapters.empty()) {
                            for (const auto& adapter : adapters) {
                                NetworkAdapterData data;
                                wcsncpy_s(data.name, adapter.name.c_str(), _TRUNCATE);
                                wcsncpy_s(data.mac, adapter.mac.c_str(), _TRUNCATE);
                                wcsncpy_s(data.ipAddress, adapter.ip.c_str(), _TRUNCATE);
                                wcsncpy_s(data.adapterType, adapter.adapterType.c_str(), _TRUNCATE);
                                data.speed = adapter.speed;
                                sysInfo.adapters.push_back(data);
                            }
                            sysInfo.networkAdapterName = WinUtils::WstringToString(adapters[0].name);
                            sysInfo.networkAdapterMac = WinUtils::WstringToString(adapters[0].mac);
                            sysInfo.networkAdapterIp = WinUtils::WstringToString(adapters[0].ip);
                            sysInfo.networkAdapterType = WinUtils::WstringToString(adapters[0].adapterType);
                            sysInfo.networkAdapterSpeed = adapters[0].speed;
                        }
                    }
#else
                    // macOS/Linux网络适配器检测
                    // TODO: 实现跨平台网络适配器检测
                    Logger::Debug("Cross-platform network adapter detection not yet implemented");
#endif
                }
                catch (const std::bad_alloc& e) {
                    Logger::Error("Failed to get network adapter information - out of memory: " + std::string(e.what()));
                    sysInfo.adapters.clear();
                    sysInfo.networkAdapterName = "Out of memory";
                    sysInfo.networkAdapterMac = "00-00-00-00-00-00";
                    sysInfo.networkAdapterIp = "N/A";
                    sysInfo.networkAdapterType = "Unknown";
                    sysInfo.networkAdapterSpeed = 0;
                }
                catch (const std::exception& e) {
                    Logger::Error("Failed to get network adapter information: " + std::string(e.what()));
                    sysInfo.adapters.clear();
                    sysInfo.networkAdapterName = "No network adapter detected";
                    sysInfo.networkAdapterMac = "00-00-00-00-00-00";
                    sysInfo.networkAdapterIp = "N/A";
                    sysInfo.networkAdapterType = "Unknown";
                    sysInfo.networkAdapterSpeed = 0;
                }
                catch (...) {
                    Logger::Error("Failed to get network adapter information - unknown exception");
                    sysInfo.adapters.clear();
                    sysInfo.networkAdapterName = "Unknown exception";
                    sysInfo.networkAdapterMac = "00-00-00-00-00-00";
                    sysInfo.networkAdapterIp = "N/A";
                    sysInfo.networkAdapterType = "Unknown";
                    sysInfo.networkAdapterSpeed = 0;
                }

                // 温度数据
                try {
                    auto temperatures = TemperatureWrapper::GetTemperatures();
                    sysInfo.temperatures.clear();
                    sysInfo.cpuTemperature = 0;
                    sysInfo.gpuTemperature = 0;
                    for (const auto& temp : temperatures) {
                        std::string nameLower = temp.first;
                        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        if (nameLower.find("gpu") != std::string::npos || nameLower.find("graphics") != std::string::npos) {
                            sysInfo.gpuTemperature = temp.second;
                            sysInfo.temperatures.push_back({"GPU", temp.second});
                        } else if (nameLower.find("cpu") != std::string::npos || nameLower.find("package") != std::string::npos) {
                            sysInfo.cpuTemperature = temp.second;
                            sysInfo.temperatures.push_back({"CPU", temp.second});
                        } else {
                            sysInfo.temperatures.push_back(temp);
                        }
                    }
                }
                catch (const std::bad_alloc& e) {
                    Logger::Error("Failed to get temperature data - out of memory: " + std::string(e.what()));
                    sysInfo.temperatures.clear();
                    sysInfo.cpuTemperature = 0;
                    sysInfo.gpuTemperature = 0;
                }
                catch (const std::exception& e) {
                    Logger::Error("Failed to get temperature data: " + std::string(e.what()));
                    sysInfo.temperatures.clear();
                    sysInfo.cpuTemperature = 0;
                    sysInfo.gpuTemperature = 0;
                }
                catch (...) {
                    Logger::Error("Failed to get temperature data - unknown exception");
                    sysInfo.temperatures.clear();
                    sysInfo.cpuTemperature = 0;
                    sysInfo.gpuTemperature = 0;
                }

                // USB设备数据
                try {
                    sysInfo.usbDevices.clear();
                    
                    if (SharedMemoryManager::GetBuffer()) {
                        // USB数据会在SharedMemoryManager中处理
                    }
                }
                catch (const std::bad_alloc& e) {
                    Logger::Error("Failed to get USB device data - out of memory: " + std::string(e.what()));
                    sysInfo.usbDevices.clear();
                }
                catch (const std::exception& e) {
                    Logger::Error("Failed to get USB device data: " + std::string(e.what()));
                    sysInfo.usbDevices.clear();
                }
                catch (...) {
                    Logger::Error("Failed to get USB device data - unknown exception");
                    sysInfo.usbDevices.clear();
                }

#ifdef PLATFORM_WINDOWS
                // TPM信息采集
                try {
                    if (wmiManager && wmiManager->IsInitialized()) {
                        TpmInfo tpm(*wmiManager);
                        const auto& td = tpm.GetTpmData();

                        sysInfo.hasTpm = tpm.HasTpm();
                        sysInfo.tpmManufacturer = WinUtils::WstringToString(td.manufacturerName);
                        sysInfo.tpmManufacturerId = WinUtils::WstringToString(td.manufacturerId);
                        sysInfo.tpmVersion = WinUtils::WstringToString(td.version);
                        sysInfo.tpmFirmwareVersion = WinUtils::WstringToString(td.firmwareVersion);
                        sysInfo.tpmStatus = WinUtils::WstringToString(td.status);
                        sysInfo.tpmEnabled = td.isEnabled;
                        sysInfo.tpmIsActivated = td.isActivated;
                        sysInfo.tpmIsOwned = td.isOwned;
                        sysInfo.tpmReady = td.isReady;
                        sysInfo.tpmTbsAvailable = td.tbsAvailable;
                        sysInfo.tpmPhysicalPresenceRequired = td.physicalPresenceRequired;
                        sysInfo.tpmSpecVersion = td.specVersion;
                        sysInfo.tpmTbsVersion = td.tbsVersion;
                        sysInfo.tpmErrorMessage = WinUtils::WstringToString(td.errorMessage);
                        sysInfo.tmpDetectionMethod = WinUtils::WstringToString(td.detectionMethod);
                        sysInfo.tmpWmiDetectionWorked = td.wmiDetectionWorked;
                        sysInfo.tmpTbsDetectionWorked = td.tbsDetectionWorked;
                    }
                }
                catch (const std::exception& e) {
                    Logger::Error("TPM information collection failed: " + std::string(e.what()));
                }
                catch (...) {
                    Logger::Error("TPM information collection failed - unknown exception");
                }
#endif

                // 数据验证
                try {
                    if (sysInfo.cpuUsage < 0.0 || sysInfo.cpuUsage > 100.0) {
                        Logger::Warn("Abnormal CPU usage data: " + std::to_string(sysInfo.cpuUsage) + "%, reset to 0");
                        sysInfo.cpuUsage = 0.0;
                    }
                    
                    if (sysInfo.totalMemory > 0) {
                        if (sysInfo.usedMemory > sysInfo.totalMemory) {
                            Logger::Warn("Used memory exceeds total memory, data anomaly");
                            sysInfo.usedMemory = sysInfo.totalMemory;
                        }
                        if (sysInfo.availableMemory > sysInfo.totalMemory) {
                            Logger::Warn("Available memory exceeds total memory, data anomaly");
                            sysInfo.availableMemory = sysInfo.totalMemory;
                        }
                    }
                }
                catch (const std::exception& e) {
                    Logger::Error("Exception during data validation: " + std::string(e.what()));
                }
                catch (...) {
                    Logger::Error("Unknown exception during data validation");
                }

                // 写入共享内存
                try {
                    if (SharedMemoryManager::GetBuffer()) {
                        SharedMemoryManager::WriteToSharedMemory(sysInfo);
                        if (isDetailedLogging) {
                            Logger::Debug("Successfully updated shared memory");
                        }
                    } else {
                        Logger::Error("Shared memory buffer not available");
                        if (SharedMemoryManager::InitSharedMemory()) {
                            SharedMemoryManager::WriteToSharedMemory(sysInfo);
                            if (isDetailedLogging) {
                                Logger::Info("Reinitialized and updated shared memory");
                            }
                        } else {
                            Logger::Error("Failed to reinitialize shared memory: " + SharedMemoryManager::GetLastError());
                        }
                    }
                }
                catch (const std::bad_alloc& e) {
                    Logger::Error("Out of memory while processing system information: " + std::string(e.what()));
                }
                catch (const std::exception& e) {
                    Logger::Error("Exception while processing system information: " + std::string(e.what()));
                }
                catch (...) {
                    Logger::Error("Unknown exception while processing system information");
                }

                // 计算循环执行时间并自适应休眠
                try {
                    auto loopEnd = std::chrono::high_resolution_clock::now();
                    auto loopDuration = std::chrono::duration_cast<std::chrono::milliseconds>(loopEnd - loopStart);
                    
                    int targetCycleTime = 1000;
                    int sleepTime = (std::max)(targetCycleTime - static_cast<int>(loopDuration.count()), 100);
                    
                    if (isDetailedLogging) {
                        double loopTimeSeconds = loopDuration.count() / 1000.0;
                        double sleepTimeSeconds = sleepTime / 1000.0;
                        
                        std::stringstream ss;
                        ss << std::fixed << std::setprecision(2);
                        ss << "Main monitoring loop iteration #" << loopCounter 
                           << " took " << loopTimeSeconds << " seconds, will sleep for " 
                           << sleepTimeSeconds << " seconds";
                        
                        Logger::Debug(ss.str());
                    }
                    
                    // 休眠时检查退出标志
                    auto sleepStart = std::chrono::high_resolution_clock::now();
                    while (!g_shouldExit.load()) {
                        try {
                            auto now = std::chrono::high_resolution_clock::now();
                            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - sleepStart);
                            if (elapsed.count() >= sleepTime) {
                                break;
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        }
                        catch (const std::exception& e) {
                            Logger::Warn("Exception during sleep: " + std::string(e.what()));
                            break;
                        }
                        catch (...) {
                            Logger::Warn("Unknown exception during sleep");
                            break;
                        }
                    }
                }
                catch (const std::exception& e) {
                    Logger::Error("Exception while calculating loop time: " + std::string(e.what()));
                    try {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    } catch (...) {
                        Logger::Fatal("System sleep function abnormal");
                    }
                }
                catch (...) {
                    Logger::Error("Unknown exception while calculating loop time");
                    try {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    } catch (...) {
                        Logger::Fatal("System sleep function abnormal");
                    }
                }
                
                try {
                    loopCounter++;
                    if (loopCounter < 0 || loopCounter > 2000000000) {
                        Logger::Warn("Loop counter abnormal, reset to 1");
                        loopCounter = 1;
                    }
                }
                catch (...) {
                    Logger::Error("Failed to update loop counter");
                    loopCounter = 1;
                }
                
                if (isFirstRun) {
                    isFirstRun = false;
                }
            }
            catch (const std::bad_alloc& e) {
                Logger::Critical("Memory allocation exception in main loop: " + std::string(e.what()));
                try {
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                } catch (...) {
                    Logger::Fatal("Unable to execute sleep, system severely abnormal");
                }
                continue;
            }
            catch (const std::exception& e) {
                Logger::Critical("Exception in main loop: " + std::string(e.what()));
                try {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                } catch (...) {
                    Logger::Fatal("Unable to execute sleep, system severely abnormal");
                }
                continue;
            }
            catch (...) {
                Logger::Fatal("Unknown exception in main loop");
                try {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                } catch (...) {
                    SafeExit(1);
                }
                continue;
            }
        }
        
        Logger::Info("Program received exit signal, starting cleanup");
        SafeExit(0);
    }
    catch (const std::exception& e) {
        Logger::Fatal("Program encountered a fatal error: " + std::string(e.what()));
        SafeExit(1);
    }
    catch (...) {
        Logger::Fatal("Program encountered an unknown fatal error");
        SafeExit(1);
    }
}