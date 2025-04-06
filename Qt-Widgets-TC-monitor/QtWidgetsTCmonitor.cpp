#include "stdafx.h"
#include "QtWidgetsTCmonitor.h"
#include "ui_QtWidgetsTCmonitor.h"  // ȷ�����ļ�����

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPainter>
#include <QTimer>
#include <queue>
#include <sstream>
#include <iomanip>

QtWidgetsTCmonitor::QtWidgetsTCmonitor(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::QtWidgetsTCmonitorClass)
{
    ui->setupUi(this);

    // Set window properties
    setWindowTitle(tr("ϵͳӲ��������"));
    resize(800, 600);

    // Initialize UI
    setupUI();

    // Set up update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &QtWidgetsTCmonitor::updateCharts);
    updateTimer->start(1000); // Update once per second
}

QtWidgetsTCmonitor::~QtWidgetsTCmonitor()
{
    if (updateTimer) {
        updateTimer->stop();
        delete updateTimer;
    }
    delete ui;
}

void QtWidgetsTCmonitor::setupUI()
{
    // Create main scroll area
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    setCentralWidget(scrollArea);

    // Create main container
    QWidget* container = new QWidget(scrollArea);
    scrollArea->setWidget(container);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(container);

    // Create sections
    createCpuSection();
    createMemorySection();
    createGpuSection();
    createTemperatureSection();
    createDiskSection();

    // Add to main layout
    mainLayout->addWidget(cpuGroupBox);
    mainLayout->addWidget(memoryGroupBox);
    mainLayout->addWidget(gpuGroupBox);
    mainLayout->addWidget(temperatureGroupBox);
    mainLayout->addWidget(diskGroupBox);
    mainLayout->addStretch();
}

void QtWidgetsTCmonitor::createCpuSection()
{
    cpuGroupBox = new QGroupBox(tr("��������Ϣ"), this);
    QGridLayout* layout = new QGridLayout(cpuGroupBox);

    // Add labels
    int row = 0;
    layout->addWidget(new QLabel(tr("����:"), this), row, 0);
    infoLabels["cpuName"] = new QLabel(this);
    layout->addWidget(infoLabels["cpuName"], row++, 1);

    layout->addWidget(new QLabel(tr("�������:"), this), row, 0);
    infoLabels["physicalCores"] = new QLabel(this);
    layout->addWidget(infoLabels["physicalCores"], row++, 1);

    layout->addWidget(new QLabel(tr("�߼�����:"), this), row, 0);
    infoLabels["logicalCores"] = new QLabel(this);
    layout->addWidget(infoLabels["logicalCores"], row++, 1);

    layout->addWidget(new QLabel(tr("���ܺ���:"), this), row, 0);
    infoLabels["performanceCores"] = new QLabel(this);
    layout->addWidget(infoLabels["performanceCores"], row++, 1);

    layout->addWidget(new QLabel(tr("��Ч����:"), this), row, 0);
    infoLabels["efficiencyCores"] = new QLabel(this);
    layout->addWidget(infoLabels["efficiencyCores"], row++, 1);

    layout->addWidget(new QLabel(tr("CPUʹ����:"), this), row, 0);
    infoLabels["cpuUsage"] = new QLabel(this);
    layout->addWidget(infoLabels["cpuUsage"], row++, 1);

    layout->addWidget(new QLabel(tr("���߳�:"), this), row, 0);
    infoLabels["hyperThreading"] = new QLabel(this);
    layout->addWidget(infoLabels["hyperThreading"], row++, 1);

    layout->addWidget(new QLabel(tr("���⻯:"), this), row, 0);
    infoLabels["virtualization"] = new QLabel(this);
    layout->addWidget(infoLabels["virtualization"], row++, 1);
}

void QtWidgetsTCmonitor::createMemorySection()
{
    memoryGroupBox = new QGroupBox(tr("�ڴ���Ϣ"), this);
    QGridLayout* layout = new QGridLayout(memoryGroupBox);

    int row = 0;
    layout->addWidget(new QLabel(tr("���ڴ�:"), this), row, 0);
    infoLabels["totalMemory"] = new QLabel(this);
    layout->addWidget(infoLabels["totalMemory"], row++, 1);

    layout->addWidget(new QLabel(tr("�����ڴ�:"), this), row, 0);
    infoLabels["usedMemory"] = new QLabel(this);
    layout->addWidget(infoLabels["usedMemory"], row++, 1);

    layout->addWidget(new QLabel(tr("�����ڴ�:"), this), row, 0);
    infoLabels["availableMemory"] = new QLabel(this);
    layout->addWidget(infoLabels["availableMemory"], row++, 1);

    layout->addWidget(new QLabel(tr("�ڴ�ʹ����:"), this), row, 0);
    infoLabels["memoryUsage"] = new QLabel(this);
    layout->addWidget(infoLabels["memoryUsage"], row++, 1);
}

