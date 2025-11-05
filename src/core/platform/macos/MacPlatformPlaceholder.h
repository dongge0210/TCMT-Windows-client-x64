#pragma once
#include "../../common/BaseInfo.h"
#include <memory>
#include <string>

#ifdef PLATFORM_MACOS

// 前向声明
class MacGpuInfo;

class MacNetworkAdapter : public INetworkAdapter {
public:
    MacNetworkAdapter(const std::string& name) : m_name(name) {}
    virtual ~MacNetworkAdapter() = default;
    
    // 基本接口实现（占位符）
    virtual std::string GetName() const override { return m_name; }
    virtual std::string GetDescription() const override { return "macOS Network Adapter"; }
    virtual bool IsConnected() const override { return true; }
    virtual double GetUploadSpeed() const override { return 0.0; }
    virtual double GetDownloadSpeed() const override { return 0.0; }
    virtual uint64_t GetTotalBytesSent() const override { return 0; }
    virtual uint64_t GetTotalBytesReceived() const override { return 0; }
    virtual std::string GetMacAddress() const override { return "00:00:00:00:00:00"; }
    virtual std::string GetIpAddress() const override { return "192.168.1.100"; }
    virtual bool Initialize() override { return true; }
    virtual void Cleanup() override {}
    virtual bool Update() override { return true; }

private:
    std::string m_name;
};

class MacDiskInfo : public IDiskInfo {
public:
    MacDiskInfo(const std::string& name) : m_name(name) {}
    virtual ~MacDiskInfo() = default;
    
    // 基本接口实现（占位符）
    virtual std::string GetName() const override { return m_name; }
    virtual std::string GetModel() const override { return "macOS Disk"; }
    virtual uint64_t GetTotalSize() const override { return 1000000000000ULL; } // 1TB
    virtual uint64_t GetUsedSpace() const override { return 500000000000ULL; }  // 500GB
    virtual uint64_t GetFreeSpace() const override { return 500000000000ULL; }  // 500GB
    virtual double GetUsagePercentage() const override { return 50.0; }
    virtual double GetReadSpeed() const override { return 0.0; }
    virtual double GetWriteSpeed() const override { return 0.0; }
    virtual bool IsHealthy() const override { return true; }
    virtual double GetTemperature() const override { return 0.0; }
    virtual bool Initialize() override { return true; }
    virtual void Cleanup() override {}
    virtual bool Update() override { return true; }

private:
    std::string m_name;
};

class MacTemperatureWrapper : public ITemperatureMonitor {
public:
    MacTemperatureWrapper() = default;
    virtual ~MacTemperatureWrapper() = default;
    
    // 基本接口实现（占位符）
    virtual double GetCpuTemperature() const override { return 45.0; }
    virtual double GetGpuTemperature() const override { return 40.0; }
    virtual std::vector<std::pair<std::string, double>> GetAllTemperatures() const override { 
        return {{"CPU", 45.0}, {"GPU", 40.0}}; 
    }
    virtual bool Initialize() override { return true; }
    virtual void Cleanup() override {}
    virtual bool Update() override { return true; }
};

class MacTpmInfo : public ITpmInfo {
public:
    MacTpmInfo() = default;
    virtual ~MacTpmInfo() = default;
    
    // 基本接口实现（占位符）
    virtual bool IsPresent() const override { return false; } // macOS通常不使用TPM
    virtual std::string GetVersion() const override { return ""; }
    virtual std::string GetManufacturer() const override { return ""; }
    virtual bool IsEnabled() const override { return false; }
    virtual bool IsActivated() const override { return false; }
    virtual bool Initialize() override { return true; }
    virtual void Cleanup() override {}
    virtual bool Update() override { return true; }
};

class MacUsbInfo : public IUsbMonitor {
public:
    MacUsbInfo() = default;
    virtual ~MacUsbInfo() = default;
    
    // 基本接口实现（占位符）
    virtual std::vector<UsbDevice> GetConnectedDevices() const override { 
        return std::vector<UsbDevice>(); 
    }
    virtual bool IsDeviceConnected(const std::string& deviceId) const override { return false; }
    virtual bool Initialize() override { return true; }
    virtual void Cleanup() override {}
    virtual bool Update() override { return true; }
};

class MacOSInfo : public IOSInfo {
public:
    MacOSInfo() = default;
    virtual ~MacOSInfo() = default;
    
    // 基本接口实现（占位符）
    virtual std::string GetName() const override { return "macOS"; }
    virtual std::string GetVersion() const override { return "14.0"; }
    virtual std::string GetBuildNumber() const override { return "23A344"; }
    virtual std::string GetArchitecture() const override { return "arm64"; }
    virtual std::string GetComputerName() const override { return "MacBook"; }
    virtual std::string GetUserName() const override { return "user"; }
    virtual time_t GetBootTime() const override { return 0; }
    virtual time_t GetLastSystemUpdateTime() const override { return 0; }
    virtual bool Initialize() override { return true; }
    virtual void Cleanup() override {}
    virtual bool Update() override { return true; }
};

class MacDataCollector : public IDataCollector {
public:
    MacDataCollector() = default;
    virtual ~MacDataCollector() = default;
    
    // 基本接口实现（占位符）
    virtual bool StartCollection() override { return true; }
    virtual bool StopCollection() override { return true; }
    virtual bool IsCollecting() const override { return false; }
    virtual bool UpdateAll() override { return true; }
    virtual bool Initialize() override { return true; }
    virtual void Cleanup() override {}
    virtual bool Update() override { return true; }
};

#endif // PLATFORM_MACOS