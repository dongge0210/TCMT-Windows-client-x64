// QtDisplayBridge.cpp
#include "QtDisplayBridge.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QGroupBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include "../utils/Logger.h"

// Qt�����������ඨ��
class SystemMonitorWindow : public QMainWindow {
public:
    SystemMonitorWindow(QWidget* parent = nullptr);
    void updateSystemInfo(const SystemInfo& sysInfo);

private:
    // CPU��Ϣ��ʾ���
    QLabel* cpuNameLabel;
    QLabel* cpuCoresLabel;
    QLabel* cpuThreadsLabel;
    QLabel* cpuPCoresLabel;
    QLabel* cpuECoresLabel;
    QLabel* cpuFeaturesLabel;
    QProgressBar* cpuUsageBar;
    QLabel* cpuPCoreFreqLabel;
    QLabel* cpuECoreFreqLabel;

    // �ڴ���Ϣ��ʾ���
    QProgressBar* memoryUsageBar;
    QLabel* memoryTotalLabel;
    QLabel* memoryUsedLabel;
    QLabel* memoryAvailableLabel;

    // GPU��Ϣ��ʾ���
    QLabel* gpuNameLabel;
    QLabel* gpuBrandLabel;
    QLabel* gpuMemoryLabel;
    QLabel* gpuFreqLabel;

    // �¶���Ϣ��ʾ
    QTreeWidget* temperatureTreeWidget;

    // ������Ϣ��ʾ
    QTreeWidget* diskTreeWidget;

    // ���½���ķ���
    void setupUi();
    void setupCpuInfo(QWidget* cpuTab);
    void setupMemoryInfo(QWidget* memoryTab);
    void setupGpuInfo(QWidget* gpuTab);
    void setupTemperatureInfo(QWidget* tempTab);
    void setupDiskInfo(QWidget* diskTab);
};

// ��ȷ���徲̬��Ա����
QApplication* QtDisplayBridge::qtAppInstance = nullptr;
SystemMonitorWindow* QtDisplayBridge::monitorWindow = nullptr;
bool QtDisplayBridge::initialized = false;

// SystemMonitorWindowʵ��
SystemMonitorWindow::SystemMonitorWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("ϵͳӲ�����");
    setMinimumSize(800, 600);
    setupUi();
}

void SystemMonitorWindow::setupUi() {
    // ������Ҫ���ֺ�ѡ�����
    QTabWidget* tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    // ��������ѡ�
    QWidget* cpuTab = new QWidget();
    QWidget* memoryTab = new QWidget();
    QWidget* gpuTab = new QWidget();
    QWidget* tempTab = new QWidget();
    QWidget* diskTab = new QWidget();

    // ���ø���ѡ�������
    setupCpuInfo(cpuTab);
    setupMemoryInfo(memoryTab);
    setupGpuInfo(gpuTab);
    setupTemperatureInfo(tempTab);
    setupDiskInfo(diskTab);

    // ���ѡ���ѡ�����
    tabWidget->addTab(cpuTab, "������");
    tabWidget->addTab(memoryTab, "�ڴ�");
    tabWidget->addTab(gpuTab, "�Կ�");
    tabWidget->addTab(tempTab, "�¶�");
    tabWidget->addTab(diskTab, "����");
}

void SystemMonitorWindow::setupCpuInfo(QWidget* cpuTab) {
    QVBoxLayout* layout = new QVBoxLayout(cpuTab);

    // CPU���ƺͻ�����Ϣ
    QGroupBox* infoGroup = new QGroupBox("CPU��Ϣ", cpuTab);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);

    cpuNameLabel = new QLabel("����: δ֪");
    cpuCoresLabel = new QLabel("���������: 0");
    cpuThreadsLabel = new QLabel("�߼��߳���: 0");
    cpuPCoresLabel = new QLabel("���ܺ�����: 0");
    cpuECoresLabel = new QLabel("��Ч������: 0");
    cpuFeaturesLabel = new QLabel("����֧��: δ֪");

    infoLayout->addWidget(cpuNameLabel);
    infoLayout->addWidget(cpuCoresLabel);
    infoLayout->addWidget(cpuThreadsLabel);
    infoLayout->addWidget(cpuPCoresLabel);
    infoLayout->addWidget(cpuECoresLabel);
    infoLayout->addWidget(cpuFeaturesLabel);

    // CPUʹ����
    QGroupBox* usageGroup = new QGroupBox("CPUʹ����", cpuTab);
    QVBoxLayout* usageLayout = new QVBoxLayout(usageGroup);

    cpuUsageBar = new QProgressBar();
    cpuUsageBar->setMinimum(0);
    cpuUsageBar->setMaximum(100);
    cpuUsageBar->setValue(0);
    cpuUsageBar->setFormat("%p%");

    usageLayout->addWidget(cpuUsageBar);

    // CPUƵ��
    QGroupBox* freqGroup = new QGroupBox("CPUƵ��", cpuTab);
    QVBoxLayout* freqLayout = new QVBoxLayout(freqGroup);

    cpuPCoreFreqLabel = new QLabel("���ܺ���Ƶ��: 0 GHz");
    cpuECoreFreqLabel = new QLabel("��Ч����Ƶ��: 0 GHz");

    freqLayout->addWidget(cpuPCoreFreqLabel);
    freqLayout->addWidget(cpuECoreFreqLabel);

    // ��������鵽������
    layout->addWidget(infoGroup);
    layout->addWidget(usageGroup);
    layout->addWidget(freqGroup);
    layout->addStretch(1);
}

