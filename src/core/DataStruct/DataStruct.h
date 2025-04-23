// DataStruct.h
#pragma once
#include <windows.h>
#include <string>
#include <vector>

#pragma pack(push, 1) // ȷ���ڴ����

// GPU��Ϣ
struct GPUData {
    wchar_t name[128];    // GPU����
    wchar_t brand[64];    // Ʒ��
    uint64_t memory;      // �Դ棨�ֽڣ�
    double coreClock;     // ����Ƶ�ʣ�MHz��
};

// ������������Ϣ
struct NetworkAdapterData {
    wchar_t name[128];    // ����������
    wchar_t mac[32];      // MAC��ַ
    uint64_t speed;       // �ٶȣ�bps��
};

// ������Ϣ
struct DiskData {
    wchar_t letter;       // �̷�����L'C'��
    wchar_t label[128];   // ���
    wchar_t fileSystem[64];// �ļ�ϵͳ
    uint64_t totalSize;   // ���������ֽڣ�
    uint64_t usedSpace;   // ���ÿռ䣨�ֽڣ�
    uint64_t freeSpace;   // ���ÿռ䣨�ֽڣ�
    // Only modify the DiskData structure to add freeSpace field
    struct DiskData {
        wchar_t letter;       // �̷�����L'C'��
        wchar_t label[128];   // ���
        wchar_t fileSystem[64];// �ļ�ϵͳ
        uint64_t totalSize;   // ���������ֽڣ�
        uint64_t usedSpace;   // ���ÿռ䣨�ֽڣ�
        uint64_t freeSpace;   // ���ÿռ䣨�ֽڣ�
    };
};

// �¶ȴ�������Ϣ
struct TemperatureData {
    wchar_t sensorName[64]; // ����������
    double temperature;     // �¶ȣ����϶ȣ�
};

// SystemInfo�ṹ
struct SystemInfo {
    std::string cpuName;
    int physicalCores;
    int logicalCores;
    double cpuUsage;
    int performanceCores;
    int efficiencyCores;
    double performanceCoreFreq;
    double efficiencyCoreFreq;
    bool hyperThreading;
    bool virtualization;
    uint64_t totalMemory;
    uint64_t usedMemory;
    uint64_t availableMemory;
    std::vector<GPUData> gpus;
    std::vector<NetworkAdapterData> adapters;
    std::vector<DiskData> disks;
    std::vector<std::pair<std::string, double>> temperatures;
    std::string osVersion;          // Added
    std::string networkAdapterName; // Added
    std::string networkAdapterMac;  // Added
    uint64_t networkAdapterSpeed;   // Added
    std::string gpuName;            // Added
    uint64_t gpuMemory;             // Added
    double gpuCoreFreq;             // Added
    std::string gpuBrand;           // Added
    SYSTEMTIME lastUpdate;
};

// �����ڴ����ṹ
struct SharedMemoryBlock {
    char cpuName[128];        // CPU����
    int physicalCores;        // ���������
    int logicalCores;         // �߼�������
    float cpuUsage;           // CPUʹ���ʣ��ٷֱȣ�
    int performanceCores;     // ���ܺ�����
    int efficiencyCores;      // ��Ч������
    double pCoreFreq;         // ���ܺ���Ƶ�ʣ�GHz��
    double eCoreFreq;         // ��Ч����Ƶ�ʣ�GHz��
    bool hyperThreading;      // ���߳��Ƿ�����
    bool virtualization;      // ���⻯�Ƿ�����
    uint64_t totalMemory;     // ���ڴ棨�ֽڣ�
    uint64_t usedMemory;      // �����ڴ棨�ֽڣ�
    uint64_t availableMemory; // �����ڴ棨�ֽڣ�

    // GPU��Ϣ��֧�����2��GPU��
    GPUData gpus[2];

    // ������������֧�����4����������
    NetworkAdapterData adapters[4];

    // ������Ϣ��֧�����8�����̣�
    DiskData disks[8];

    // �¶����ݣ�֧��10����������
    TemperatureData temperatures[10];

    int adapterCount;
    int tempCount;
    int gpuCount;
    int diskCount;
    SYSTEMTIME lastUpdate;
    CRITICAL_SECTION lock;
};
#pragma pack(pop)