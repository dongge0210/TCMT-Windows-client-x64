#pragma once
#include "../common/PlatformDefs.h"
#include <string>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    typedef DWORD DWORD;
    typedef unsigned short WORD;
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <sys/utsname.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <unistd.h>
    typedef unsigned long DWORD;
    typedef unsigned short WORD;
#elif defined(PLATFORM_LINUX)
    #include <sys/sysinfo.h>
    #include <sys/utsname.h>
    #include <unistd.h>
    typedef unsigned long DWORD;
    typedef unsigned short WORD;
#endif

class OSInfo {
public:
    OSInfo();
    std::string GetVersion() const;
    std::string GetKernelVersion() const;
    std::string GetArchitecture() const;
    std::string GetHostname() const;
    std::string GetSystemUptime() const;
    std::string GetBootTime() const;
    void Initialize();
    
    // 跨平台特有方法
#ifdef PLATFORM_MACOS
    std::string GetMacModel() const;
    std::string GetMacSerialNumber() const;
    std::string GetMacUUID() const;
    std::string GetMacSIPStatus() const;
    bool IsMacSIPEnabled() const;
    bool IsMacSecureBootEnabled() const;
    std::string GetMacSystemIntegrityProtectionStatus() const;
    std::string GetMacSecureEnclaveVersion() const;
#elif defined(PLATFORM_LINUX)
    std::string GetLinuxDistribution() const;
    std::string GetLinuxKernelVersion() const;
    std::string GetLinuxDesktopEnvironment() const;
    std::string GetLinuxPackageManager() const;
#endif

private:
    std::string osVersion;
    std::string kernelVersion;
    std::string architecture;
    std::string hostname;
    std::string systemUptime;
    std::string bootTime;
    bool initialized;
    
    // Windows特有字段
#ifdef PLATFORM_WINDOWS
    DWORD majorVersion;
    DWORD minorVersion;
    DWORD buildNumber;
#endif
    
    // 跨平台私有方法
    void DetectSystemInfo();
    
#ifdef PLATFORM_WINDOWS
    void DetectWindowsSystemInfo();
#elif defined(PLATFORM_MACOS)
    void DetectMacSystemInfo();
    std::string GetMacSystemReport();
    void GetMacHardwareInfo();
    void GetMacSoftwareInfo();
    void GetMacSecurityInfo();
#elif defined(PLATFORM_LINUX)
    void DetectLinuxSystemInfo();
    void GetLinuxHardwareInfo();
    void GetLinuxSoftwareInfo();
    void GetLinuxSecurityInfo();
#endif
};
