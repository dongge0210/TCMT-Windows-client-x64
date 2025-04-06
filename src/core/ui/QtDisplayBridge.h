// QtDisplayBridge.h
#pragma once

#include <string>
#include <vector>
#include <map>
#include <QApplication>
#include <QWidget>
#include <memory>
#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QTreeWidget>

// ϵͳ��Ϣ�ṹ�壬���ڴ���ϵͳ���ݵ�Qt����
struct SystemInfo {
    // CPU��Ϣ
    std::string cpuName;
    int physicalCores = 0;
    int logicalCores = 0;
    int performanceCores = 0;
    int efficiencyCores = 0;
    double cpuUsage = 0.0;
    bool hyperThreading = false;
    bool virtualization = false;
    double performanceCoreFreq = 0.0;
    double efficiencyCoreFreq = 0.0;

    // �ڴ���Ϣ
    uint64_t totalMemory = 0;
    uint64_t usedMemory = 0;
    uint64_t availableMemory = 0;

    // GPU��Ϣ
    std::string gpuName;
    std::string gpuBrand;
    uint64_t gpuMemory = 0;
    double gpuCoreFreq = 0.0;

    // �¶���Ϣ��������ƣ��¶�ֵ��
    std::vector<std::pair<std::string, double>> temperatures;

    // ������Ϣ�ṹ
    struct DiskInfo {
        char letter;
        std::string label;
        std::string fileSystem;
        uint64_t totalSize = 0;
        uint64_t usedSpace = 0;
        uint64_t freeSpace = 0;
    };
    std::vector<DiskInfo> disks;
};

// ǰ������Qt���Ӵ�����
class SystemMonitorWindow;

/**
 * @brief Qt��ʾ�Ž���
 *
 * �������Ϊϵͳ��غ�����Qt GUI֮���������
 * �����ʼ��Qt���������������Լ�������ʾ����
 */
class QtDisplayBridge {
public:
    /**
     * @brief ��ʼ��Qt����
     *
     * @param argc �����в�������
     * @param argv �����в�������
     * @return bool ��ʼ���Ƿ�ɹ�
     */
    static bool Initialize(int argc, char* argv[]);

    /**
     * @brief ����ϵͳ��ش���
     *
     * @return bool �����Ƿ�ɹ�
     */
    static bool CreateMonitorWindow();

    /**
     * @brief ����ϵͳ��Ϣ��Qt����
     *
     * @param sysInfo ϵͳ��Ϣ�ṹ��
     */
    static void UpdateSystemInfo(const SystemInfo& sysInfo);

    /**
     * @brief ���Qt�����Ƿ��ѳ�ʼ��
     *
     * @return bool �Ƿ��ѳ�ʼ��
     */
    static bool IsInitialized();

    /**
     * @brief ����Qt��Դ
     */
    static void Cleanup();

private:
    static QApplication* qtAppInstance;  // QtӦ�ó���ʵ��
    static SystemMonitorWindow* monitorWindow;  // ��ش���ʵ��
    static bool initialized;  // ��ʼ����־
};
