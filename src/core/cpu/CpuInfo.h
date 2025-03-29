#pragma once
#include <string>
#include <windows.h>
#include <pdh.h>
#include <queue>
#include <vector>

class CpuInfo {
public:
    CpuInfo();
    ~CpuInfo();

    double GetUsage();
    std::string GetName();
    int GetTotalCores() const;
    int GetSmallCores() const;
    int GetLargeCores() const;
    double GetLargeCoreSpeed() const;    // ��������ȡ���ܺ���Ƶ��
    double GetSmallCoreSpeed() const;    // ��������ȡ��Ч����Ƶ��
    DWORD GetCurrentSpeed() const;       // ���ּ�����
    bool IsHyperThreadingEnabled() const;
    bool IsVirtualizationEnabled() const;

private:
    void DetectCores();
    void InitializeCounter();
    void CleanupCounter();
    void UpdateCoreSpeeds();             // ���������º���Ƶ��
    std::string GetNameFromRegistry();
    double updateUsage();

    // ������Ϣ
    std::string cpuName;
    int totalCores;
    int smallCores;
    int largeCores;
    double cpuUsage;

    // Ƶ����Ϣ
    std::vector<DWORD> largeCoresSpeeds; // ���ܺ���Ƶ��
    std::vector<DWORD> smallCoresSpeeds; // ��Ч����Ƶ��
    DWORD lastUpdateTime;                // �ϴθ���ʱ��

    // PDH ���������
    PDH_HQUERY queryHandle;
    PDH_HCOUNTER counterHandle;
    bool counterInitialized;
};