void SystemMonitorWindow::setupMemoryInfo(QWidget* memoryTab) {
    QVBoxLayout* layout = new QVBoxLayout(memoryTab);

    // �ڴ�ʹ����
    QGroupBox* usageGroup = new QGroupBox("�ڴ�ʹ����", memoryTab);
    QVBoxLayout* usageLayout = new QVBoxLayout(usageGroup);

    memoryUsageBar = new QProgressBar();
    memoryUsageBar->setMinimum(0);
    memoryUsageBar->setMaximum(100);
    memoryUsageBar->setValue(0);
    memoryUsageBar->setFormat("%p%");

    usageLayout->addWidget(memoryUsageBar);

    // �ڴ���ϸ��Ϣ
    QGroupBox* detailGroup = new QGroupBox("�ڴ���ϸ��Ϣ", memoryTab);
    QVBoxLayout* detailLayout = new QVBoxLayout(detailGroup);

    memoryTotalLabel = new QLabel("���ڴ�: 0 GB");
    memoryUsedLabel = new QLabel("�����ڴ�: 0 GB");
    memoryAvailableLabel = new QLabel("�����ڴ�: 0 GB");

    detailLayout->addWidget(memoryTotalLabel);
    detailLayout->addWidget(memoryUsedLabel);
    detailLayout->addWidget(memoryAvailableLabel);

    // ��������鵽������
    layout->addWidget(usageGroup);
    layout->addWidget(detailGroup);
    layout->addStretch(1);
}

void SystemMonitorWindow::setupGpuInfo(QWidget* gpuTab) {
    QVBoxLayout* layout = new QVBoxLayout(gpuTab);

    // GPU������Ϣ
    QGroupBox* infoGroup = new QGroupBox("GPU��Ϣ", gpuTab);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);

    gpuNameLabel = new QLabel("����: δ֪");
    gpuBrandLabel = new QLabel("Ʒ��: δ֪");
    gpuMemoryLabel = new QLabel("�Դ�: 0 GB");
    gpuFreqLabel = new QLabel("Ƶ��: 0 MHz");

    infoLayout->addWidget(gpuNameLabel);
    infoLayout->addWidget(gpuBrandLabel);
    infoLayout->addWidget(gpuMemoryLabel);
    infoLayout->addWidget(gpuFreqLabel);

    // ��������鵽������
    layout->addWidget(infoGroup);
    layout->addStretch(1);
}

void SystemMonitorWindow::setupTemperatureInfo(QWidget* tempTab) {
    QVBoxLayout* layout = new QVBoxLayout(tempTab);

    // �¶���Ϣ���οؼ�
    temperatureTreeWidget = new QTreeWidget(tempTab);
    temperatureTreeWidget->setHeaderLabels(QStringList() << "���" << "�¶�");
    temperatureTreeWidget->setColumnWidth(0, 250);
    temperatureTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    layout->addWidget(temperatureTreeWidget);
}

void SystemMonitorWindow::setupDiskInfo(QWidget* diskTab) {
    QVBoxLayout* layout = new QVBoxLayout(diskTab);

    // ������Ϣ���οؼ�
    diskTreeWidget = new QTreeWidget(diskTab);
    diskTreeWidget->setHeaderLabels(QStringList() << "������" << "���" << "�ļ�ϵͳ" << "������" << "���ÿռ�" << "���ÿռ�" << "ʹ����");
    diskTreeWidget->setColumnWidth(0, 80);
    diskTreeWidget->setColumnWidth(1, 150);
    diskTreeWidget->setColumnWidth(2, 80);
    diskTreeWidget->setColumnWidth(3, 100);
    diskTreeWidget->setColumnWidth(4, 100);
    diskTreeWidget->setColumnWidth(5, 100);
    diskTreeWidget->setColumnWidth(6, 80);
    diskTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    layout->addWidget(diskTreeWidget);
}

