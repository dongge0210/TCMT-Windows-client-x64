// DiskInfo.cpp
#include "DiskInfo.h"
#include "../Utils/WinUtils.h"
#include "../Utils/Logger.h"
#include "../Utils/WmiManager.h"
#include <wbemidl.h>
#include <comdef.h>
#include <shellapi.h> // SHGetFileInfoW fallback display name
#include <algorithm>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Shell32.lib")

DiskInfo::DiskInfo() { 
    Logger::Debug("DiskInfo: 初始化磁盘信息");
    QueryDrives(); 
}

void DiskInfo::QueryDrives() {
    Logger::Debug("DiskInfo: 开始查询驱动器信息");
    drives.clear();
    DWORD driveMask = GetLogicalDrives();
    if (driveMask == 0) { Logger::Error("GetLogicalDrives 失败"); return; }
    Logger::Debug("DiskInfo: 检测到驱动器掩码: " + std::to_string(driveMask));
    
    int validDriveCount = 0;
    for (int i = 0; i < 26; ++i) {
        if ((driveMask & (1 << i)) == 0) continue;
        char driveLetter = static_cast<char>('A' + i);
        if (driveLetter == 'A' || driveLetter == 'B') continue; // 跳过软驱
        std::wstring rootPath; rootPath.reserve(4); rootPath.push_back(static_cast<wchar_t>(L'A'+i)); rootPath.append(L":\\");
        Logger::Debug(L"DiskInfo: 检查驱动器 " + rootPath);
        
        UINT driveType = GetDriveTypeW(rootPath.c_str());
        if (!(driveType == DRIVE_FIXED || driveType == DRIVE_REMOVABLE)) {
            Logger::Debug(L"DiskInfo: 驱动器 " + rootPath + L" 类型不匹配，跳过");
            continue;
        }
        Logger::Debug(L"DiskInfo: 驱动器 " + rootPath + L" 类型匹配");
        
        ULARGE_INTEGER freeBytesAvailable{}; ULARGE_INTEGER totalBytes{}; ULARGE_INTEGER totalFreeBytes{};
        if (!GetDiskFreeSpaceExW(rootPath.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) { Logger::Warn(L"GetDiskFreeSpaceEx 失败: " + rootPath); continue; }
        if (totalBytes.QuadPart == 0) continue;
        DriveInfo info{}; info.letter = driveLetter; info.totalSize = totalBytes.QuadPart; info.freeSpace = totalFreeBytes.QuadPart; info.usedSpace = (totalBytes.QuadPart >= totalFreeBytes.QuadPart)? (totalBytes.QuadPart - totalFreeBytes.QuadPart):0ULL;
        Logger::Debug(L"DiskInfo: 驱动器 " + rootPath + L" 空间信息 - 总计: " + std::to_wstring(info.totalSize) + L", 可用: " + std::to_wstring(info.freeSpace));
        
        // 获取卷标 / 文件系统
        wchar_t volumeName[MAX_PATH + 1] = {0};
        wchar_t fileSystemName[MAX_PATH + 1] = {0};
        DWORD fsFlags = 0;
        BOOL gotInfo = GetVolumeInformationW(rootPath.c_str(), volumeName, MAX_PATH, nullptr, nullptr, &fsFlags, fileSystemName, MAX_PATH);
        if (!gotInfo) {
            info.label = L""; // 空表示未命名或获取失败
            info.fileSystem = L"未知";
            Logger::Warn(L"GetVolumeInformation 失败: " + rootPath);
        } else {
            info.label = volumeName;
            info.fileSystem = fileSystemName;
            Logger::Debug(L"DiskInfo: 驱动器 " + rootPath + L" 卷标: " + info.label + L", 文件系统: " + info.fileSystem);
        }
        // 若卷标仍为空，尝试通过卷 GUID 路径再次获取（更稳健）
        if (info.label.empty()) {
            wchar_t volumeGuidPath[MAX_PATH + 1] = {0};
            if (GetVolumeNameForVolumeMountPointW(rootPath.c_str(), volumeGuidPath, MAX_PATH)) {
                wchar_t volumeName2[MAX_PATH + 1] = {0};
                wchar_t fileSystemName2[MAX_PATH + 1] = {0};
                DWORD fsFlags2 = 0;
                if (GetVolumeInformationW(volumeGuidPath, volumeName2, MAX_PATH, nullptr, nullptr, &fsFlags2, fileSystemName2, MAX_PATH)) {
                    if (volumeName2[0] != L'\0') info.label = volumeName2;
                    if (fileSystemName2[0] != L'\0') info.fileSystem = fileSystemName2;
                    Logger::Debug(L"DiskInfo: 通过GUID路径获取驱动器 " + rootPath + L" 信息 - 卷标: " + info.label + L", 文件系统: " + info.fileSystem);
                }
            }
        }
        // 若仍为空，使用 Shell 友好显示名作为兜底（本地磁盘等）
        if (info.label.empty()) {
            SHFILEINFOW sfi{};
            if (SHGetFileInfoW(rootPath.c_str(), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES)) {
                if (sfi.szDisplayName[0] != L'\0') {
                    info.label = sfi.szDisplayName; // 例如：本地磁盘 (C:)
                    Logger::Debug(L"DiskInfo: 通过Shell获取驱动器 " + rootPath + L" 显示名称: " + info.label);
                }
            }
        }
        // 最后兜底
        if (info.label.empty()) { 
            info.label = L"未命名";
            Logger::Debug(L"DiskInfo: 驱动器 " + rootPath + L" 使用默认卷标: " + info.label);
        }
        if (info.fileSystem.empty()) { 
            info.fileSystem = L"未知";
            Logger::Debug(L"DiskInfo: 驱动器 " + rootPath + L" 使用默认文件系统: " + info.fileSystem);
        }

        drives.push_back(std::move(info));
        validDriveCount++;
        Logger::Debug(L"DiskInfo: 成功添加驱动器 " + rootPath);
    }
    std::sort(drives.begin(), drives.end(), [](const DriveInfo& a,const DriveInfo& b){return a.letter<b.letter;});
    Logger::Debug("DiskInfo: 驱动器查询完成，共找到 " + std::to_string(validDriveCount) + " 个有效驱动器");
}

void DiskInfo::Refresh() { 
    Logger::Debug("DiskInfo: 刷新磁盘信息");
    QueryDrives(); 
}
const std::vector<DriveInfo>& DiskInfo::GetDrives() const { return drives; }

std::vector<DiskData> DiskInfo::GetDisks() {
    Logger::Debug("DiskInfo: 获取磁盘数据，共 " + std::to_string(drives.size()) + " 个驱动器");
    std::vector<DiskData> disks; disks.reserve(drives.size());
    for (const auto& drive : drives) { 
        DiskData d; 
        d.letter=drive.letter; 
        d.totalSize=drive.totalSize; 
        d.freeSpace=drive.freeSpace; 
        d.usedSpace=drive.usedSpace; 
        d.label=WinUtils::WstringToString(drive.label); 
        d.fileSystem=WinUtils::WstringToString(drive.fileSystem); 
        disks.push_back(std::move(d)); 
        Logger::Debug("DiskInfo: 添加磁盘数据 - 驱动器 " + std::string(1, d.letter) + ": " + d.label);
    }
    Logger::Debug("DiskInfo: 磁盘数据获取完成，共 " + std::to_string(disks.size()) + " 个磁盘");
    return disks;
}

// ---------------- 物理磁盘 + 逻辑盘符映射实现合并 ----------------
static bool ParseDiskPartition(const std::wstring& text, int& diskIndexOut) {
    size_t posDisk = text.find(L"Disk #"); if (posDisk==std::wstring::npos) return false; posDisk += 6; if (posDisk>=text.size()) return false; int num=0; bool any=false; while (posDisk<text.size() && iswdigit(text[posDisk])) { any=true; num = num*10 + (text[posDisk]-L'0'); ++posDisk; } if(!any) return false; diskIndexOut = num; return true; }

void DiskInfo::CollectPhysicalDisks(WmiManager& wmi, const std::vector<DiskData>& logicalDisks, SystemInfo& sysInfo) {
    Logger::Debug("DiskInfo: 开始收集物理磁盘信息");
    IWbemServices* svc = wmi.GetWmiService(); if (!svc) { Logger::Warn("WMI 服务无效，跳过物理磁盘枚举"); return; }
    std::map<int,std::vector<char>> physicalIndexToLetters;
    // 1. Win32_DiskDriveToDiskPartition
    {
        Logger::Debug("DiskInfo: 查询 Win32_DiskDriveToDiskPartition");
        IEnumWbemClassObject* pEnum=nullptr; HRESULT hr=svc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_DiskDriveToDiskPartition"), WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,nullptr,&pEnum);
        if (SUCCEEDED(hr)&&pEnum){ IWbemClassObject* obj=nullptr; ULONG ret=0; int mappingCount = 0; while(pEnum->Next(WBEM_INFINITE,1,&obj,&ret)==S_OK){ VARIANT ant; VARIANT dep; VariantInit(&ant); VariantInit(&dep); if (SUCCEEDED(obj->Get(L"Antecedent",0,&ant,0,0)) && ant.vt==VT_BSTR && SUCCEEDED(obj->Get(L"Dependent",0,&dep,0,0)) && dep.vt==VT_BSTR){ int diskIdx=-1; if (ParseDiskPartition(dep.bstrVal,diskIdx)){ /* mapping captured via next query */ mappingCount++; } } VariantClear(&ant); VariantClear(&dep); obj->Release(); } pEnum->Release(); Logger::Debug("DiskInfo: Win32_DiskDriveToDiskPartition 查询完成，找到 " + std::to_string(mappingCount) + " 个映射"); } else { Logger::Warn("查询 Win32_DiskDriveToDiskPartition 失败"); }
    }
    // 2. Win32_LogicalDiskToPartition 建立盘符 -> 物理索引
    std::map<char,int> letterToDiskIndex; {
        Logger::Debug("DiskInfo: 查询 Win32_LogicalDiskToPartition");
        IEnumWbemClassObject* pEnum=nullptr; HRESULT hr=svc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_LogicalDiskToPartition"), WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,nullptr,&pEnum);
        if (SUCCEEDED(hr)&&pEnum){ IWbemClassObject* obj=nullptr; ULONG ret=0; int mappingCount = 0; while(pEnum->Next(WBEM_INFINITE,1,&obj,&ret)==S_OK){ VARIANT ant; VARIANT dep; VariantInit(&ant); VariantInit(&dep); if (SUCCEEDED(obj->Get(L"Antecedent",0,&ant,0,0)) && ant.vt==VT_BSTR && SUCCEEDED(obj->Get(L"Dependent",0,&dep,0,0)) && dep.vt==VT_BSTR){ int diskIdx=-1; if (ParseDiskPartition(ant.bstrVal,diskIdx)){ std::wstring depStr = dep.bstrVal; size_t pos = depStr.find(L"DeviceID=\""); if (pos!=std::wstring::npos){ pos+=10; if (pos<depStr.size()){ wchar_t letterW = depStr[pos]; if(letterW && letterW!=L'"'){ letterToDiskIndex[ static_cast<char>(::toupper(letterW)) ] = diskIdx; mappingCount++; } } } } } VariantClear(&ant); VariantClear(&dep); obj->Release(); } pEnum->Release(); Logger::Debug("DiskInfo: Win32_LogicalDiskToPartition 查询完成，找到 " + std::to_string(mappingCount) + " 个映射"); } else { Logger::Warn("查询 Win32_LogicalDiskToPartition 失败"); } }
    for (auto& kv: letterToDiskIndex) physicalIndexToLetters[kv.second].push_back(kv.first);
    Logger::Debug("DiskInfo: 盘符到物理磁盘索引映射完成，共 " + std::to_string(letterToDiskIndex.size()) + " 个映射");
    // 3. Win32_DiskDrive 基本信息
    std::map<int,PhysicalDiskSmartData> tempDisks; {
        Logger::Debug("DiskInfo: 查询 Win32_DiskDrive");
        IEnumWbemClassObject* pEnum=nullptr; HRESULT hr=svc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT Index,Model,SerialNumber,InterfaceType,Size,MediaType FROM Win32_DiskDrive"), WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,nullptr,&pEnum);
        if (SUCCEEDED(hr)&&pEnum){ IWbemClassObject* obj=nullptr; ULONG ret=0; int diskCount = 0; while(pEnum->Next(WBEM_INFINITE,1,&obj,&ret)==S_OK){ VARIANT vIndex,vModel,vSerial,vIface,vSize,vMedia; VariantInit(&vIndex);VariantInit(&vModel);VariantInit(&vSerial);VariantInit(&vIface);VariantInit(&vSize);VariantInit(&vMedia); if (SUCCEEDED(obj->Get(L"Index",0,&vIndex,0,0)) && (vIndex.vt==VT_I4||vIndex.vt==VT_UI4)){ int idx=(vIndex.vt==VT_I4)?vIndex.intVal:static_cast<int>(vIndex.uintVal); PhysicalDiskSmartData data{}; if (SUCCEEDED(obj->Get(L"Model",0,&vModel,0,0))&&vModel.vt==VT_BSTR) wcsncpy_s(data.model,vModel.bstrVal,_TRUNCATE); if (SUCCEEDED(obj->Get(L"SerialNumber",0,&vSerial,0,0))&&vSerial.vt==VT_BSTR) wcsncpy_s(data.serialNumber,vSerial.bstrVal,_TRUNCATE); if (SUCCEEDED(obj->Get(L"InterfaceType",0,&vIface,0,0))&&vIface.vt==VT_BSTR) wcsncpy_s(data.interfaceType,vIface.bstrVal,_TRUNCATE); if (SUCCEEDED(obj->Get(L"Size",0,&vSize,0,0))){ if (vSize.vt==VT_UI8) data.capacity=vSize.ullVal; else if (vSize.vt==VT_BSTR) data.capacity=_wcstoui64(vSize.bstrVal,nullptr,10);} if (SUCCEEDED(obj->Get(L"MediaType",0,&vMedia,0,0))&&vMedia.vt==VT_BSTR){ std::wstring media=vMedia.bstrVal; if (media.find(L"SSD")!=std::wstring::npos||media.find(L"Solid State")!=std::wstring::npos) wcsncpy_s(data.diskType,L"SSD",_TRUNCATE); else wcsncpy_s(data.diskType,L"HDD",_TRUNCATE);} else wcsncpy_s(data.diskType,L"未知",_TRUNCATE); data.smartSupported=false; data.smartEnabled=false; data.healthPercentage=0; data.temperature=0.0; data.logicalDriveCount=0; tempDisks[idx]=data; diskCount++; Logger::Debug(L"DiskInfo: 添加物理磁盘 - 索引: " + std::to_wstring(idx) + L", 型号: " + data.model); } VariantClear(&vIndex);VariantClear(&vModel);VariantClear(&vSerial);VariantClear(&vIface);VariantClear(&vSize);VariantClear(&vMedia); obj->Release(); } pEnum->Release(); Logger::Debug("DiskInfo: Win32_DiskDrive 查询完成，找到 " + std::to_string(diskCount) + " 个物理磁盘"); } else { Logger::Warn("查询 Win32_DiskDrive 失败"); } }
    // 4. 填充盘符
    int letterMappingCount = 0;
    for (auto& kv: physicalIndexToLetters){ int diskIdx=kv.first; auto it=tempDisks.find(diskIdx); if(it==tempDisks.end()) continue; auto& pd=it->second; int count=0; for(char L: kv.second){ if(count>=8) break; pd.logicalDriveLetters[count++]=L; } pd.logicalDriveCount=count; letterMappingCount += count; Logger::Debug(L"DiskInfo: 物理磁盘 " + std::to_wstring(diskIdx) + L" 关联了 " + std::to_wstring(count) + L" 个逻辑驱动器"); }
    Logger::Debug("DiskInfo: 物理磁盘到逻辑驱动器映射完成，共 " + std::to_string(letterMappingCount) + " 个映射");
    // 5. 写入 SystemInfo
    sysInfo.physicalDisks.clear(); for (auto& kv: tempDisks){ sysInfo.physicalDisks.push_back(kv.second); if (sysInfo.physicalDisks.size()>=8) break; }
    Logger::Debug(L"物理磁盘枚举完成: " + std::to_wstring(sysInfo.physicalDisks.size()) + L" 个");
}