void QtWidgetsTCmonitor::createGpuSection()
{
    gpuGroupBox = new QGroupBox(tr("�Կ���Ϣ"), this);
    QGridLayout* layout = new QGridLayout(gpuGroupBox);

    int row = 0;
    layout->addWidget(new QLabel(tr("����:"), this), row, 0);
    infoLabels["gpuName"] = new QLabel(this);
    layout->addWidget(infoLabels["gpuName"], row++, 1);

    layout->addWidget(new QLabel(tr("Ʒ��:"), this), row, 0);
    infoLabels["gpuBrand"] = new QLabel(this);
    layout->addWidget(infoLabels["gpuBrand"], row++, 1);

    layout->addWidget(new QLabel(tr("�Դ�:"), this), row, 0);
    infoLabels["gpuMemory"] = new QLabel(this);
    layout->addWidget(infoLabels["gpuMemory"], row++, 1);

    layout->addWidget(new QLabel(tr("����Ƶ��:"), this), row, 0);
    infoLabels["gpuCoreFreq"] = new QLabel(this);
    layout->addWidget(infoLabels["gpuCoreFreq"], row++, 1);
}

void QtWidgetsTCmonitor::createTemperatureSection()
{
    temperatureGroupBox = new QGroupBox(tr("�¶ȼ��"), this);
    QVBoxLayout* layout = new QVBoxLayout(temperatureGroupBox);

    // Temperature labels
    QGridLayout* tempLabelsLayout = new QGridLayout();
    tempLabelsLayout->addWidget(new QLabel(tr("CPU�¶�:"), this), 0, 0);
    infoLabels["cpuTemp"] = new QLabel(this);
    tempLabelsLayout->addWidget(infoLabels["cpuTemp"], 0, 1);

    tempLabelsLayout->addWidget(new QLabel(tr("GPU�¶�:"), this), 1, 0);
    infoLabels["gpuTemp"] = new QLabel(this);
    tempLabelsLayout->addWidget(infoLabels["gpuTemp"], 1, 1);

    layout->addLayout(tempLabelsLayout);

    // Create charts
    // CPU temperature chart
    cpuTempChart = new QChart();
    cpuTempChart->setTitle(tr("CPU�¶���ʷ"));
    cpuTempSeries = new QLineSeries();
    cpuTempChart->addSeries(cpuTempSeries);

    // Set up axes
    QValueAxis* axisX = new QValueAxis();
    axisX->setRange(0, MAX_DATA_POINTS);
    axisX->setLabelFormat("%d");
    axisX->setTitleText(tr("ʱ�� (��)"));

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setLabelFormat("%d");
    axisY->setTitleText(tr("�¶� (��C)"));

    cpuTempChart->addAxis(axisX, Qt::AlignBottom);
    cpuTempChart->addAxis(axisY, Qt::AlignLeft);
    cpuTempSeries->attachAxis(axisX);
    cpuTempSeries->attachAxis(axisY);

    cpuTempChartView = new QChartView(cpuTempChart);
    cpuTempChartView->setRenderHint(QPainter::Antialiasing);

    // GPU temperature chart
    gpuTempChart = new QChart();
    gpuTempChart->setTitle(tr("GPU�¶���ʷ"));
    gpuTempSeries = new QLineSeries();
    gpuTempChart->addSeries(gpuTempSeries);

    QValueAxis* axisX2 = new QValueAxis();
    axisX2->setRange(0, MAX_DATA_POINTS);
    axisX2->setLabelFormat("%d");
    axisX2->setTitleText(tr("ʱ�� (��)"));

    QValueAxis* axisY2 = new QValueAxis();
    axisY2->setRange(0, 100);
    axisY2->setLabelFormat("%d");
    axisY2->setTitleText(tr("�¶� (��C)"));

    gpuTempChart->addAxis(axisX2, Qt::AlignBottom);
    gpuTempChart->addAxis(axisY2, Qt::AlignLeft);
    gpuTempSeries->attachAxis(axisX2);
    gpuTempSeries->attachAxis(axisY2);

    gpuTempChartView = new QChartView(gpuTempChart);
    gpuTempChartView->setRenderHint(QPainter::Antialiasing);

    // Create horizontal splitter
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(cpuTempChartView);
    splitter->addWidget(gpuTempChartView);

    layout->addWidget(splitter);
}

