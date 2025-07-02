// DiskInfo.h
#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include "../DataStruct/DataStruct.h"

struct DriveInfo {
    char letter;
    uint64_t totalSize;
    uint64_t freeSpace;
    uint64_t usedSpace;
    std::wstring label;
    std::wstring fileSystem;
};

class DiskInfo {
public:
    DiskInfo(); // �޲�������
    const std::vector<DriveInfo>& GetDrives() const;
    void Refresh();
    std::vector<DiskData> GetDisks(); // �������д�����Ϣ

private:
    void QueryDrives();
    std::vector<DriveInfo> drives;
};