void SystemMonitorWindow::updateSystemInfo(const SystemInfo& sysInfo) {
    // ����CPU��Ϣ
    cpuNameLabel->setText(QString("����: ") + QString::fromStdString(sysInfo.cpuName));
    cpuCoresLabel->setText(QString("���������: ") + QString::number(sysInfo.physicalCores));
    cpuThreadsLabel->setText(QString("�߼��߳���: ") + QString::number(sysInfo.logicalCores));
    cpuPCoresLabel->setText(QString("���ܺ�����: ") + QString::number(sysInfo.performanceCores));
    cpuECoresLabel->setText(QString("��Ч������: ") + QString::number(sysInfo.efficiencyCores));

    std::string features = "����֧��: ";
    features += sysInfo.hyperThreading ? "���߳�: ��" : "���߳�: ��";
    features += ", ";
    features += sysInfo.virtualization ? "���⻯: ��" : "���⻯: ��";
    cpuFeaturesLabel->setText(features.c_str());

    cpuUsageBar->setValue(static_cast<int>(sysInfo.cpuUsage));

    char pFreqBuf[32], eFreqBuf[32];
    snprintf(pFreqBuf, sizeof(pFreqBuf), "���ܺ���Ƶ��: %.2f GHz", sysInfo.performanceCoreFreq / 1000.0);
    snprintf(eFreqBuf, sizeof(eFreqBuf), "��Ч����Ƶ��: %.2f GHz", sysInfo.efficiencyCoreFreq / 1000.0);
    cpuPCoreFreqLabel->setText(pFreqBuf);
    cpuECoreFreqLabel->setText(eFreqBuf);

    // �����ڴ���Ϣ
    double memUsagePercent = 0;
    if (sysInfo.totalMemory > 0) {
        memUsagePercent = (static_cast<double>(sysInfo.usedMemory) / sysInfo.totalMemory) * 100.0;
    }
    memoryUsageBar->setValue(static_cast<int>(memUsagePercent));

    char memBuf[3][64];
    snprintf(memBuf[0], sizeof(memBuf[0]), "���ڴ�: %.2f GB", sysInfo.totalMemory / (1024.0 * 1024.0 * 1024.0));
    snprintf(memBuf[1], sizeof(memBuf[1]), "�����ڴ�: %.2f GB", sysInfo.usedMemory / (1024.0 * 1024.0 * 1024.0));
    snprintf(memBuf[2], sizeof(memBuf[2]), "�����ڴ�: %.2f GB", sysInfo.availableMemory / (1024.0 * 1024.0 * 1024.0));

    memoryTotalLabel->setText(memBuf[0]);
    memoryUsedLabel->setText(memBuf[1]);
    memoryAvailableLabel->setText(memBuf[2]);

    // ����GPU��Ϣ
    gpuNameLabel->setText(QString("����: ") + QString::fromStdString(sysInfo.gpuName));
    gpuBrandLabel->setText(QString("Ʒ��: ") + QString::fromStdString(sysInfo.gpuBrand));

    char gpuMemBuf[64], gpuFreqBuf[64];
    snprintf(gpuMemBuf, sizeof(gpuMemBuf), "�Դ�: %.2f GB", sysInfo.gpuMemory / (1024.0 * 1024.0 * 1024.0));
    snprintf(gpuFreqBuf, sizeof(gpuFreqBuf), "Ƶ��: %.0f MHz", sysInfo.gpuCoreFreq);

    gpuMemoryLabel->setText(gpuMemBuf);
    gpuFreqLabel->setText(gpuFreqBuf);

    // �����¶���Ϣ
    temperatureTreeWidget->clear();
    for (const auto& temp : sysInfo.temperatures) {
        QTreeWidgetItem* item = new QTreeWidgetItem(temperatureTreeWidget);
        item->setText(0, temp.first.c_str());

        char tempBuf[16];
        snprintf(tempBuf, sizeof(tempBuf), "%.1f ��C", temp.second);
        item->setText(1, tempBuf);

        // Ϊ�������ú�ɫ����
        if (temp.second > 80.0) {
            item->setForeground(1, Qt::red);
        }
        else if (temp.second > 70.0) {
            item->setForeground(1, QColor(255, 165, 0)); // ��ɫ
        }
    }

    // ���´�����Ϣ
    diskTreeWidget->clear();
    for (const auto& disk : sysInfo.disks) {
        QTreeWidgetItem* item = new QTreeWidgetItem(diskTreeWidget);

        char driveBuf[8];
        snprintf(driveBuf, sizeof(driveBuf), "%c:", disk.letter);
        item->setText(0, driveBuf);
        item->setText(1, disk.label.c_str());
        item->setText(2, disk.fileSystem.c_str());

        char sizeBufs[4][32];
        snprintf(sizeBufs[0], sizeof(sizeBufs[0]), "%.2f GB", disk.totalSize / (1024.0 * 1024.0 * 1024.0));
        snprintf(sizeBufs[1], sizeof(sizeBufs[1]), "%.2f GB", disk.usedSpace / (1024.0 * 1024.0 * 1024.0));
        snprintf(sizeBufs[2], sizeof(sizeBufs[2]), "%.2f GB", disk.freeSpace / (1024.0 * 1024.0 * 1024.0));

        double usagePercent = 0;
        if (disk.totalSize > 0) {
            usagePercent = (static_cast<double>(disk.usedSpace) / disk.totalSize) * 100.0;
        }
        snprintf(sizeBufs[3], sizeof(sizeBufs[3]), "%.1f%%", usagePercent);

        item->setText(3, sizeBufs[0]);
        item->setText(4, sizeBufs[1]);
        item->setText(5, sizeBufs[2]);
        item->setText(6, sizeBufs[3]);

        // Ϊ�����Ĵ������þ���ɫ
        if (usagePercent > 90.0) {
            item->setForeground(6, Qt::red);
        }
        else if (usagePercent > 75.0) {
            item->setForeground(6, QColor(255, 165, 0)); // ��ɫ
        }
    }
}

