using System.IO;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;
using System.Text;
using WPF_UI1.Models;
using Serilog;

namespace WPF_UI1.Services
{
    public class SharedMemoryService : IDisposable
    {
        private MemoryMappedFile? _mmf;
        private MemoryMappedViewAccessor? _accessor;
        private readonly object _lock = new();
        private bool _disposed = false;

        private const string SHARED_MEMORY_NAME = "SystemMonitorSharedMemory";
        private const string GLOBAL_SHARED_MEMORY_NAME = "Global\\SystemMonitorSharedMemory";
        private const string LOCAL_SHARED_MEMORY_NAME = "Local\\SystemMonitorSharedMemory";
        private const int SHARED_MEMORY_SIZE = 65536; // 64KB

        public bool IsInitialized { get; private set; }
        public string LastError { get; private set; } = string.Empty;

        // C++���ݽṹ���� - ��C++����ȫƥ��
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SharedMemoryBlock
        {
            // CPU��Ϣ - ʹ��wchar_t����
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public ushort[] cpuName; // wchar_t ��Windows����16λ
            
            public int physicalCores;
            public int logicalCores;
            public double cpuUsage;
            public int performanceCores;
            public int efficiencyCores;
            public double pCoreFreq;
            public double eCoreFreq;
            public bool hyperThreading;
            public bool virtualization;
            
            // �ڴ���Ϣ
            public ulong totalMemory;
            public ulong usedMemory;
            public ulong availableMemory;
            
            // �¶���Ϣ
            public double cpuTemperature;
            public double gpuTemperature;
            
            // GPU��Ϣ��֧�����2��GPU��
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
            public GPUDataStruct[] gpus;
            
            // ������������֧�����4����������
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public NetworkAdapterStruct[] adapters;
            
            // �߼�������Ϣ��֧�����8�����̣�
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public SharedDiskDataStruct[] disks;
            
            // �������SMART��Ϣ��֧�����8��������̣�
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public PhysicalDiskSmartDataStruct[] physicalDisks;
            
            // �¶����ݣ�֧��10����������
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
            public TemperatureDataStruct[] temperatures;
            
            // ������
            public int adapterCount;
            public int tempCount;
            public int gpuCount;
            public int diskCount;
            public int physicalDiskCount;
            
            // ������ʱ��
            public SYSTEMTIME lastUpdate;
            
            // CRITICAL_SECTION (��C#�к���)
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 24)]
            public byte[] lockData;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct GPUDataStruct
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public ushort[] name;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
            public ushort[] brand;
            public ulong memory;
            public double coreClock;
            public bool isVirtual;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct NetworkAdapterStruct
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public ushort[] name;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public ushort[] mac;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
            public ushort[] ipAddress;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public ushort[] adapterType;
            public ulong speed;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SharedDiskDataStruct
        {
            public byte letter; // char ��8λ
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public ushort[] label;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public ushort[] fileSystem;
            public ulong totalSize;
            public ulong usedSpace;
            public ulong freeSpace;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TemperatureDataStruct
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
            public ushort[] sensorName;
            public double temperature;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct PhysicalDiskSmartDataStruct
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public ushort[] model;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
            public ushort[] serialNumber;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public ushort[] firmwareVersion;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public ushort[] interfaceType;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
            public ushort[] diskType;
            public ulong capacity;
            public double temperature;
            public byte healthPercentage;
            public bool isSystemDisk;
            public bool smartEnabled;
            public bool smartSupported;
            // ... ����SMART����ʡ���Ա��ּ��
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct SYSTEMTIME
        {
            public ushort wYear;
            public ushort wMonth;
            public ushort wDayOfWeek;
            public ushort wDay;
            public ushort wHour;
            public ushort wMinute;
            public ushort wSecond;
            public ushort wMilliseconds;
        }

        public bool Initialize()
        {
            lock (_lock)
            {
                if (IsInitialized)
                    return true;

                try
                {
                    // ���Զ���������ʽ�򿪹����ڴ�
                    string[] names = { GLOBAL_SHARED_MEMORY_NAME, LOCAL_SHARED_MEMORY_NAME, SHARED_MEMORY_NAME };
                    
                    foreach (string name in names)
                    {
                        try
                        {
                            Log.Debug($"���Դ򿪹����ڴ�: {name}");
                            
                            // ���Դ����еĹ����ڴ�
                            _mmf = MemoryMappedFile.OpenExisting(name, MemoryMappedFileRights.Read);
                            _accessor = _mmf.CreateViewAccessor(0, SHARED_MEMORY_SIZE, MemoryMappedFileAccess.Read);
                            
                            IsInitialized = true;
                            Log.Information($"? �ɹ����ӵ������ڴ�: {name}");
                            
                            return true;
                        }
                        catch (FileNotFoundException)
                        {
                            Log.Debug($"? �����ڴ治����: {name}");
                            continue;
                        }
                        catch (Exception ex)
                        {
                            Log.Warning($"?? �򿪹����ڴ�ʧ�� {name}: {ex.Message}");
                            continue;
                        }
                    }

                    LastError = "�޷��ҵ������ڴ棬��ȷ��C++��������������";
                    Log.Error($"?? {LastError}");
                    return false;
                }
                catch (Exception ex)
                {
                    LastError = $"��ʼ�������ڴ�ʱ��������: {ex.Message}";
                    Log.Error(ex, LastError);
                    return false;
                }
            }
        }

        public SystemInfo? ReadSystemInfo()
        {
            lock (_lock)
            {
                if (!IsInitialized || _accessor == null)
                {
                    if (!Initialize())
                        return null;
                }

                try
                {
                    // ��ȡ�����Ĺ����ڴ�����
                    return ReadCompleteSystemInfo();
                }
                catch (Exception ex)
                {
                    LastError = $"��ȡ�����ڴ�����ʱ��������: {ex.Message}";
                    Log.Error(ex, LastError);
                    
                    // �������³�ʼ��
                    Dispose();
                    IsInitialized = false;
                    
                    return null;
                }
            }
        }

        private SystemInfo ReadCompleteSystemInfo()
        {
            if (_accessor == null)
                throw new InvalidOperationException("�����ڴ������δ��ʼ��");

            try
            {
                // ����ṹ���С����ȡ����
                int structSize = Marshal.SizeOf<SharedMemoryBlock>();
                var buffer = new byte[structSize];
                
                // ��ȡ�ֽ�����
                int bytesToRead = Math.Min(structSize, (int)_accessor.Capacity);
                _accessor.ReadArray(0, buffer, 0, bytesToRead);

                // ת��Ϊ�ṹ��
                var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
                try
                {
                    var sharedData = Marshal.PtrToStructure<SharedMemoryBlock>(handle.AddrOfPinnedObject());
                    return ConvertToSystemInfo(sharedData);
                }
                finally
                {
                    handle.Free();
                }
            }
            catch (Exception ex)
            {
                Log.Error(ex, "��ȡ����ϵͳ��Ϣʧ�ܣ�����ʹ�ü򻯷���");
                
                // ����ṹ�����ʧ�ܣ����Լ򻯵Ķ�ȡ����
                return ReadSimplifiedSystemInfo();
            }
        }

        private SystemInfo ReadSimplifiedSystemInfo()
        {
            var systemInfo = new SystemInfo();

            try
            {
                // ����ֱ�Ӷ�ȡ�ؼ������ֶ�
                // CPU���� (ƫ����0��wchar_t[128])
                string cpuName = ReadWideString(0, 128);
                if (!string.IsNullOrWhiteSpace(cpuName))
                {
                    systemInfo.CpuName = cpuName;
                }
                else
                {
                    systemInfo.CpuName = "�޷���ȡCPU��Ϣ";
                }

                // ���������ڴ���Ϣ������C++�ṹ�����ƫ������
                int offset = 128 * 2; // ����wchar_t[128] cpuName
                
                try
                {
                    systemInfo.PhysicalCores = _accessor!.ReadInt32(offset);
                    systemInfo.LogicalCores = _accessor.ReadInt32(offset + 4);
                    systemInfo.CpuUsage = _accessor.ReadDouble(offset + 8);
                    systemInfo.PerformanceCores = _accessor.ReadInt32(offset + 16);
                    systemInfo.EfficiencyCores = _accessor.ReadInt32(offset + 20);
                    
                    // �����ڴ����ݣ���Լƫ���� offset + 40��
                    int memOffset = offset + 40;
                    systemInfo.TotalMemory = _accessor.ReadUInt64(memOffset);
                    systemInfo.UsedMemory = _accessor.ReadUInt64(memOffset + 8);
                    systemInfo.AvailableMemory = _accessor.ReadUInt64(memOffset + 16);
                    
                    // �¶����ݣ���Լƫ���� memOffset + 24��
                    int tempOffset = memOffset + 24;
                    systemInfo.CpuTemperature = _accessor.ReadDouble(tempOffset);
                    systemInfo.GpuTemperature = _accessor.ReadDouble(tempOffset + 8);
                    
                    Log.Debug($"�򻯶�ȡ�ɹ�: CPU={systemInfo.CpuName}, �ڴ�={systemInfo.TotalMemory/(1024*1024*1024)}GB");
                }
                catch (Exception ex)
                {
                    Log.Warning($"��ȡ��ֵ����ʧ��: {ex.Message}");
                    // ����Ĭ��ֵ
                }

                systemInfo.LastUpdate = DateTime.Now;
                return systemInfo;
            }
            catch (Exception ex)
            {
                Log.Error(ex, "�򻯶�ȡҲʧ��");
                
                return new SystemInfo
                {
                    CpuName = $"���ݶ�ȡʧ��: {ex.Message}",
                    PhysicalCores = 0,
                    LogicalCores = 0,
                    TotalMemory = 0,
                    LastUpdate = DateTime.Now
                };
            }
        }

        private SystemInfo ConvertToSystemInfo(SharedMemoryBlock sharedData)
        {
            var systemInfo = new SystemInfo();
            
            try
            {
                // CPU��Ϣ
                systemInfo.CpuName = SafeWideCharArrayToString(sharedData.cpuName) ?? "δ֪������";
                systemInfo.PhysicalCores = sharedData.physicalCores;
                systemInfo.LogicalCores = sharedData.logicalCores;
                systemInfo.PerformanceCores = sharedData.performanceCores;
                systemInfo.EfficiencyCores = sharedData.efficiencyCores;
                systemInfo.CpuUsage = sharedData.cpuUsage;
                systemInfo.PerformanceCoreFreq = sharedData.pCoreFreq;
                systemInfo.EfficiencyCoreFreq = sharedData.eCoreFreq;
                systemInfo.HyperThreading = sharedData.hyperThreading;
                systemInfo.Virtualization = sharedData.virtualization;

                // �ڴ���Ϣ
                systemInfo.TotalMemory = sharedData.totalMemory;
                systemInfo.UsedMemory = sharedData.usedMemory;
                systemInfo.AvailableMemory = sharedData.availableMemory;

                // �¶���Ϣ
                systemInfo.CpuTemperature = sharedData.cpuTemperature;
                systemInfo.GpuTemperature = sharedData.gpuTemperature;

                // GPU��Ϣ
                systemInfo.Gpus.Clear();
                for (int i = 0; i < Math.Min(sharedData.gpuCount, sharedData.gpus?.Length ?? 0); i++)
                {
                    var gpu = sharedData.gpus[i];
                    systemInfo.Gpus.Add(new GpuData
                    {
                        Name = SafeWideCharArrayToString(gpu.name) ?? "δ֪GPU",
                        Brand = SafeWideCharArrayToString(gpu.brand) ?? "δ֪Ʒ��",
                        Memory = gpu.memory,
                        CoreClock = gpu.coreClock,
                        IsVirtual = gpu.isVirtual
                    });
                }

                // ������������Ϣ
                systemInfo.Adapters.Clear();
                for (int i = 0; i < Math.Min(sharedData.adapterCount, sharedData.adapters?.Length ?? 0); i++)
                {
                    var adapter = sharedData.adapters[i];
                    systemInfo.Adapters.Add(new NetworkAdapterData
                    {
                        Name = SafeWideCharArrayToString(adapter.name) ?? "δ֪����",
                        Mac = SafeWideCharArrayToString(adapter.mac) ?? "00-00-00-00-00-00",
                        IpAddress = SafeWideCharArrayToString(adapter.ipAddress) ?? "δ����",
                        AdapterType = SafeWideCharArrayToString(adapter.adapterType) ?? "δ֪����",
                        Speed = adapter.speed
                    });
                }

                // ������Ϣ
                systemInfo.Disks.Clear();
                for (int i = 0; i < Math.Min(sharedData.diskCount, sharedData.disks?.Length ?? 0); i++)
                {
                    var disk = sharedData.disks[i];
                    systemInfo.Disks.Add(new DiskData
                    {
                        Letter = (char)disk.letter,
                        Label = SafeWideCharArrayToString(disk.label) ?? "δ����",
                        FileSystem = SafeWideCharArrayToString(disk.fileSystem) ?? "δ֪",
                        TotalSize = disk.totalSize,
                        UsedSpace = disk.usedSpace,
                        FreeSpace = disk.freeSpace
                    });
                }

                // �¶ȴ�������Ϣ
                systemInfo.Temperatures.Clear();
                for (int i = 0; i < Math.Min(sharedData.tempCount, sharedData.temperatures?.Length ?? 0); i++)
                {
                    var temp = sharedData.temperatures[i];
                    string sensorName = SafeWideCharArrayToString(temp.sensorName) ?? $"������{i}";
                    systemInfo.Temperatures.Add(new TemperatureData
                    {
                        SensorName = sensorName,
                        Temperature = temp.temperature
                    });
                }

                // ���ü������ֶ�
                if (systemInfo.Gpus.Count > 0)
                {
                    var firstGpu = systemInfo.Gpus[0];
                    systemInfo.GpuName = firstGpu.Name;
                    systemInfo.GpuBrand = firstGpu.Brand;
                    systemInfo.GpuMemory = firstGpu.Memory;
                    systemInfo.GpuCoreFreq = firstGpu.CoreClock;
                    systemInfo.GpuIsVirtual = firstGpu.IsVirtual;
                }

                if (systemInfo.Adapters.Count > 0)
                {
                    var firstAdapter = systemInfo.Adapters[0];
                    systemInfo.NetworkAdapterName = firstAdapter.Name;
                    systemInfo.NetworkAdapterMac = firstAdapter.Mac;
                    systemInfo.NetworkAdapterIp = firstAdapter.IpAddress;
                    systemInfo.NetworkAdapterType = firstAdapter.AdapterType;
                    systemInfo.NetworkAdapterSpeed = firstAdapter.Speed;
                }

                systemInfo.LastUpdate = DateTime.Now;

                Log.Debug($"? �ɹ�����ϵͳ��Ϣ: CPU={systemInfo.CpuName}, �ڴ�={systemInfo.TotalMemory / (1024*1024*1024)}GB, GPU����={systemInfo.Gpus.Count}, ��������={systemInfo.Adapters.Count}, ��������={systemInfo.Disks.Count}");

                return systemInfo;
            }
            catch (Exception ex)
            {
                Log.Error(ex, "ת��ϵͳ��Ϣʱ�������󣬻��˵��򻯷���");
                return ReadSimplifiedSystemInfo();
            }
        }

        private string ReadWideString(int offset, int maxChars)
        {
            try
            {
                var chars = new List<char>();
                for (int i = 0; i < maxChars; i++)
                {
                    ushort wchar = _accessor!.ReadUInt16(offset + i * 2);
                    if (wchar == 0) break;
                    chars.Add((char)wchar);
                }
                return new string(chars.ToArray()).Trim();
            }
            catch
            {
                return string.Empty;
            }
        }

        private string? SafeWideCharArrayToString(ushort[]? wcharArray)
        {
            if (wcharArray == null || wcharArray.Length == 0)
                return null;

            try
            {
                var chars = new List<char>();
                foreach (ushort wchar in wcharArray)
                {
                    if (wchar == 0) break;
                    chars.Add((char)wchar);
                }
                
                if (chars.Count == 0)
                    return null;

                string result = new string(chars.ToArray()).Trim();
                return string.IsNullOrWhiteSpace(result) ? null : result;
            }
            catch (Exception ex)
            {
                Log.Debug($"���ַ�����ת��ʧ��: {ex.Message}");
                return null;
            }
        }

        public void Dispose()
        {
            if (_disposed)
                return;

            lock (_lock)
            {
                _accessor?.Dispose();
                _mmf?.Dispose();
                _accessor = null;
                _mmf = null;
                IsInitialized = false;
                _disposed = true;
            }

            GC.SuppressFinalize(this);
        }

        ~SharedMemoryService()
        {
            Dispose();
        }
    }
}