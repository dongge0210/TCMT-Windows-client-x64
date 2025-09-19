using System.Collections.ObjectModel;
using System.ComponentModel; // 注释
using System.Runtime.CompilerServices; // 注释

namespace WPF_UI1.Models
{
    // 对应C++的SystemInfo结构
    public class SystemInfo
    {
        public string CpuName { get; set; } = string.Empty;
        public int PhysicalCores { get; set; }
        public int LogicalCores { get; set; }
        public double CpuUsage { get; set; }
        public int PerformanceCores { get; set; }
        public int EfficiencyCores { get; set; }
        public double PerformanceCoreFreq { get; set; }
        public double EfficiencyCoreFreq { get; set; }
        public bool HyperThreading { get; set; }
        public bool Virtualization { get; set; }
        // 内存信息
        public ulong TotalMemory { get; set; }
        public ulong UsedMemory { get; set; }
        public ulong AvailableMemory { get; set; }
        // GPU信息
        public List<GpuData> Gpus { get; set; } = new();
        public string GpuName { get; set; } = string.Empty;
        public string GpuBrand { get; set; } = string.Empty;
        public ulong GpuMemory { get; set; }
        public double GpuCoreFreq { get; set; }
        public bool GpuIsVirtual { get; set; }
        // 网卡信息
        public List<NetworkAdapterData> Adapters { get; set; } = new();
        public string NetworkAdapterName { get; set; } = string.Empty;
        public string NetworkAdapterMac { get; set; } = string.Empty;
        public string NetworkAdapterIp { get; set; } = string.Empty;
        public string NetworkAdapterType { get; set; } = string.Empty;
        public ulong NetworkAdapterSpeed { get; set; }
        // 逻辑磁盘信息（含分区）
        public List<DiskData> Disks { get; set; } = new();
        // 物理磁盘信息（SMART能力）
        public List<PhysicalDiskSmartData> PhysicalDisks { get; set; } = new();
        // 温度信息
        public List<TemperatureData> Temperatures { get; set; } = new();
        public double CpuTemperature { get; set; }
        public double GpuTemperature { get; set; }
        public double CpuUsageSampleIntervalMs { get; set; } // 最近一次CPU使用率采样间隔
        public DateTime LastUpdate { get; set; }
        // TPM 信息（显示用）
        public bool HasTpm { get; set; }
        public string TpmManufacturer { get; set; } = string.Empty;
        public string TpmManufacturerId { get; set; } = string.Empty;
        public string TpmVersion { get; set; } = string.Empty;
        public string TpmFirmwareVersion { get; set; } = string.Empty;
        public string TpmStatus { get; set; } = string.Empty;
        public bool TpmEnabled { get; set; }
        public bool TpmIsActivated { get; set; }
        public bool TpmIsOwned { get; set; }
        public bool TpmReady { get; set; }
        public bool TpmTbsAvailable { get; set; }
        public bool TpmPhysicalPresenceRequired { get; set; }
        public uint TpmSpecVersion { get; set; }
        public uint TpmTbsVersion { get; set; }
        public string TpmErrorMessage { get; set; } = string.Empty;
        public string TpmDetectionMethod { get; set; } = string.Empty; // 检测方法
        public bool TpmWmiDetectionWorked { get; set; } // WMI检测是否成功
        public bool TpmTbsDetectionWorked { get; set; } // TBS检测是否成功
    }

