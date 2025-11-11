#include "CLISystemInfo.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>

bool SharedMemoryDataParser::ConvertToCLISystemInfo(const SystemInfo& source, CLISystemInfo& target) {
    try {
        
        target = CLISystemInfo();
        
        
        target.connected = true;
        target.lastUpdateTime = GetCurrentTimestamp();
        target.connectionStatus = "Connected";
        
        
        target.cpuName = source.cpuName;
        target.physicalCores = source.physicalCores;
        target.logicalCores = source.logicalCores;
        target.performanceCores = source.performanceCores;
        target.efficiencyCores = source.efficiencyCores;
        target.cpuUsage = source.cpuUsage;
        target.cpuTemperature = source.cpuTemperature;
        target.cpuBaseFrequencyMHz = source.cpuBaseFrequencyMHz;
        target.cpuCurrentFrequencyMHz = source.cpuCurrentFrequencyMHz;
        target.cpuUsageSampleIntervalMs = source.cpuUsageSampleIntervalMs;
        target.hyperThreading = source.hyperThreading;
        target.virtualization = source.virtualization;
        
        
        target.totalMemory = source.totalMemory;
        target.usedMemory = source.usedMemory;
        target.availableMemory = source.availableMemory;
        target.memoryUsagePercent = (source.totalMemory > 0) ? 
            (static_cast<double>(source.usedMemory) / source.totalMemory * 100.0) : 0.0;
        
        
        target.gpus = ParseGpuData(source.gpus);
        target.gpuTemperature = source.gpuTemperature;
        
        
        target.temperatures = ParseTemperatureData(source.temperatures);
        
        
        target.disks = ParseDiskData(source.disks);
        target.physicalDisks = ParsePhysicalDiskSmartData(source.physicalDisks);
        
        
        target.adapters = ParseNetworkAdapterData(source.adapters);
        
        
        target.usbDevices = ParseUSBDeviceData(source.usbDevices);
        
        
        target.tpm = ParseTpmData(source);
        
        
        target.osVersion = source.osVersion;
        
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::vector<CLIGpuData> SharedMemoryDataParser::ParseGpuData(const std::vector<GPUData>& source) {
    std::vector<CLIGpuData> result;
    
    for (const auto& gpu : source) {
        CLIGpuData cliGpu;
        cliGpu.name = ParseWideString(gpu.name, 128);
        cliGpu.brand = ParseWideString(gpu.brand, 64);
        cliGpu.memory = gpu.memory;
        cliGpu.coreClock = gpu.coreClock;
        cliGpu.isVirtual = gpu.isVirtual;
        
        result.push_back(cliGpu);
    }
    
    return result;
}

std::vector<CLINetworkAdapterData> SharedMemoryDataParser::ParseNetworkAdapterData(const std::vector<NetworkAdapterData>& source) {
    std::vector<CLINetworkAdapterData> result;
    
    for (const auto& adapter : source) {
        CLINetworkAdapterData cliAdapter;
        cliAdapter.name = ParseWideString(adapter.name, 128);
        cliAdapter.mac = ParseWideString(adapter.mac, 32);
        cliAdapter.ipAddress = ParseWideString(adapter.ipAddress, 64);
        cliAdapter.adapterType = ParseWideString(adapter.adapterType, 32);
        cliAdapter.speed = adapter.speed;
        
        result.push_back(cliAdapter);
    }
    
    return result;
}

std::vector<CLIDiskData> SharedMemoryDataParser::ParseDiskData(const std::vector<DiskData>& source) {
    std::vector<CLIDiskData> result;
    
    for (const auto& disk : source) {
        CLIDiskData cliDisk;
        cliDisk.letter = disk.letter;
        cliDisk.label = disk.label;
        cliDisk.fileSystem = disk.fileSystem;
        cliDisk.totalSize = disk.totalSize;
        cliDisk.usedSpace = disk.usedSpace;
        cliDisk.freeSpace = disk.freeSpace;
        cliDisk.physicalDiskIndex = -1;
        
        result.push_back(cliDisk);
    }
    
    return result;
}

std::vector<CLITemperatureData> SharedMemoryDataParser::ParseTemperatureData(const std::vector<std::pair<std::string, double>>& source) {
    std::vector<CLITemperatureData> result;
    
    for (const auto& temp : source) {
        result.emplace_back(temp.first, temp.second);
    }
    
    return result;
}

std::vector<CLIUSBDeviceData> SharedMemoryDataParser::ParseUSBDeviceData(const std::vector<USBDeviceInfo>& source) {
    std::vector<CLIUSBDeviceData> result;
    
    for (const auto& usb : source) {
        CLIUSBDeviceData cliUsb;
        cliUsb.drivePath = usb.drivePath;
        cliUsb.volumeLabel = usb.volumeLabel;
        cliUsb.totalSize = usb.totalSize;
        cliUsb.freeSpace = usb.freeSpace;
        cliUsb.isUpdateReady = usb.isUpdateReady;
        cliUsb.state = static_cast<int>(usb.state);
        cliUsb.lastUpdate = usb.lastUpdate;
        
        result.push_back(cliUsb);
    }
    
    return result;
}

std::vector<CLIPhysicalDiskSmartData> SharedMemoryDataParser::ParsePhysicalDiskSmartData(const std::vector<PhysicalDiskSmartData>& source) {
    std::vector<CLIPhysicalDiskSmartData> result;
    
    for (const auto& disk : source) {
        CLIPhysicalDiskSmartData cliDisk;
        cliDisk.model = ParseWideString(disk.model, 128);
        cliDisk.serialNumber = ParseWideString(disk.serialNumber, 64);
        cliDisk.firmwareVersion = ParseWideString(disk.firmwareVersion, 32);
        cliDisk.interfaceType = ParseWideString(disk.interfaceType, 32);
        cliDisk.diskType = ParseWideString(disk.diskType, 16);
        cliDisk.capacity = disk.capacity;
        cliDisk.temperature = disk.temperature;
        cliDisk.healthPercentage = disk.healthPercentage;
        cliDisk.isSystemDisk = disk.isSystemDisk;
        cliDisk.smartEnabled = disk.smartEnabled;
        cliDisk.smartSupported = disk.smartSupported;
        
        result.push_back(cliDisk);
    }
    
    return result;
}

CLITpmData SharedMemoryDataParser::ParseTpmData(const SystemInfo& source) {
    CLITpmData cliTpm;
    
    cliTpm.hasTpm = source.hasTpm;
    cliTpm.manufacturer = source.tpmManufacturer;
    cliTpm.manufacturerId = source.tpmManufacturerId;
    cliTpm.version = source.tpmVersion;
    cliTpm.firmwareVersion = source.tpmFirmwareVersion;
    cliTpm.status = source.tpmStatus;
    cliTpm.isEnabled = source.tpmEnabled;
    cliTpm.isActivated = source.tpmIsActivated;
    cliTpm.isOwned = source.tpmIsOwned;
    cliTpm.isReady = source.tpmReady;
    cliTpm.tbsAvailable = source.tpmTbsAvailable;
    cliTpm.physicalPresenceRequired = source.tpmPhysicalPresenceRequired;
    cliTpm.specVersion = source.tpmSpecVersion;
    cliTpm.tbsVersion = source.tpmTbsVersion;
    cliTpm.errorMessage = source.tpmErrorMessage;
    cliTpm.detectionMethod = source.tmpDetectionMethod;
    
    return cliTpm;
}

std::string SharedMemoryDataParser::FormatBytes(uint64_t bytes) {
    if (bytes == 0) {
        return "0 B";
    }
    
    const uint64_t KB = 1024ULL;
    const uint64_t MB = KB * KB;
    const uint64_t GB = MB * KB;
    const uint64_t TB = GB * KB;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    
    if (bytes >= TB) {
        oss << (static_cast<double>(bytes) / TB) << " TB";
    } else if (bytes >= GB) {
        oss << (static_cast<double>(bytes) / GB) << " GB";
    } else if (bytes >= MB) {
        oss << (static_cast<double>(bytes) / MB) << " MB";
    } else if (bytes >= KB) {
        oss << (static_cast<double>(bytes) / KB) << " KB";
    } else {
        oss << bytes << " B";
    }
    
    return oss.str();
}

std::string SharedMemoryDataParser::FormatFrequency(double mhz) {
    if (mhz <= 0) {
        return "Unknown";
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    
    if (mhz >= 1000) {
        oss << (mhz / 1000.0) << " GHz";
    } else {
        oss << mhz << " MHz";
    }
    
    return oss.str();
}

std::string SharedMemoryDataParser::FormatPercentage(double value) {
    if (value < 0 || std::isnan(value)) {
        return "Unknown";
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value << "%";
    return oss.str();
}

std::string SharedMemoryDataParser::FormatTemperature(double temperature) {
    if (temperature < -100 || std::isnan(temperature)) {
        return "Unknown";
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << temperature << "°C";
    return oss.str();
}

std::string SharedMemoryDataParser::FormatNetworkSpeed(uint64_t bps) {
    if (bps == 0) {
        return "Disconnected";
    }
    
    const uint64_t K = 1000ULL;
    const uint64_t M = K * K;
    const uint64_t G = M * K;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    
    if (bps >= G) {
        oss << (static_cast<double>(bps) / G) << " Gbps";
    } else if (bps >= M) {
        oss << (static_cast<double>(bps) / M) << " Mbps";
    } else if (bps >= K) {
        oss << (static_cast<double>(bps) / K) << " Kbps";
    } else {
        oss << bps << " bps";
    }
    
    return oss.str();
}

std::string SharedMemoryDataParser::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    oss << std::put_time(&timeinfo, "%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string SharedMemoryDataParser::WideStringToUTF8(const std::wstring& wideStr) {
    if (wideStr.empty()) {
        return "";
    }
    
    std::string utf8Str;
    utf8Str.reserve(wideStr.length());
    
    for (wchar_t wc : wideStr) {
        if (wc < 0x80) {
            utf8Str.push_back(static_cast<char>(wc));
        } else if (wc < 0x800) {
            utf8Str.push_back(static_cast<char>(0xC0 | (wc >> 6)));
            utf8Str.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
        } else {
            utf8Str.push_back(static_cast<char>(0xE0 | (wc >> 12)));
            utf8Str.push_back(static_cast<char>(0x80 | ((wc >> 6) & 0x3F)));
            utf8Str.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
        }
    }
    
    return utf8Str;
}

std::string SharedMemoryDataParser::ParseWideString(const wchar_t* data, int maxLength) {
    if (data == nullptr || maxLength <= 0) {
        return "";
    }
    
    
    int length = 0;
    while (length < maxLength && data[length] != L'\0') {
        ++length;
    }
    
    if (length == 0) {
        return "";
    }
    
    try {
        std::wstring wideStr(data, length);
        return WideStringToUTF8(wideStr);
    }
    catch (const std::exception&) {
        return "";
    }
}