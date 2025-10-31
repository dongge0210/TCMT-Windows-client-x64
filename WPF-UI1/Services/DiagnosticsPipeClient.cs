using System;using System.IO;using System.IO.Pipes;using System.Text;using System.Text.Json;using System.Threading;using System.Threading.Tasks;using Serilog;
namespace WPF_UI1.Services {
 public class DiagnosticsPipeSnapshot { public long timestamp{get;set;} public uint abiVersion{get;set;} public uint writeSequence{get;set;} public uint snapshotVersion{get;set;} public ushort cpuLogicalCores{get;set;} public ulong memoryTotalMB{get;set;} public ulong memoryUsedMB{get;set;} public int expectedSize{get;set;} public Offsets offsets{get;set;} public string[] logs{get;set;} = Array.Empty<string>(); }
 public class Offsets { public int tempSensors{get;set;} public int tempSensorCount{get;set;} public int smartDisks{get;set;} public int smartDiskCount{get;set;} public int futureReserved{get;set;} public int sharedmemHash{get;set;} public int extensionPad{get;set;} }
 public sealed class DiagnosticsPipeClient : IDisposable {
 private readonly CancellationTokenSource _cts=new();
 public event Action<DiagnosticsPipeSnapshot>? SnapshotReceived;
 public void Start(){ Task.Run(RunAsync); }
 private async Task RunAsync(){ while(!_cts.IsCancellationRequested){ try{ using var pipe=new NamedPipeClientStream(".","SysMonDiag",PipeDirection.In); await pipe.ConnectAsync(3000,_cts.Token); using var br=new BinaryReader(pipe,Encoding.UTF8,false); while(pipe.IsConnected && !_cts.IsCancellationRequested){ var lenBytes=br.ReadBytes(4); if(lenBytes.Length<4) break; int len=BitConverter.ToInt32(lenBytes,0); var payload=br.ReadBytes(len); if(payload.Length<len) break; var json=Encoding.UTF8.GetString(payload); try{ var snap=JsonSerializer.Deserialize<DiagnosticsPipeSnapshot>(json); if(snap!=null) SnapshotReceived?.Invoke(snap); } catch(Exception ex){ Log.Debug("解析诊断JSON失败: "+ex.Message); } } } catch(Exception ex){ Log.Debug("管道连接失败: "+ex.Message); await Task.Delay(2000,_cts.Token); } } }
 public void Dispose(){ _cts.Cancel(); }
 }
}
