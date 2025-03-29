#pragma once
#include <string>
#include <windows.h>
#include <pdh.h>
#include <queue>

class CpuInfo {
public:
    CpuInfo();
    ~CpuInfo();  // ���������������

    double GetUsage();
    std::string GetName();
    int GetTotalCores() const;
    int GetSmallCores() const;
    int GetLargeCores() const;
    DWORD GetCurrentSpeed() const;
    bool IsHyperThreadingEnabled() const;
    bool IsVirtualizationEnabled() const;

private:
    void DetectCores();
    void InitializeCounter();    // ����
    void CleanupCounter();       // ����
    std::string GetNameFromRegistry();
    double updateUsage();

    // ������Ϣ
    std::string cpuName;
    int totalCores;
    int smallCores;
    int largeCores;
    double cpuUsage;

    // PDH ���������
    PDH_HQUERY queryHandle;
    PDH_HCOUNTER counterHandle;
    bool counterInitialized;
};
