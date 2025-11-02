#pragma once
#include "DataStruct.h"
#include <windows.h>
#include <string>
#include "../usb/USBInfo.h"
#include "../Utils/WmiManager.h"

// Shared memory management class to avoid multiple definitions
class SharedMemoryManager {
private:
    static HANDLE hMapFile;
    static SharedMemoryBlock* pBuffer;
    static std::string lastError; // Store last error message
    
    // 为MotherboardInfo提供WMI服务访问
    static IWbemServices* GetWmiService();
    
    // WMI管理器实例（用于主板信息采集）
    static WmiManager* wmiManager;
    
    // USB监控管理器实例
    static USBInfoManager* usbManager;

public:
    // Initialize shared memory
    static bool InitSharedMemory();

    // Write system info to shared memory
    static void WriteToSharedMemory(const SystemInfo& sysInfo);

    // Clean up shared memory resources
    static void CleanupSharedMemory();

    // Get buffer pointer (if needed)
    static SharedMemoryBlock* GetBuffer() { return pBuffer; }
    
    // Get last error message
    static std::string GetLastError();
};
