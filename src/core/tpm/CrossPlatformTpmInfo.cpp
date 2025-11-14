#include "CrossPlatformTpmInfo.h"
#include "../Utils/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <tbs.h>
    #include <winioctl.h>
#elif defined(PLATFORM_MACOS)
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <IOKit/IOKitLib.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <linux/fs.h>
#endif

// 尝试包含tpm2-tss头文件
#ifdef USE_TPM2_TSS
    #include <tss2/tss2_esys.h>
    #include <tss2/tss2_tcti.h>
#endif

CrossPlatformTpmInfo::CrossPlatformTpmInfo() {
    // 初始化数据结构
    memset(&tpmData, 0, sizeof(tpmData));
}

CrossPlatformTpmInfo::~CrossPlatformTpmInfo() {
    // 清理资源
}

bool CrossPlatformTpmInfo::Initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        Logger::Info("Initializing cross-platform TPM detection");
        
        // 检测tpm2-tss库可用性
        tpm2TssAvailable = DetectTpmViaTpm2Tss();
        
        // 根据平台选择检测方法
        bool detected = false;
#ifdef PLATFORM_WINDOWS
        detected = DetectTpmOnWindows();
#elif defined(PLATFORM_MACOS)
        detected = DetectTpmOnMacOS();
#elif defined(PLATFORM_LINUX)
        detected = DetectTpmOnLinux();
#endif
        
        if (detected) {
            Logger::Info("TPM detected successfully via " + tpmData.detectionMethod);
            
            // 获取TPM能力
            GetCapabilities();
            
            // 检查健康状态
            CheckHealthStatus();
        } else {
            Logger::Warn("No TPM detected on this system");
            tpmData.errorMessage = "No TPM device found";
        }
        
        initialized = true;
        return detected;
    }
    catch (const std::exception& e) {
        Logger::Error("TPM initialization failed: " + std::string(e.what()));
        tpmData.errorMessage = e.what();
        return false;
    }
}