void QtWidgetsTCmonitor::createDiskSection()
{
    diskGroupBox = new QGroupBox(tr("������Ϣ"), this);
    QVBoxLayout* layout = new QVBoxLayout(diskGroupBox);

    // Disk info container
    QWidget* diskContainer = new QWidget();
    layout->addWidget(diskContainer);
}

void QtWidgetsTCmonitor::updateTemperatureData(const std::vector<std::pair<std::string, float>>& temperatures)
{
    float cpuTemp = 0;
    float gpuTemp = 0;
    bool cpuFound = false;
    bool gpuFound = false;

    // Iterate through temperature data
    for (const auto& temp : temperatures) {
        if (temp.first == "CPU Package" || temp.first == "CPU Temperature" || temp.first == "CPU Average Core") {
            cpuTemp = temp.second;
            cpuFound = true;
            infoLabels["cpuTemp"]->setText(formatTemperature(cpuTemp));
        }
        else if (temp.first.find("GPU Core") != std::string::npos) {
            gpuTemp = temp.second;
            gpuFound = true;
            infoLabels["gpuTemp"]->setText(formatTemperature(gpuTemp));
        }
    }

    // If no data found, update to no data state
    if (!cpuFound) {
        infoLabels["cpuTemp"]->setText(tr("������"));
    }
    if (!gpuFound) {
        infoLabels["gpuTemp"]->setText(tr("������"));
    }

    // Update temperature history
    if (cpuFound) {
        cpuTempHistory.push(cpuTemp);
        if (cpuTempHistory.size() > MAX_DATA_POINTS) {
            cpuTempHistory.pop();
        }
    }

    if (gpuFound) {
        gpuTempHistory.push(gpuTemp);
        if (gpuTempHistory.size() > MAX_DATA_POINTS) {
            gpuTempHistory.pop();
        }
    }

    // Save current system info temperature data
    std::vector<std::pair<std::string, double>> tempDoubles;
    for (const auto& temp : temperatures) {
        tempDoubles.push_back({temp.first, static_cast<double>(temp.second)});
    }
    currentSysInfo.temperatures = tempDoubles;
}

void QtWidgetsTCmonitor::updateSystemInfo(const SystemInfo& sysInfo)
{
    currentSysInfo = sysInfo;

    // Update CPU info
    infoLabels["cpuName"]->setText(QString::fromStdString(sysInfo.cpuName));
    infoLabels["physicalCores"]->setText(QString::number(sysInfo.physicalCores));
    infoLabels["logicalCores"]->setText(QString::number(sysInfo.logicalCores));
    infoLabels["performanceCores"]->setText(QString::number(sysInfo.performanceCores));
    infoLabels["efficiencyCores"]->setText(QString::number(sysInfo.efficiencyCores));
    infoLabels["cpuUsage"]->setText(formatPercentage(sysInfo.cpuUsage));
    infoLabels["hyperThreading"]->setText(sysInfo.hyperThreading ? tr("������") : tr("δ����"));
    infoLabels["virtualization"]->setText(sysInfo.virtualization ? tr("������") : tr("δ����"));

    // Update memory info
    infoLabels["totalMemory"]->setText(formatSize(sysInfo.totalMemory));
    infoLabels["usedMemory"]->setText(formatSize(sysInfo.usedMemory));
    infoLabels["availableMemory"]->setText(formatSize(sysInfo.availableMemory));

    double memoryUsagePercent = static_cast<double>(sysInfo.usedMemory) / sysInfo.totalMemory * 100.0;
    infoLabels["memoryUsage"]->setText(formatPercentage(memoryUsagePercent));

    // Update GPU info
    infoLabels["gpuName"]->setText(QString::fromStdString(sysInfo.gpuName));
    infoLabels["gpuBrand"]->setText(QString::fromStdString(sysInfo.gpuBrand));
    infoLabels["gpuMemory"]->setText(formatSize(sysInfo.gpuMemory));
    infoLabels["gpuCoreFreq"]->setText(formatFrequency(sysInfo.gpuCoreFreq));

    // Convert temperature data to float for updateTemperatureData
    std::vector<std::pair<std::string, float>> tempFloats;
    for (const auto& temp : sysInfo.temperatures) {
        tempFloats.push_back({temp.first, static_cast<float>(temp.second)});
    }
    
    // Update temperature data
    updateTemperatureData(tempFloats);

    // Update disk info
    // Need to clear existing disk info layout first
    QLayout* currentLayout = diskGroupBox->layout();
    QWidget* diskContainer = nullptr;

    if (currentLayout) {
        // Get disk container from current layout
        for (int i = 0; i < currentLayout->count(); ++i) {
            QWidget* widget = currentLayout->itemAt(i)->widget();
            if (widget) {
                diskContainer = widget;
                break;
            }
        }
    }

    // If container not found, create a new one
    if (!diskContainer) {
        diskContainer = new QWidget(diskGroupBox);
        static_cast<QVBoxLayout*>(currentLayout)->addWidget(diskContainer);
    }

    // Delete existing layout
    if (diskContainer->layout()) {
        QLayoutItem* child;
        while ((child = diskContainer->layout()->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->deleteLater();
            }
            delete child;
        }
        delete diskContainer->layout();
    }

    // Create new layout
    QVBoxLayout* diskLayout = new QVBoxLayout(diskContainer);

    // Add disk info
    for (const auto& disk : sysInfo.disks) {
        QString diskLabel = QString("%1: %2").arg(disk.letter).arg(tr("������"));
        if (!disk.label.empty()) {
            diskLabel += QString(" (%1)").arg(QString::fromStdString(disk.label));
        }

        QGroupBox* diskBox = new QGroupBox(diskLabel);
        QGridLayout* diskInfoLayout = new QGridLayout(diskBox);

        int row = 0;
        if (!disk.fileSystem.empty()) {
            diskInfoLayout->addWidget(new QLabel(tr("�ļ�ϵͳ:")), row, 0);
            diskInfoLayout->addWidget(new QLabel(QString::fromStdString(disk.fileSystem)), row++, 1);
        }

        diskInfoLayout->addWidget(new QLabel(tr("������:")), row, 0);
        diskInfoLayout->addWidget(new QLabel(formatSize(disk.totalSize)), row++, 1);

        diskInfoLayout->addWidget(new QLabel(tr("���ÿռ�:")), row, 0);
        diskInfoLayout->addWidget(new QLabel(formatSize(disk.usedSpace)), row++, 1);

        diskInfoLayout->addWidget(new QLabel(tr("���ÿռ�:")), row, 0);
        diskInfoLayout->addWidget(new QLabel(formatSize(disk.freeSpace)), row++, 1);

        double usagePercent = static_cast<double>(disk.usedSpace) / disk.totalSize * 100.0;
        diskInfoLayout->addWidget(new QLabel(tr("ʹ����:")), row, 0);
        diskInfoLayout->addWidget(new QLabel(formatPercentage(usagePercent)), row++, 1);

        diskLayout->addWidget(diskBox);
    }

    diskLayout->addStretch();
}