    public abstract class NotifyBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler? PropertyChanged;
        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string? name = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
            return true;
        }
    }

    public class GpuData : NotifyBase
    {
        private string _name = string.Empty;
        private string _brand = string.Empty;
        private ulong _memory;
        private double _coreClock;
        private bool _isVirtual;
        public string Name { get => _name; set => SetProperty(ref _name, value); }
        public string Brand { get => _brand; set => SetProperty(ref _brand, value); }
        public ulong Memory { get => _memory; set => SetProperty(ref _memory, value); }
        public double CoreClock { get => _coreClock; set => SetProperty(ref _coreClock, value); }
        public bool IsVirtual { get => _isVirtual; set => SetProperty(ref _isVirtual, value); }
    }

    public class NetworkAdapterData : NotifyBase
    {
        private string _name = string.Empty;
        private string _mac = string.Empty;
        private string _ipAddress = string.Empty;
        private string _adapterType = string.Empty;
        private ulong _speed;
        public string Name { get => _name; set => SetProperty(ref _name, value); }
        public string Mac { get => _mac; set => SetProperty(ref _mac, value); }
        public string IpAddress { get => _ipAddress; set => SetProperty(ref _ipAddress, value); }
        public string AdapterType { get => _adapterType; set => SetProperty(ref _adapterType, value); }
        public ulong Speed { get => _speed; set => SetProperty(ref _speed, value); }
    }

    // 逻辑磁盘展示结构
    public class DiskData : NotifyBase
    {
        private char _letter;
        private string _label = string.Empty;
        private string _fileSystem = string.Empty;
        private ulong _totalSize;
        private ulong _usedSpace;
        private ulong _freeSpace;
        private int _physicalDiskIndex = -1;
        public char Letter { get => _letter; set => SetProperty(ref _letter, value); }
        public string Label { get => _label; set => SetProperty(ref _label, value); }
        public string FileSystem { get => _fileSystem; set => SetProperty(ref _fileSystem, value); }
        public ulong TotalSize { get => _totalSize; set => SetProperty(ref _totalSize, value); }
        public ulong UsedSpace { get => _usedSpace; set => SetProperty(ref _usedSpace, value); }
        public ulong FreeSpace { get => _freeSpace; set => SetProperty(ref _freeSpace, value); }
        public int PhysicalDiskIndex { get => _physicalDiskIndex; set => SetProperty(ref _physicalDiskIndex, value); }
    }

    // SMART 属性
    public class SmartAttributeData
    {
        public byte Id { get; set; }
        public byte Current { get; set; }
        public byte Worst { get; set; }
        public byte Threshold { get; set; }
        public ulong RawValue { get; set; }
        public string Name { get; set; } = string.Empty;
        public string Description { get; set; } = string.Empty;
        public bool IsCritical { get; set; }
        public double PhysicalValue { get; set; }
        public string Units { get; set; } = string.Empty;
    }

    // 物理磁盘（聚合对象+分区列表）
    public class PhysicalDiskSmartData
    {
        public string Model { get; set; } = string.Empty;
        public string SerialNumber { get; set; } = string.Empty;
        public string FirmwareVersion { get; set; } = string.Empty;
        public string InterfaceType { get; set; } = string.Empty;
        public string DiskType { get; set; } = string.Empty; // SSD/HDD
        public ulong Capacity { get; set; }
        public double Temperature { get; set; }
        public byte HealthPercentage { get; set; }
        public bool IsSystemDisk { get; set; }
        public bool SmartEnabled { get; set; }
        public bool SmartSupported { get; set; }
        public ulong PowerOnHours { get; set; }
        public ulong PowerCycleCount { get; set; }
        public ulong ReallocatedSectorCount { get; set; }
        public ulong CurrentPendingSector { get; set; }
        public ulong UncorrectableErrors { get; set; }
        public double WearLeveling { get; set; }
        public ulong TotalBytesWritten { get; set; }
        public ulong TotalBytesRead { get; set; }
        public List<char> LogicalDriveLetters { get; set; } = new();
        public List<SmartAttributeData> Attributes { get; set; } = new();
    }

    public class TemperatureData
    {
        public string SensorName { get; set; } = string.Empty;
        public double Temperature { get; set; }
    }

    // WPF 物理磁盘视图模型 + 展示字段
    public class PhysicalDiskView : NotifyBase
    {
        private PhysicalDiskSmartData _disk;
        public PhysicalDiskSmartData Disk { get => _disk; set => SetProperty(ref _disk, value); }
        public ObservableCollection<DiskData> Partitions { get; } = new();
        public string LettersDisplay => Partitions.Count == 0 ? "无分区" : string.Join(", ", Partitions.Select(p => p.Letter + ":"));
        public string DisplayName => Disk == null ? "未知磁盘" : $"{Disk.Model} ({LettersDisplay})";
    }
}