bool CrossPlatformTpmInfo::DetectTpmOnWindows() {
    try {
        // 首先尝试使用tpm2-tss库
        if (tpm2TssAvailable) {
            if (DetectTpmViaTpm2Tss()) {
                tpmData.detectionMethod = "tpm2-tss";
                tpmData.tpm2TssDetectionWorked = true;
                return true;
            }
        }
        
#ifdef PLATFORM_WINDOWS
        // 回退到TBS API
        TBS_CONTEXT_HANDLE hContext = NULL;
        TBS_RESULT result = Tbsi_Context_Create(&hContext, NULL);
        
        if (result == TBS_SUCCESS) {
            // 获取TPM信息
            TBS_CONTEXT_INFO contextInfo;
            DWORD cbContextInfo = sizeof(contextInfo);
            
            result = Tbsi_Context_GetInfo(hContext, &cbContextInfo, &contextInfo);
            if (result == TBS_SUCCESS) {
                tpmData.tpmPresent = true;
                tpmData.detectionMethod = "TBS API";
                
                // 获取TPM版本信息
                TPM_VERSION_INFO versionInfo;
                DWORD cbVersionInfo = sizeof(versionInfo);
                
                result = Tbsi_Get_TPM_Version(hContext, &cbVersionInfo, &versionInfo);
                if (result == TBS_SUCCESS) {
                    tpmData.specVersion = versionInfo.specVersion;
                    tpmData.specRevision = versionInfo.specRevision;
                    tpmData.specLevel = std::to_string(versionInfo.specLevel);
                    
                    // 转换制造商ID
                    if (versionInfo.manufacturerID > 0) {
                        char mfrId[5] = {0};
                        memcpy(mfrId, &versionInfo.manufacturerID, 4);
                        tpmData.manufacturerId = mfrId;
                        
                        // 常见制造商映射
                        if (strcmp(mfrId, "INTC") == 0) {
                            tpmData.manufacturer = "Intel";
                        } else if (strcmp(mfrId, "AMD") == 0) {
                            tpmData.manufacturer = "AMD";
                        } else if (strcmp(mfrId, "STM") == 0) {
                            tpmData.manufacturer = "STMicroelectronics";
                        } else if (strcmp(mfrId, "IFX") == 0) {
                            tpmData.manufacturer = "Infineon";
                        } else if (strcmp(mfrId, "NTZ") == 0) {
                            tpmData.manufacturer = "Nationz";
                        } else {
                            tpmData.manufacturer = "Unknown (" + tpmData.manufacturerId + ")";
                        }
                    }
                }
            }
            
            Tbsi_Context_Close(hContext);
            return true;
        }
#else
        // 非Windows平台，这里不应该执行
        return false;
#endif
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("Windows TPM detection failed: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::DetectTpmOnMacOS() {
    try {
        // 检查TPM设备路径
        const std::vector<std::string> tpmPaths = {
            "/dev/tpm0",
            "/dev/tpm1",
            "/dev/tss0",
            "/dev/tss1"
        };
        
        for (const auto& path : tpmPaths) {
            if (CheckTpmDevice(path)) {
                tpmData.detectionMethod = "Device node: " + path;
                return true;
            }
        }
        
        // 使用IOKit检测TPM
        io_iterator_t iterator;
        kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, 
                                                         IOServiceMatching("AppleTPM2"), 
                                                         &iterator);
        
        if (kr == KERN_SUCCESS) {
            io_service_t service;
            bool found = false;
            
            while ((service = IOIteratorNext(iterator)) != 0) {
                tpmData.tpmPresent = true;
                tpmData.detectionMethod = "IOKit";
                tpmData.manufacturer = "Apple";
                
                IOObjectRelease(service);
                found = true;
                break;
            }
            
            IOObjectRelease(iterator);
            return found;
        }
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("macOS TPM detection failed: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::DetectTpmOnLinux() {
    try {
        // 检查TPM设备路径
        const std::vector<std::string> tpmPaths = {
            "/dev/tpm0",
            "/dev/tpm1",
            "/dev/tss0",
            "/dev/tss1",
            "/dev/tpmrm0"
        };
        
        for (const auto& path : tpmPaths) {
            if (CheckTpmDevice(path)) {
                tpmData.detectionMethod = "Device node: " + path;
                return true;
            }
        }
        
        // 检查sysfs中的TPM信息
        const std::vector<std::string> sysfsPaths = {
            "/sys/class/tpm/tpm0",
            "/sys/class/tpm/tpm1",
            "/sys/class/misc/tpm0",
            "/sys/class/misc/tpm1"
        };
        
        for (const auto& path : sysfsPaths) {
            std::string capsPath = path + "/caps";
            std::ifstream capsFile(capsPath);
            
            if (capsFile.is_open()) {
                std::string line;
                while (std::getline(capsFile, line)) {
                    if (line.find("TPM 2.0") != std::string::npos) {
                        tpmData.tpmPresent = true;
                        tpmData.detectionMethod = "sysfs: " + path;
                        tpmData.specLevel = "2.0";
                        return true;
                    } else if (line.find("TPM 1.2") != std::string::npos) {
                        tpmData.tpmPresent = true;
                        tpmData.detectionMethod = "sysfs: " + path;
                        tpmData.specLevel = "1.2";
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("Linux TPM detection failed: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::CheckTpmDevice(const std::string& devicePath) {
    struct stat st;
    if (stat(devicePath.c_str(), &st) == 0) {
        // 尝试打开设备
        int fd = open(devicePath.c_str(), O_RDONLY);
        if (fd >= 0) {
            close(fd);
            tpmData.tpmPresent = true;
            return true;
        }
    }
    return false;
}

bool CrossPlatformTpmInfo::DetectTpmViaTpm2Tss() {
#ifdef USE_TPM2_TSS
    try {
        TSS2_RC rc;
        ESYS_CONTEXT *ctx = NULL;
        
        // 尝试创建TCTI上下文
        TSS2_TCTI_CONTEXT *tcti = NULL;
        rc = Tss2_Tcti_Device_Init(&tcti, NULL, "/dev/tpm0");
        
        if (rc != TSS2_RC_SUCCESS) {
            Logger::Debug("Failed to initialize TCTI device context");
            return false;
        }
        
        // 初始化ESYS上下文
        rc = Esys_Initialize(&ctx, tcti, NULL);
        if (rc != TSS2_RC_SUCCESS) {
            Logger::Debug("Failed to initialize ESYS context");
            Tss2_Tcti_Finalize(&tcti);
            return false;
        }
        
        // 获取TPM能力
        TPM2B_CAPS capabilities;
        rc = Esys_GetCapability(ctx, ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
                                TPM2_CAP_TPM_PROPERTIES, ESYS_TR_NONE, ESYS_TR_NONE,
                                &capabilities);
        
        if (rc == TSS2_RC_SUCCESS) {
            tpmData.tpmPresent = true;
            tpmData.tpm2TssDetectionWorked = true;
            
            // 解析能力信息
            // TODO: 解析TPM2B_CAPS结构获取详细信息
        }
        
        // 清理资源
        Esys_Finalize(&ctx);
        Tss2_Tcti_Finalize(&tcti);
        
        return tpmData.tpmPresent;
    }
    catch (const std::exception& e) {
        Logger::Error("tpm2-tss detection failed: " + std::string(e.what()));
        return false;
    }
#else
    Logger::Debug("tpm2-tss support not compiled in");
    return false;
#endif
}

bool CrossPlatformTpmInfo::DetectTpmViaSystemCommand() {
    try {
        // 使用系统命令检测TPM
#ifdef PLATFORM_WINDOWS
        // Windows: 使用tpmtool或Get-TPM
        std::string command = "powershell \"Get-TPM\"";
#else
        // Unix-like: 使用tpm2-tools
        std::string command = "tpm2_startup -c";
#endif
        
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return false;
        }
        
        char buffer[128];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        
        pclose(pipe);
        
        // 解析输出
        if (result.find("TPM") != std::string::npos) {
            tpmData.tpmPresent = true;
            tpmData.detectionMethod = "System command";
            return true;
        }
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("System command TPM detection failed: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::PerformSelfTest() {
    try {
        if (!tpmData.tpmPresent) {
            Logger::Warn("No TPM available for self-test");
            return false;
        }
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss执行自检
            ESYS_CONTEXT *ctx = NULL;
            TSS2_TCTI_CONTEXT *tcti = NULL;
            TSS2_RC rc;
            
            // 初始化上下文...
            // TODO: 实现tpm2-tss自检
            
            return true;
        }
#endif
        
        // 使用系统命令执行自检
#ifdef PLATFORM_WINDOWS
        std::string command = "tpmtool selftest";
#else
        std::string command = "tpm2_selftest";
#endif
        
        int result = system(command.c_str());
        return result == 0;
    }
    catch (const std::exception& e) {
        Logger::Error("TPM self-test failed: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::GetCapabilities() {
    try {
        if (!tpmData.tpmPresent) {
            return false;
        }
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss获取能力
            ESYS_CONTEXT *ctx = NULL;
            TSS2_TCTI_CONTEXT *tcti = NULL;
            TSS2_RC rc;
            
            // 初始化上下文...
            // TODO: 实现tpm2-tss能力获取
            
            return true;
        }
#endif
        
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to get TPM capabilities: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::CheckHealthStatus() {
    try {
        if (!tpmData.tpmPresent) {
            return false;
        }
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss检查健康状态
            // TODO: 实现tpm2-tss健康检查
            tpmData.status = "OK";
            return true;
        }
#endif
        
        tpmData.status = "OK";
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to check TPM health: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::ResetTpm() {
    try {
        if (!tpmData.tpmPresent) {
            return false;
        }
        
        Logger::Warn("TPM reset operation requested - this may clear all keys");
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss重置TPM
            // TODO: 实现tpm2-tss重置
            return true;
        }
#endif
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to reset TPM: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::ClearTpm() {
    try {
        if (!tpmData.tpmPresent) {
            return false;
        }
        
        Logger::Warn("TPM clear operation requested - this will clear all keys");
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss清除TPM
            // TODO: 实现tpm2-tss清除
            return true;
        }
#endif
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to clear TPM: " + std::string(e.what()));
        return false;
    }
}

std::vector<uint8_t> CrossPlatformTpmInfo::GetPcrValue(uint32_t pcrIndex) {
    std::vector<uint8_t> result;
    
    try {
        if (!tpmData.tpmPresent) {
            return result;
        }
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss获取PCR值
            // TODO: 实现tpm2-tss PCR读取
        }
#endif
        
        return result;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to get PCR value: " + std::string(e.what()));
        return result;
    }
}

std::vector<uint8_t> CrossPlatformTpmInfo::GetRandom(uint32_t size) {
    std::vector<uint8_t> result(size);
    
    try {
        if (!tpmData.tpmPresent) {
            return result;
        }
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss获取随机数
            // TODO: 实现tpm2-tss随机数生成
        }
#endif
        
        return result;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to get random bytes: " + std::string(e.what()));
        return result;
    }
}

bool CrossPlatformTpmInfo::SealData(const std::vector<uint8_t>& data, 
                                      const std::vector<uint8_t>& pcrList,
                                      std::vector<uint8_t>& sealedData) {
    try {
        if (!tpmData.tpmPresent) {
            return false;
        }
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss密封数据
            // TODO: 实现tpm2-tss数据密封
            return true;
        }
#endif
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to seal data: " + std::string(e.what()));
        return false;
    }
}

bool CrossPlatformTpmInfo::UnsealData(const std::vector<uint8_t>& sealedData,
                                        const std::vector<uint8_t>& pcrList,
                                        std::vector<uint8_t>& data) {
    try {
        if (!tpmData.tpmPresent) {
            return false;
        }
        
#ifdef USE_TPM2_TSS
        if (tpm2TssAvailable) {
            // 使用tpm2-tss解封数据
            // TODO: 实现tpm2-tss数据解封
            return true;
        }
#endif
        
        return false;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to unseal data: " + std::string(e.what()));
        return false;
    }
}