void QtWidgetsTCmonitor::updateCharts()
{
    // Update CPU temperature chart
    cpuTempSeries->clear();
    int pointIndex = 0;
    std::queue<float> cpuTempCopy = cpuTempHistory;
    while (!cpuTempCopy.empty()) {
        cpuTempSeries->append(pointIndex, cpuTempCopy.front());
        cpuTempCopy.pop();
        pointIndex++;
    }

    // Update GPU temperature chart
    gpuTempSeries->clear();
    pointIndex = 0;
    std::queue<float> gpuTempCopy = gpuTempHistory;
    while (!gpuTempCopy.empty()) {
        gpuTempSeries->append(pointIndex, gpuTempCopy.front());
        gpuTempCopy.pop();
        pointIndex++;
    }
}

void QtWidgetsTCmonitor::on_pushButton_clicked()
{
    QMessageBox::information(this, tr("ϵͳ���"), tr("���ڼ��ϵͳӲ����Ϣ"));
}

QString QtWidgetsTCmonitor::formatSize(uint64_t bytes)
{
    constexpr double KB = 1024.0;
    constexpr double MB = KB * KB;
    constexpr double GB = MB * KB;
    constexpr double TB = GB * KB;

    QString result;
    if (bytes >= TB) {
        result = QString("%1 TB").arg(bytes / TB, 0, 'f', 2);
    }
    else if (bytes >= GB) {
        result = QString("%1 GB").arg(bytes / GB, 0, 'f', 2);
    }
    else if (bytes >= MB) {
        result = QString("%1 MB").arg(bytes / MB, 0, 'f', 2);
    }
    else if (bytes >= KB) {
        result = QString("%1 KB").arg(bytes / KB, 0, 'f', 2);
    }
    else {
        result = QString("%1 B").arg(bytes);
    }

    return result;
}

QString QtWidgetsTCmonitor::formatPercentage(double value)
{
    return QString("%1%").arg(value, 0, 'f', 1);
}

QString QtWidgetsTCmonitor::formatTemperature(double value)
{
    return QString(u"%1��C").arg(static_cast<int>(value));
}

QString QtWidgetsTCmonitor::formatFrequency(double value)
{
    if (value >= 1000) {
        return QString("%1 GHz").arg(value / 1000.0, 0, 'f', 2);
    }
    else {
        return QString("%1 MHz").arg(value, 0, 'f', 2);
    }
}