// QtDisplayBridgeʵ��
bool QtDisplayBridge::Initialize(int argc, char* argv[]) {
    try {
        if (initialized) {
            Logger::Warning("Qt�����Ѿ���ʼ��");
            return true;
        }

        // ����QtӦ�ó���ʵ��
        qtAppInstance = new QApplication(argc, argv);
        if (!qtAppInstance) {
            Logger::Error("�޷�����QtӦ�ó���ʵ��");
            return false;
        }

        // ����Ӧ�ó��������Ϣ
        QApplication::setApplicationName("ϵͳӲ�����");
        QApplication::setOrganizationName("Ӳ����ع���");

        initialized = true;
        Logger::Info("Qt������ʼ���ɹ�");
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("Qt������ʼ��ʧ��: " + std::string(e.what()));
        return false;
    }
}

bool QtDisplayBridge::CreateMonitorWindow() {
    try {
        if (!initialized) {
            Logger::Error("���Դ�������ǰ���ȳ�ʼ��Qt����");
            return false;
        }

        if (monitorWindow) {
            Logger::Warning("��ش����Ѿ�����");
            return true;
        }

        // ��������ʾ��ش���
        monitorWindow = new SystemMonitorWindow();
        monitorWindow->show();

        Logger::Info("��ش��ڴ����ɹ�");
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("��ش��ڴ���ʧ��: " + std::string(e.what()));
        return false;
    }
}

void QtDisplayBridge::UpdateSystemInfo(const SystemInfo& sysInfo) {
    if (!initialized || !monitorWindow) {
        return;
    }

    // ͨ��Qt�¼�ѭ����ȫ�ظ���UI
    auto* win = monitorWindow; // capture a local pointer for the lambda
    QMetaObject::invokeMethod(win, [sysInfo, win]() {
        win->updateSystemInfo(sysInfo);
    }, Qt::QueuedConnection);
}

bool QtDisplayBridge::IsInitialized() {
    return initialized;
}

void QtDisplayBridge::Cleanup() {
    // �����ں�Ӧ�ó���ʵ��
    if (monitorWindow) {
        monitorWindow->close();
        delete monitorWindow;
        monitorWindow = nullptr;
    }

    if (qtAppInstance) {
        // ����Ҫ��ʽɾ��qtAppInstance��������Ӧ�ó������ʱ�Զ�����
        qtAppInstance = nullptr;
    }

    initialized = false;
    Logger::Info("Qt��Դ������");
}
