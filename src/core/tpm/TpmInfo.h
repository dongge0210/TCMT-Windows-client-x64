#pragma once
#include "../common/PlatformDefs.h"
#include <string>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    class WmiManager;
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <IOKit/IOKitLib.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <unistd.h>
    #include <fstream>
    #include <sstream>
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
#endif

class TpmInfo {
public:
    struct TpmData {
        std::string manufacturerName;      // TPM 制造商
        std::string manufacturerId;        // 制造商ID
        std::string version;               // TPM版本
        std::string firmwareVersion;       // 固件版本
        bool isEnabled = false;             // TPM是否启用
        bool isActivated = false;           // TPM是否激活
        bool isOwned = false;               // TPM是否已拥有
        bool isReady = false;               // TPM是否就绪
        uint32_t specVersion = 0;           // TPM规范版本
        bool physicalPresenceRequired = false; // 是否需要物理存在
        std::string status;                // TPM状态
        
        // TBS (TPM Base Services) 信息 - Windows特有
        bool tbsAvailable = false;          // TBS是否可用
        uint32_t tbsVersion = 0;            // TBS版本
        std::string errorMessage;          // 错误信息（如果有）
        
        // 检测方法信息
        std::string detectionMethod;       // 使用的检测方法 ("TBS", "WMI", "TBS+WMI", "未检测到", "平台不支持")
        bool wmiDetectionWorked = false;    // WMI检测是否成功
        bool tbsDetectionWorked = false;    // TBS检测是否成功
        
        // 跨平台特有字段
        bool isSupported = false;           // 平台是否支持TPM
        std::string platformInfo;          // 平台特定信息
    };

#ifdef PLATFORM_WINDOWS
    TpmInfo(WmiManager& manager);
#else
    TpmInfo();
#endif
    ~TpmInfo();

    const TpmData& GetTpmData() const;
    bool HasTpm() const { return hasTpm; }

private:
    void DetectTpm();
    
#ifdef PLATFORM_WINDOWS
    void DetectTpmViaWmi();
    void DetectTpmViaTbs();
    void DetermineDetectionMethod(); // 确定使用的检测方法
    WmiManager* wmiManager;
#elif defined(PLATFORM_MACOS)
    void DetectMacTpm();
    bool CheckMacTpmSupport();
    std::string GetMacSystemIntegrityProtection();
#elif defined(PLATFORM_LINUX)
    void DetectLinuxTpm();
    bool CheckLinuxTpmDevice();
    std::string GetLinuxTpmVersion();
#endif
    
    TpmData tpmData;
    bool hasTpm = false;
};