// DiskInfo.h
#pragma once
#include "../common/PlatformDefs.h"
#include <string>
#include <vector>
#include <map>
#include "../DataStruct/DataStruct.h"

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    class WmiManager; // 前向声明，避免头文件依赖膨胀
#elif defined(PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <sys/mount.h>
    #include <diskarbitration/diskarbitration.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/storage/IOBlockStorageDevice.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <unistd.h>
#elif defined(PLATFORM_LINUX)
    #include <sys/statvfs.h>
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
#endif

struct DriveInfo {
    char letter;  // Windows驱动器字母，macOS/Linux上为0
    std::string mountPoint;  // macOS/Linux挂载点
    std::string devicePath;   // 设备路径（如 /dev/disk1s1）
    uint64_t totalSize;
    uint64_t freeSpace;
    uint64_t usedSpace;
    std::string label;
    std::string fileSystem;
    bool isRemovable;
    bool isSSD;
    std::string interfaceType;  // SATA, NVMe, USB等
    
    // macOS特有字段
    std::string diskUUID;
    std::string volumeUUID;
    bool isAPFS;
    bool isEncrypted;
};

class DiskInfo {
public:
    DiskInfo(); // 无参数构造
    ~DiskInfo();
    
    const std::vector<DriveInfo>& GetDrives() const;
    void Refresh();
    std::vector<DiskData> GetDisks(); // 返回所有逻辑磁盘信息

#ifdef PLATFORM_WINDOWS
    // Windows特有：收集物理磁盘及逻辑盘符映射
    static void CollectPhysicalDisks(WmiManager& wmi, const std::vector<DiskData>& logicalDisks, SystemInfo& sysInfo);
#endif

private:
    void QueryDrives();
    
#ifdef PLATFORM_WINDOWS
    void QueryWindowsDrives();
#elif defined(PLATFORM_MACOS)
    void QueryMacDrives();
    void GetMacDiskProperties(const std::string& devicePath, DriveInfo& info) const;
    bool IsMacSSD(const std::string& devicePath) const;
    std::string GetMacFileSystem(const std::string& mountPoint) const;
    std::string GetMacDiskLabel(const std::string& mountPoint) const;
    void GetMacDiskPerformance(const std::string& devicePath, DriveInfo& info) const;
#elif defined(PLATFORM_LINUX)
    void QueryLinuxDrives();
    bool IsLinuxSSD(const std::string& devicePath) const;
    std::string GetLinuxFileSystem(const std::string& mountPoint) const;
    std::string GetLinuxDiskLabel(const std::string& devicePath) const;
#endif

    std::vector<DriveInfo> drives;
    bool initialized;
};

