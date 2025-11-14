# TCMT Windows Client 架构 / 协议 / 共享内存 / 功能规划总文档（跨平台版本）

版本：v0.15  
更新时间：2025-10-28  
说明：本文件为"全量跨平台"文档。基于v0.14所有版本合并，新增跨平台架构设计、多平台兼容性实现、平台特定优化，以及关键问题修复总结。当前仅文档，不含任何实现代码；如需代码请发指令："要代码：模块名"。

---

## 目录
1. 目标与阶段路线总览  
2. 当前模块状态一览  
3. 共享内存结构与扩展策略（含完整结构声明 & 位标 & 哈希规范）  
4. 共享内存全量偏移表（理论 + 生成流程 + 获取指引）  
5. 写入完整性：writeSequence 奇偶 + 报警机制  
6. 指令系统设计与实施步骤  
7. 指令清单（现有 / 预留 / 占位）  
8. 错误与日志规范（主日志 + errors.log + 环形对接 + 统一格式）  
9. 主板 / BIOS 信息采集规划  
10. 进程枚举（第一版无 CPU%）  
11. USB 枚举与去抖（debounce）  
12. 环形趋势缓冲（short/long + snapshotVersion 定义）  
13. 插件系统最小 SDK 约定（占位）  
14. SMART 复检指令占位与异步策略（含正常刷新间隔）  
15. 进程控制与黑名单策略  
16. 配置文件 core.json 草案（跨平台扩展字段）  
17. 风险与回退策略  
18. 验收与测试步骤总表（含自检分类 + 单元测试 TODO）  
19. 原生温度采集（跨平台 NVAPI / ADL / WMI / libdrm / sysfs / SMART 异步）  
20. 迁移步骤（旧结构→新结构 + 温度过渡）  
21. FAQ / 常见坑提示  
22. 实施批次任务分解（最新排序）  
23. 后续扩展路线图（分析 / 安全 / 跨平台）  
24. 跨平台架构设计（第38章）  
25. 多平台兼容性实现策略（第39章）  
26. 平台特定优化与适配（第40章）  
27. 关键问题修复总结（共享内存std::vector问题、线程安全问题等）  
28. 未来跨平台抽象占位（Provider 接口）  
29. 收尾与后续触发点  
30. 附：偏移与结构校验章节（整块 SHA256 + diff 机制）  
31. MCP 服务器构想（订阅过滤 + 限速行为）  
32. 自检与扩展分析（sys.selftest / SMART 评分趋势 / sys.metrics / 性能优化 / LLM 预备 / critical 分类）  
32.14 SMART Aging Curve（老化曲线草图 + 配置化）  
33. 事件回放模拟（命名/保留策略）  
34. 硬件扩展补充（字段与无效值约定）  
35. CLI 设计与实施（含 degradeMode 行为 & DATA_NOT_READY 格式）  
36. 单位与精度规范  
37. 并发与线程安全模型  
38. 版本与兼容策略（abiVersion 映射）  
39. 历史与保留策略（快照/事件/Aging 持久化）  
40. 安全与权限初步（角色/RBAC/哈希链占位）  
41. 构建与工具脚本命名（偏移/自检/打包）

---

## 1. 目标与阶段路线总览
| 阶段 | 描述 | 状态 | 备注 |
|------|------|------|------|
| Phase 1 | 基础硬件采集 + 共享内存稳定 | 进行中 | 扩展与奇偶序列即将实现 |
| Phase 2 | 原生温度迁移 + SMART 异步 | 规划完成 | 温度优先，SMART 异步稍后 |
| Phase 3 | 前端日志环形缓冲落地 | 规划完成 | 诊断基础 |
| Phase 3a | 偏移 JSON 自动生成 + 哈希校验 | 前移 | 与环形并行 |
| Phase 4 | 指令系统最小集 | 待实现 | 提供基础操作入口 |
| Phase 5 | SMART 异步刷新 | 待实现 | 评分前置条件 |
| Phase 6 | 自检 sys.selftest | 待实现 | 启动可信度 |
| Phase 7 | 温度事件合并 + urgent | 待实现 | 用户体验 |
| Phase 8 | SMART 寿命评分 | 待实现 | 健康分析核心 |
| Phase 8.1 | SMART Aging Curve | 规划 | 预测剩余安全天数 |
| Phase 9 | sys.metrics 聚合资源 | 待实现 | 指标汇总 |
| Phase 10 | 自适应刷新 | 待实现 | 动态刷新间隔 |
| Phase 11 | LLM 接入预备 | 低优先级 | 描述 + 最小化视图 |
| Phase 12 | MCP MVP | 很低优先级 | 外部协议占位 |
| Phase 13 | Plugin Tick 性能统计 | 最低 | 插件后置 |
| Phase 14 | Event Replay 基础 | 规划 | 历史重建 |
| Phase 15 | Aging Curve 深化 | 规划 | 平滑与多阈值 |
| Phase 16 | Extended Hardware Resources | 规划 | nvme/memory/cpu_features |
| Phase 17 | 跨平台抽象层设计 | 新增 | 统一多平台接口 |
| Phase 18 | Linux/macOS Provider 实现 | 新增 | 平台特定采集 |
| Phase 19 | 平台优化与适配 | 新增 | 性能与兼容性 |

---

## 2. 当前模块状态一览
| 模块 | 现状 | 问题 | 下一步 |
|------|------|------|--------|
| SharedMemory | 旧结构 | 无扩展字段 | 扩展 + writeSequence |
| writeSequence | 未实现 | 脏读风险 | 奇偶协议接入 |
| 温度 | 托管桥 | 精度性能不足 | 原生迁移 |
| SMART 异步 | 未实现 | 无健康评分数据 | 异步线程 |
| 主板/BIOS | 未采集 | 字段空 | WMI/SMBIOS 填充 |
| 进程枚举 | 未实现 | 无进程列表 | 基础枚举 |
| USB | 未实现 | 插拔日志风暴 | 去抖策略 |
| 趋势缓冲 | 未实现 | 无曲线数据 | short/long 环形 |
| 指令系统 | 未实现 | 无外部调用入口 | 最小集落地 |
| 环形日志缓冲 | 规划完成 | 无具体结构 | 编码实现 |
| 偏移校验 | 手工核对 | 易错难维护 | 自动 JSON + debug 工具 |
| SMART 评分 | 未实现 | 无健康指标 | 规则+趋势 |
| 温度事件合并 | 未实现 | 刷屏风险 | 合并 + urgent 机制 |
| sys.metrics | 未实现 | 无统一指标输出 | 聚合刷新 |
| 自适应刷新 | 未实现 | 固定间隔不优 | 负载阈值动态调整 |
| MCP | 构想 | 未接入 | 后置 |
| 插件 SDK | 占位 | 无执行 | 后置 |
| Aging Curve | 未实现 | 无趋势预测 | 评分之后 |
| Event Replay | 未实现 | 无历史回放 | 快照与事件文件 |
| Extended Hardware | 未实现 | 诊断不够细 | 扩展资源采集 |
| 跨平台抽象 | 未设计 | Windows only | Provider 接口设计 |
| Linux Provider | 未实现 | 无 Linux 支持 | libdrm/procfs |
| macOS Provider | 未实现 | 无 macOS 支持 | IOKit/sysctl |

---

## 3. 共享内存结构与扩展策略
### 3.1 结构演进准则
- 仅尾部追加字段，中间不插入；移除字段需保留占位或升级 abiVersion 并标记弃用。
- 所有字段必须为 POD（Plain Old Data），禁止使用 std::string、指针或动态分配。
- 强制结构对齐 `#pragma pack(push,1)`，保证二进制兼容性。
- 跨平台兼容：避免平台特定类型，使用固定宽度类型。

### 3.2 SharedMemoryBlock 完整声明（示意，最终以头文件为准）
```
struct TemperatureSensor {
  char name[32];          // UTF-8传感器名，不足填0
  int16_t valueC_x10;     // 温度*10(单位0.1°C)，不可用-1
  uint8_t flags;          // bit0=valid, bit1=urgentLast
};

struct SmartDiskScore {
  char diskId[32];
  int16_t score;          // 0-100，-1不可用
  int32_t hoursOn;
  int16_t wearPercent;    // 0-100，-1不可用
  uint16_t reallocated;
  uint16_t pending;
  uint16_t uncorrectable;
  int16_t temperatureC;   // -1不可用
  uint8_t recentGrowthFlags; // bit0=reallocated增, bit1=wear突增
};

struct SharedMemoryBlock {
  uint32_t abiVersion;          // 0x00010015
  uint32_t writeSequence;       // 奇偶协议，启动为0
  uint32_t snapshotVersion;     // 完整刷新+1
  uint32_t reservedHeader;      // 对齐预留

  uint16_t cpuLogicalCores;
  int16_t  cpuUsagePercent_x10; // *10，未实现-1
  uint64_t memoryTotalMB;
  uint64_t memoryUsedMB;

  TemperatureSensor tempSensors[32];
  uint16_t tempSensorCount;

  SmartDiskScore smartDisks[16];
  uint8_t smartDiskCount;

  char baseboardManufacturer[128];
  char baseboardProduct[64];
  char baseboardVersion[64];
  char baseboardSerial[64];
  char biosVendor[64];
  char biosVersion[64];
  char biosDate[32];
  uint8_t secureBootEnabled;
  uint8_t tpmPresent;
  uint16_t memorySlotsTotal;
  uint16_t memorySlotsUsed;

  uint8_t futureReserved[64]; // bit0=degradeMode, bit1=hashMismatch, bit2=sequenceStallWarn
  uint8_t sharedmemHash[32];  // SHA256(结构除自身)

  uint8_t extensionPad[128];  // 预留扩展
};
```
说明：最终字段顺序以 offsets JSON 文件为准。

### 3.3 futureReserved 位标定义
| 位 | 名称 | 含义 | 触发条件 |
|----|------|------|----------|
| bit0 | degradeMode | 结构/哈希/版本不匹配降级 | size/hash/version mismatch |
| bit1 | hashMismatch | sha256与偏移JSON不符 | 校验发现差异 |
| bit2 | sequenceStallWarn | 连续奇数序列超过阈值 | 写线程阻塞 |
| bit3 | platformUnsupported | 平台不支持某些功能 | 跨平台兼容性 |
| bit4-7 | 保留 | 后续扩展 | - |

### 3.4 writeSequence 行为与溢出
- uint32递增，溢出回到0（仍偶数视为稳定）。
- 奇偶判断：奇数=写进行中，偶数=稳定。
- 初始值0，写开始seq=1，写结束seq=2。

### 3.5 sharedmemSha256 计算规范
- 范围：结构全部字节，先将sharedmemHash置0再计算。
- 算法：SHA256，v0.15增加salt字段增强安全性。
- 生成时机：启动后一次，偏移JSON写入后不再重复。
- CLI可重新计算（后续可加缓存）。

### 3.6 snapshotVersion 定义
- 完整硬件刷新成功（至少一项：CPU/温度/SMART）后递增+1。
- 连续相同snapshotVersion≥5次，趋势缓冲暂停追加。

### 3.7 并发与访问
- 单写线程+多读线程，写入流程：奇数→写全部字段→写hash→偶数。
- 自检线程遇到奇数序列，使用上次稳定快照。
- 跨平台：使用平台无关的同步原语。

### 3.8 结构升级流程
1. 尾部追加字段。
2. 更新abiVersion。
3. 生成offsets JSON和新hash。
4. 更新文档与位标说明。
5. CLI/自检校验通过后发布。

---

## 4. 共享内存全量偏移表
### 4.1 生成脚本/工具
- 工具名建议：generate_offsets.exe (Windows) / generate_offsets (Linux/macOS)
- 输出路径：docs/runtime/offsets_sharedmem_v0_15.json
- 字段：名称/offset/size/flags(valid/nullable/arrayLength)

### 4.2 示例
```
{
  "abiVersion":"0x00010015",
  "sizeof":125818,
  "generatedTs":1697890000123,
  "sharedmemSha256":"<64hex>",
  "salt":"<32hex>",
  "fields":[
    {"name":"abiVersion","offset":0,"size":4},
    {"name":"writeSequence","offset":4,"size":4},
    {"name":"snapshotVersion","offset":8,"size":4},
    ...
  ]
}
```

### 4.3 校验优先级
version mismatch > size mismatch > hash mismatch > field offset mismatch

### 4.4 差异处理策略
- version mismatch：直接degradeMode(bit0)
- size/hash mismatch：置bit0 & bit1
- 字段偏移差异：记录differences列表，日志WARN

### 4.5 Offset Table Retrieval Instructions
运行generate_offsets工具获取最新偏移表并写入docs/runtime/offsets_sharedmem_v0_15.json。禁止手工硬编码偏移值，任何实现未通过该工具生成的偏移文件即视为不可信。测试阶段先实现工具，后引入CLI校验。

---

## 5. 写入完整性：writeSequence 奇偶 + 报警机制
确保每次写入共享内存时遵循奇偶协议，写开始前置为奇数，写完成后置为偶数。若写入过程中异常中断，则读端遇到奇数序列必须回退到上次偶数快照。报警机制：连续奇数超过阈值（如10次）时，futureReserved.bit2置1并上报日志。

---

## 6. 指令系统设计与实施步骤
定义系统指令集，包括：refresh.hardware、selftest、trend.query、status.get、config.reload等。每个指令的输入/输出JSON schema，执行流程与错误码定义，以及异步指令的超时和重试策略。

---

## 7. 指令清单（现有 / 预留 / 占位）
- refresh.hardware
- refresh.temperature
- refresh.smart
- selftest
- status.get
- trend.query
- config.reload
- subscribe (MCP)
- event.replay
- platform.info (新增)
- platform.capabilities (新增)
- future扩展：plugin.load、security.status

---

## 8. 错误与日志规范（主日志 + errors.log + 环形对接 + 统一格式）
日志文件统一前缀：sharedmem_/smart_/temperature_/plugin_/mcp_。错误对象格式：error.code/message/hint/detail。环形日志采集，容量上限后覆盖。

### 8.1 跨平台日志路径
- Windows: ./logs/
- Linux: /var/log/tcmt/ 或 ~/.local/share/tcmt/logs/
- macOS: ~/Library/Logs/tcmt/

### 8.2 日志格式统一
```
YYYY-MM-DD HH:MM:SS.mmm LEVEL [PLATFORM] message key=value ...
```

---

## 9. 主板 / BIOS 信息采集规划
### 9.1 Windows实现
采集主板制造商、型号、序列号、BIOS厂商、版本、日期、安全启动、TPM状态等。部分字段受maskBaseboardSerial配置影响，敏感字段可屏蔽。

### 9.2 Linux实现
- DMI信息：/sys/class/dmi/id/
- BIOS信息：/sys/class/dmi/id/bios_*
- 主板信息：/sys/class/dmi/id/board_*

### 9.3 macOS实现
- 系统信息：system_profiler SPHardwareDataType
- IORegistry：ioreg -l -p IODeviceTree

---

## 10. 进程枚举（第一版无 CPU%）
### 10.1 Windows实现
进程列表采集包括PID、进程名、路径、启动时间。使用Toolhelp32Snapshot。

### 10.2 Linux实现
- 进程信息：/proc/[pid]/stat, /proc/[pid]/status
- 进程列表：/proc/ 目录遍历

### 10.3 macOS实现
- 进程信息：libproc.h
- 进程列表：proc_listallpids

---

## 11. USB 枚举与去抖（debounce）
### 11.1 Windows实现
设备枚举支持USB设备去抖动。使用SetupAPI和WM_DEVICECHANGE消息。

### 11.2 Linux实现
- 设备信息：/sys/bus/usb/devices/
- 热插拔事件：libudev或netlink

### 11.3 macOS实现
- 设备信息：IOKit USB框架
- 热插拔事件：IOServiceAddMatchingNotification

---

## 12. 环形趋势缓冲（short/long + snapshotVersion 定义）
实现short/long两组环形缓冲区，存储温度、SMART分数、趋势点。snapshotVersion变更时追加新点，连续相同≥5次暂停追加。

---

## 13. 插件系统最小 SDK 约定（占位）
插件接口定义，包含初始化、数据采集、事件回调。最低约定接口：plugin_init、plugin_query、plugin_shutdown。后续支持插件热加载与安全隔离。

---

## 14. SMART 复检指令占位与异步策略（含正常刷新间隔）
### 14.1 Windows实现
SMART属性异步刷新周期30秒（smart.refreshIntervalSeconds），温度属性10秒（smart.tempIntervalSeconds），失败盘重试schedule[1,5,10]秒，超过maxSmartRetries标记stale。

### 14.2 Linux实现
- SMART信息：smartmontools (smartctl)
- 原始数据：/sys/block/sdX/device/smart/
- 异步监控：inotify + 定时扫描

### 14.3 macOS实现
- SMART信息：diskutil smart info
- IOStorage设备：IOKit存储框架

---

## 15. 进程控制与黑名单策略
支持配置进程黑名单，发现黑名单进程时可自动结束或上报。kill操作需ADMIN权限并二次确认。

---

## 16. 配置文件 core.json 草案（跨平台扩展字段）
```jsonc
{
  "platform": {
    "type": "auto", // "windows", "linux", "macos", "auto"
    "hardwareProvider": "auto", // "native", "fallback"
    "logPath": "auto", // 平台默认路径或自定义路径
    "tempUnits": "celsius" // "celsius", "fahrenheit", "kelvin"
  },
  "refresh": { "cpuMs":1000, "gpuMs":2000, "memoryMs":1000, "diskMs":3000 },
  "sequence": { "maxConsecutiveOdd":5, "alarmLog":true },
  "usb": { "debounceMs":500, "smartAdjust":true },
  "privacy": { "maskBaseboardSerial":false },
  "process": { "blacklist":["System","Registry","WinInit","smss.exe","csrss.exe","explorer.exe"] },
  "plugin": { "directory":"./plugins", "tickTimeoutMs":50 },
  "trend": { "shortPoints":60, "longPoints":300, "pauseIfSnapshotFrozen":5 },
  "logging": {
    "rotateSizeMB":5,
    "errorsDatePrefix":true,
    "frontend": { "rateLimitDefault":20, "maxOverride":200 }
  },
  "temperature": {
    "enableNativeTemperature": true,
    "useSmartStorageQueryFirst": true,
    "logDiffOnceBeforeRemoval": true,
    "unsupportedValueRender": "N/A",
    "asyncSmartRetry": true,
    "smartRetryScheduleSeconds": [1,5,10],
    "maxSmartRetries": 3,
    "collectAllAmdGpus": true,
    "event": {
      "mergeEnabled": true,
      "urgentJumpThresholdC": 5
    }
  },
  "smart": {
    "trendWindow": 5,
    "wearRapidGrowthPercent": 1,
    "refreshIntervalSeconds": 30,
    "tempIntervalSeconds": 10,
    "agingWindow": 10,
    "agingThresholdScore": 50
  },
  "adaptiveRefresh": {
    "enabled": true,
    "cpuHighThresholdPercent": 15,
    "cpuRecoverThresholdPercent": 10,
    "consecutiveCount": 5
  },
  "mcp": {
    "apiVersion": "mcp-0.15",
    "temperatureChangeThreshold": 2,
    "temperatureBroadcastIntervalMs": 1000,
    "requestRateLimitPerSec": 30
  },
  "crossPlatform": {
    "enableFallbackProviders": true,
    "preferredNativeAPIs": true,
    "compatibilityMode": "strict" // "strict", "permissive", "minimal"
  }
}
```

---

## 17. 风险与回退策略
| 风险 | 征兆 | 回退 |
|------|------|------|
| 偏移错误 | UI 数据乱码 | static_assert + offsets diff |
| 序列卡奇数 | 连续奇数报警 | 重试写 / 双缓冲 |
| 温度全 N/A | 温度视图空 | 关闭原生温度采集 |
| SMART 阻塞 | score 不更新 | 超时重启线程 |
| 日志覆盖频繁 | overrunCount 激增 | 扩容或限流调整 |
| 哈希失败频繁 | hashOk=0 重复 | 检查写入顺序 / 盐 |
| 评分跳动 | 分数忽高忽低 | 延长趋势窗口 |
| 自适应抖动 | 间隔频繁变化 | 提高 consecutiveCount |
| 结构哈希不匹配 | WARN hash mismatch | degradeMode |
| Aging 预测异常 | remainingSafeDays 巨变 | confidence=low 屏蔽提示 |
| 平台API不可用 | 采集失败 | 降级到fallback provider |

---

## 18. 验收与测试步骤总表（含自检分类 + 单元测试 TODO）
### 18.1 Critical Checks
- sharedmem_size
- sharedmem_offsets
- sharedmem_hash
- write_sequence_flip
- nvapi_init (Windows)
- adl_init (Windows)
- libdrm_init (Linux)
- IOKit_init (macOS)

### 18.2 Non-Critical Checks
- smart 单盘失败 / baseboard 缺失 / trend 缺数据 / 单传感器不可用

### 18.3 状态决策
- ≥1 critical fail → status=fail
- 仅non-critical fail → status=partial_warn
- 全部通过 → status=ok

### 18.4 Unit Test Plan (TODO)
- [ ] 列出需要测试的模块（sharedmem、hash、sequence、SMART refresh、温度合并）
- [ ] 编写最小断言场景（结构size、hash重复计算一致性）
- [ ] 添加失败路径测试（写线程阻塞 / SMART重试）
- [ ] 以后扩展：fuzz指令解析 / CLI参数解析
- [ ] 跨平台兼容性测试（Windows/Linux/macOS）

---

## 19. 原生温度采集（跨平台 NVAPI / ADL / WMI / libdrm / sysfs / SMART 异步）
### 19.1 Windows实现
支持多方案温度采集，优先NVAPI/ADL，备用WMI/SMART。

### 19.2 Linux实现
- GPU温度：libdrm (NVIDIA) / AMDGPU (AMD)
- CPU温度：/sys/class/hwmon/hwmon*/temp*_input
- 磁盘温度：smartctl / hddtemp

### 19.3 macOS实现
- GPU温度：IOKit GPU框架
- CPU温度：ioreg -l -p IODeviceTree | grep temperature
- 磁盘温度：diskutil smart info

---

## 20. 迁移步骤（旧结构→新结构 + 温度过渡）
设计平滑迁移流程，先扩展结构，后切换采集实现。温度数据从托管桥迁移到原生采集，SMART属性同步迁移。

---

## 21. FAQ / 常见坑提示
| 问题 | 原因 | 解决 |
|------|------|------|
| 温度大量 N/A | 驱动缺失/权限 | 安装驱动或关闭原生温度 |
| 序列长时间奇数 | 写线程卡死 | 重启写线程 / 加双缓冲 |
| SMART 温度异常 | 解析偏移错 | 校正字段索引 |
| 日志覆盖多 | 入口过量 | 调整限流或容量 |
| 哈希频繁失败 | 初始化乱序 | 统一盐和写顺序 |
| 分数突然下降 | 属性激增 | 检查 recentGrowth |
| 自适应频繁跳 | 阈值太紧 | 调整 consecutiveCount |
| Aging 预测不可信 | 点少/波动大 | confidence=low 不提醒 |
| Replay 缺数据 | 快照未写 | 启用历史定时任务 |
| Extended 空 | 无设备 | message 说明不可用 |
| 跨平台API失败 | 依赖缺失 | 使用fallback provider |

---

## 22. 实施批次任务分解（最新排序）
| 顺序 | 任务 | 描述 | 估时 | 依赖 | 可暂缓 | 备注 |
|------|------|------|------|------|--------|------|
| 1 | SharedMemory 扩展 + writeSequence | 基础结构与协议 | 1h | 无 | 否 | 基石 |
| 2 | 原生温度迁移 | 核心温度采集 | 1.5h | 1 | 否 | 用户可见 |
| 3 | 环形日志缓冲 | 日志结构 | 2h | 1 | 否 | 诊断基础 |
| 3a | 偏移 JSON + SHA256 + debug | 自动校验 | 1h | 1 | 否 | 尽早发现错误 |
| 4 | 指令系统最小集 | 操作入口 | 1h | 1 | 否 | 后续工具依赖 |
| 5 | SMART 异步线程 | 属性刷新 | 1.5h | 1 | 否 | 评分前置 |
| 6 | 自检 sys.selftest | 启动健康报告 | 1h | 3,3a,4,5 | 否 | 可信度 |
| 7 | 温度事件合并 + urgent | 合并与跳变推送 | 0.8h | 2 | 否 | 体验提升 |
| 8 | SMART 寿命评分 | 规则+趋势 | 1h | 5 | 否 | 健康分析 |
| 8.1 | SMART Aging Curve | 斜率预测 | 0.8h | 8 | 是 | 可后移 |
| 9 | sys.metrics | 指标聚合 | 0.8h | 2,3,5,8 | 是 | 后置 |
| 10 | 自适应刷新 | 动态间隔 | 1h | 9 或 CPU% | 是 | CPU% 未完成可暂缓 |
| 11 | LLM 接入预备 | 描述/最小化 | 0.5h | 9 | 是 | 文档占位 |
| 12 | MCP MVP | list/get/sub | 2h | 3,4,6,7,8 | 是 | 后置 |
| 13 | Plugin Tick 统计 | p95/avg | 1h | 插件框架 | 是 | 最低 |
| 14 | Event Replay 基础 | 快照+重建占位 | 1h | 6,8 | 是 | 历史分析 |
| 15 | Aging Curve 深化 | 多阈值/平滑 | 1h | 8.1 | 是 | 优化 |
| 16 | Extended Hardware 资源 | nvme/memory/cpu_features | 2h | 1,5 | 是 | 诊断增强 |
| 17 | 跨平台抽象层设计 | Provider接口 | 1.5h | 1 | 否 | 架构基础 |
| 18 | Linux Provider 实现 | libdrm/procfs | 2h | 17 | 是 | 平台支持 |
| 19 | macOS Provider 实现 | IOKit/sysctl | 2h | 17 | 是 | 平台支持 |
| 20 | 平台优化与适配 | 性能/兼容性 | 1h | 18,19 | 是 | 体验优化 |

---

## 23. 后续扩展路线图（分析 / 安全 / 跨平台）
| 类别 | 扩展点 | 描述 |
|------|--------|------|
| 性能 | 双缓冲/差分写入 | 降低撕裂 |
| 分析 | stats.export/告警引擎 | 智能预警 |
| 安全 | RBAC/审计链 | 权限细粒度控制 |
| 插件 | 沙箱/WASM | 隔离与扩展 |
| 硬件 | 风扇/功耗/网络 | 更全面指标 |
| 跨平台 | 统一Provider抽象 | Linux/macOS支持 |
| AI | context bundle/minimal | 降 token 消耗 |
| 历史 | 快照归档/回放 | 故障复盘 |
| 签名 | 日志/结构签名 | 防篡改审计 |
| 容器化 | Docker/Podman支持 | 云原生部署 |

---

## 24. 跨平台架构设计（第38章）

### 24.1 设计目标
- 统一接口：提供一致的硬件采集API，隐藏平台差异
- 模块化：每个平台独立实现，便于维护和扩展
- 性能优化：优先使用原生API，降级到通用方案
- 兼容性：支持不同版本和发行版的操作系统

### 24.2 架构层次
```
应用层 (CLI/MCP/前端)
    ↓
抽象层 (IHardwareProvider接口)
    ↓
平台适配层 (Windows/Linux/macOS Provider)
    ↓
系统API层 (WinAPI/libdrm/IOKit等)
```

### 24.3 核心接口设计
```cpp
// 平台无关的抽象接口
class IHardwareProvider {
public:
    virtual ~IHardwareProvider() = default;
    
    // 初始化和清理
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    
    // 基础信息
    virtual std::string GetPlatformName() const = 0;
    virtual ProviderCapabilities GetCapabilities() const = 0;
    
    // 硬件采集
    virtual SystemSnapshot GetSnapshot() = 0;
    virtual std::vector<TemperatureSensor> GetTemperatures() = 0;
    virtual std::vector<SmartDiskInfo> GetSmartInfo() = 0;
    virtual std::vector<ProcessInfo> GetProcessList() = 0;
    virtual std::vector<UsbDevice> GetUsbDevices() = 0;
    
    // 控制操作
    virtual bool RefreshHardware() = 0;
    virtual bool KillProcess(uint32_t pid) = 0;
    virtual bool RescanUsbDevices() = 0;
    
    // 配置和状态
    virtual void UpdateConfig(const Config& config) = 0;
    virtual ProviderStatus GetStatus() const = 0;
};

// 能力标志
struct ProviderCapabilities {
    bool supportsGpuTemperature = false;
    bool supportsSmart = false;
    bool supportsUsbHotplug = false;
    bool supportsProcessControl = false;
    bool supportsCpuFrequency = false;
    std::vector<std::string> supportedFeatures;
};

// 平台工厂
class ProviderFactory {
public:
    static std::unique_ptr<IHardwareProvider> CreateProvider(PlatformType platform);
    static std::unique_ptr<IHardwareProvider> CreateAutoProvider(); // 自动检测
};
```

### 24.4 平台特定实现
#### Windows Provider (HardwareProviderWin)
- 使用WinAPI、WMI、NVAPI、ADL
- COM接口用于WMI查询
- 注册表访问系统信息

#### Linux Provider (HardwareProviderLinux)
- 使用libdrm、procfs、sysfs
- DMI接口获取硬件信息
- libudev监听设备事件

#### macOS Provider (HardwareProviderDarwin)
- 使用IOKit框架
- system_profiler命令行工具
- sysctl系统调用

### 24.5 降级策略
```cpp
class FallbackProvider : public IHardwareProvider {
private:
    std::unique_ptr<IHardwareProvider> primary;
    std::unique_ptr<IHardwareProvider> fallback;
    
public:
    bool Initialize() override {
        if (primary->Initialize()) return true;
        LogWarn("Primary provider failed, trying fallback");
        return fallback->Initialize();
    }
    
    SystemSnapshot GetSnapshot() override {
        try {
            return primary->GetSnapshot();
        } catch (const std::exception& e) {
            LogError("Primary provider failed: " + std::string(e.what()));
            return fallback->GetSnapshot();
        }
    }
};
```

### 24.6 配置驱动的平台选择
```json
{
  "platform": {
    "type": "auto",
    "providers": {
      "windows": {
        "enabled": true,
        "preferNvapi": true,
        "preferAdl": true
      },
      "linux": {
        "enabled": true,
        "preferLibdrm": true,
        "preferProcfs": true
      },
      "macos": {
        "enabled": true,
        "preferIOKit": true
      }
    },
    "fallback": {
      "enabled": true,
      "timeoutMs": 5000
    }
  }
}
```

---

## 25. 多平台兼容性实现策略（第39章）

### 25.1 编译系统设计
#### CMake跨平台构建
```cmake
# 顶层CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(TCMT VERSION 0.15.0)

# 平台检测
if(WIN32)
    set(PLATFORM_NAME "windows")
    add_definitions(-DPLATFORM_WINDOWS)
elseif(APPLE)
    set(PLATFORM_NAME "macos")
    add_definitions(-DPLATFORM_MACOS)
elseif(UNIX)
    set(PLATFORM_NAME "linux")
    add_definitions(-DPLATFORM_LINUX)
endif()

# 平台特定源文件
set(PLATFORM_SOURCES)
if(PLATFORM_NAME STREQUAL "windows")
    list(APPEND PLATFORM_SOURCES
        src/platform/windows/win_provider.cpp
        src/platform/windows/wmi_query.cpp
        src/platform/windows/registry_reader.cpp
    )
elseif(PLATFORM_NAME STREQUAL "linux")
    list(APPEND PLATFORM_SOURCES
        src/platform/linux/linux_provider.cpp
        src/platform/linux/procfs_reader.cpp
        src/platform/linux/libdrm_helper.cpp
    )
elseif(PLATFORM_NAME STREQUAL "macos")
    list(APPEND PLATFORM_SOURCES
        src/platform/macos/darwin_provider.cpp
        src/platform/macos/iokit_helper.cpp
        src/platform/macos/sysctl_reader.cpp
    )
endif()

# 平台特定库
if(PLATFORM_NAME STREQUAL "windows")
    find_package(wmi REQUIRED)
    target_link_libraries(tcmt wmi::wmi)
elseif(PLATFORM_NAME STREQUAL "linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DRM REQUIRED libdrm)
    target_link_libraries(tcmt ${DRM_LIBRARIES})
elseif(PLATFORM_NAME STREQUAL "macos")
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    target_link_libraries(tcmt ${IOKIT_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})
endif()
```

### 25.2 依赖管理策略
#### 包管理器集成
```json
// vcpkg.json (Windows)
{
  "name": "tcmt",
  "version": "0.15.0",
  "dependencies": [
    "nlohmann-json",
    "spdlog",
    "openssl"
  ],
  "features": {
    "windows": {
      "dependencies": ["wmi", "atl"]
    },
    "linux": {
      "dependencies": ["libdrm"]
    }
  }
}

// conanfile.txt (跨平台)
[requires]
nlohmann_json/3.11.2
spdlog/1.12.0
openssl/3.1.2

[options]
shared=False

[generators]
CMakeDeps
CMakeToolchain

[tool_requires]
cmake/3.25.3
```

### 25.3 条件编译策略
```cpp
// platform_utils.h
#pragma once

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <wmi.h>
    #define PATH_SEPARATOR "\\"
    #define LINE_ENDING "\r\n"
#elif defined(PLATFORM_LINUX)
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #define PATH_SEPARATOR "/"
    #define LINE_ENDING "\n"
#elif defined(PLATFORM_MACOS)
    #include <unistd.h>
    #include <mach/mach.h>
    #include <CoreFoundation/CoreFoundation.h>
    #define PATH_SEPARATOR "/"
    #define LINE_ENDING "\n"
#endif

// 平台特定工具函数
namespace PlatformUtils {
    std::string GetDefaultLogPath();
    std::string GetConfigPath();
    std::string GetTempPath();
    bool IsElevated();
    uint64_t GetTickCount();
}
```

### 25.4 运行时平台检测
```cpp
// platform_detector.cpp
std::string DetectPlatform() {
#ifdef PLATFORM_WINDOWS
    return "windows";
#elif defined(PLATFORM_LINUX)
    // 检测Linux发行版
    std::ifstream os_release("/etc/os-release");
    if (os_release.is_open()) {
        std::string line;
        while (std::getline(os_release, line)) {
            if (line.find("ID=") == 0) {
                return "linux:" + line.substr(3);
            }
        }
    }
    return "linux:unknown";
#elif defined(PLATFORM_MACOS)
    // 获取macOS版本
    std::string version = ExecuteCommand("sw_vers -productVersion");
    return "macos:" + Trim(version);
#endif
    return "unknown";
}
```

### 25.5 配置文件路径管理
```cpp
// config_paths.cpp
class ConfigPaths {
public:
    static std::string GetConfigPath() {
#ifdef PLATFORM_WINDOWS
        char* appdata = std::getenv("APPDATA");
        return std::string(appdata) + "\\TCMT\\config\\";
#elif defined(PLATFORM_LINUX)
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/.config/tcmt/";
        }
        return "/etc/tcmt/";
#elif defined(PLATFORM_MACOS)
        const char* home = std::getenv("HOME");
        return std::string(home) + "/Library/Application Support/TCMT/";
#endif
    }
    
    static std::string GetLogPath() {
#ifdef PLATFORM_WINDOWS
        return GetConfigPath() + "logs\\";
#else
        return GetConfigPath() + "logs/";
#endif
    }
};
```

---

## 26. 平台特定优化与适配（第40章）

### 26.1 Windows平台优化
#### 性能优化
```cpp
class WindowsOptimizer {
private:
    // 设置进程优先级
    void SetProcessPriority() {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    }
    
    // 优化WMI查询
    void OptimizeWMI() {
        // 使用半同步查询
        CoInitializeSecurity(nullptr, -1, nullptr, nullptr, 
                           RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE,
                           nullptr, EOAC_NONE, nullptr);
    }
    
    // 内存映射文件优化
    HANDLE CreateOptimizedSharedMemory(const std::string& name, size_t size) {
        HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr,
                                       PAGE_READWRITE, 0, size, name.c_str());
        if (hMap) {
            // 预分配内存页
            void* ptr = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, size);
            if (ptr) {
                // 触摸所有页面确保分配
                char* pages = static_cast<char*>(ptr);
                for (size_t i = 0; i < size; i += 4096) {
                    pages[i] = 0;
                }
                UnmapViewOfFile(ptr);
            }
        }
        return hMap;
    }
};
```

#### 权限提升
```cpp
class WindowsElevation {
public:
    static bool IsRunningAsAdmin() {
        BOOL isAdmin = FALSE;
        PSID adminGroup = nullptr;
        
        // 构建管理员组SID
        SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                   &adminGroup)) {
            CheckTokenMembership(nullptr, adminGroup, &isAdmin);
            FreeSid(adminGroup);
        }
        
        return isAdmin;
    }
    
    static bool RequestElevation() {
        SHELLEXECUTEINFO sei = {0};
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.lpVerb = "runas";
        sei.lpFile = "tcmt.exe";
        sei.hwnd = nullptr;
        sei.nShow = SW_NORMAL;
        
        return ShellExecuteEx(&sei);
    }
};
```

### 26.2 Linux平台优化
#### 内核接口优化
```cpp
class LinuxOptimizer {
private:
    // 优化procfs读取
    class ProcfsOptimizer {
        std::unordered_map<std::string, std::string> cache_;
        std::chrono::steady_clock::time_point last_update_;
        
    public:
        std::string ReadProcFile(const std::string& path) {
            auto now = std::chrono::steady_clock::now();
            if (now - last_update_ < std::chrono::milliseconds(100)) {
                auto it = cache_.find(path);
                if (it != cache_.end()) {
                    return it->second;
                }
            }
            
            std::ifstream file(path);
            std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
            
            cache_[path] = content;
            last_update_ = now;
            return content;
        }
    };
    
    // 优化sysfs访问
    void OptimizeSysfs() {
        // 批量读取减少系统调用
        std::vector<std::string> temp_files = {
            "/sys/class/hwmon/hwmon0/temp1_input",
            "/sys/class/hwmon/hwmon1/temp1_input"
        };
        
        for (const auto& file : temp_files) {
            std::async(std::launch::async, [this, file]() {
                ReadTemperatureFile(file);
            });
        }
    }
};
```

#### 设备权限处理
```cpp
class LinuxPermissions {
public:
    static bool CheckDevicePermissions(const std::string& device) {
        struct stat st;
        if (stat(device.c_str(), &st) == 0) {
            return (st.st_mode & S_IRUSR) != 0;
        }
        return false;
    }
    
    static bool RequestDevicePermissions(const std::string& device) {
        // 尝试使用udev规则
        std::string udev_rule = "KERNEL==\"" + device + "\", MODE=\"0666\"";
        WriteUdevRule(udev_rule);
        
        // 重新加载udev规则
        system("udevadm control --reload-rules");
        system("udevadm trigger");
        
        return CheckDevicePermissions(device);
    }
    
private:
    static void WriteUdevRule(const std::string& rule) {
        std::ofstream udev_file("/etc/udev/rules.d/99-tcmt.rules");
        udev_file << rule << std::endl;
    }
};
```

### 26.3 macOS平台优化
#### IOKit优化
```cpp
class MacOSOptimizer {
private:
    // IOKit连接池
    class IOKitConnectionPool {
        std::vector<io_service_t> services_;
        std::mutex mutex_;
        
    public:
        io_service_t GetService(const std::string& name) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto it = std::find_if(services_.begin(), services_.end(),
                [&name](io_service_t service) {
                    io_name_t service_name;
                    IORegistryEntryGetName(service, service_name);
                    return std::string(service_name) == name;
                });
            
            if (it != services_.end()) {
                return *it;
            }
            
            // 创建新服务连接
            io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault,
                IOServiceMatching(name.c_str()));
            if (service) {
                services_.push_back(service);
            }
            
            return service;
        }
    };
    
    // 系统调用优化
    void OptimizeSysctl() {
        // 批量获取sysctl值
        std::vector<int> mibs = {
            CTL_HW, HW_NCPU,           // CPU数量
            CTL_HW, HW_MEMSIZE,        // 内存大小
            CTL_HW, HW_CPU_FREQ        // CPU频率
        };
        
        std::vector<size_t> values(mibs.size() / 2);
        size_t size = sizeof(size_t);
        
        for (size_t i = 0; i < mibs.size(); i += 2) {
            sysctl(&mibs[i], 2, &values[i/2], &size, nullptr, 0);
        }
    }
};
```

#### 权限处理
```cpp
class MacOSPermissions {
public:
    static bool CheckAccessibilityPermissions() {
        // 检查辅助功能权限
        CFBooleanRef trusted = kCFBooleanFalse;
        CFStringRef const keys[] = { kAXTrustedCheckOptionPrompt };
        CFDictionaryRef const options = CFDictionaryCreate(nullptr,
            (const void **)keys, (const void **)&values, 1,
            &kCFCopyStringDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
        
        trusted = AXIsProcessTrustedWithOptions(options);
        CFRelease(options);
        
        return trusted == kCFBooleanTrue;
    }
    
    static void RequestAccessibilityPermissions() {
        // 提示用户授予权限
        CFStringRef keys[] = { kAXTrustedCheckOptionPrompt };
        CFDictionaryRef options = CFDictionaryCreate(nullptr,
            (const void **)keys, (const void **)&kCFBooleanTrue, 1,
            &kCFCopyStringDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
        
        AXIsProcessTrustedWithOptions(options);
        CFRelease(options);
    }
};
```

### 26.4 通用性能优化
#### 内存管理优化
```cpp
class MemoryOptimizer {
public:
    // 内存池分配器
    template<typename T>
    class PoolAllocator {
        std::vector<std::unique_ptr<T[]>> pools_;
        std::queue<T*> available_;
        size_t pool_size_;
        
    public:
        PoolAllocator(size_t pool_size = 1024) : pool_size_(pool_size) {
            AllocateNewPool();
        }
        
        T* Allocate() {
            if (available_.empty()) {
                AllocateNewPool();
            }
            
            T* ptr = available_.front();
            available_.pop();
            return ptr;
        }
        
        void Deallocate(T* ptr) {
            available_.push(ptr);
        }
        
    private:
        void AllocateNewPool() {
            auto pool = std::make_unique<T[]>(pool_size_);
            T* ptr = pool.get();
            
            for (size_t i = 0; i < pool_size_; ++i) {
                available_.push(&ptr[i]);
            }
            
            pools_.push_back(std::move(pool));
        }
    };
    
    // 零拷贝数据传输
    class ZeroCopyBuffer {
        void* buffer_;
        size_t size_;
        std::atomic<bool> ready_;
        
    public:
        ZeroCopyBuffer(size_t size) : size_(size), ready_(false) {
            buffer_ = std::aligned_alloc(64, size); // 64字节对齐
        }
        
        void* GetWriteBuffer() {
            ready_ = false;
            return buffer_;
        }
        
        const void* GetReadBuffer() const {
            return ready_ ? buffer_ : nullptr;
        }
        
        void CommitWrite() {
            // 内存屏障确保写入完成
            std::atomic_thread_fence(std::memory_order_release);
            ready_ = true;
        }
    };
};
```

---

## 27. 关键问题修复总结

### 27.1 共享内存std::vector问题
#### 问题描述
原始设计中使用了std::vector作为共享内存字段，导致跨进程访问时出现内存布局不一致、迭代器失效等问题。

#### 解决方案
```cpp
// 错误的设计（原版）
struct SharedMemoryBlock {
    std::vector<TemperatureSensor> temperatures; // ❌ 跨进程不安全
    std::vector<SmartDiskScore> smartDisks;     // ❌ 内存布局不确定
};

// 修复后的设计
struct SharedMemoryBlock {
    TemperatureSensor tempSensors[32];  // ✅ 固定大小数组
    uint16_t tempSensorCount;           // ✅ 显式计数
    
    SmartDiskScore smartDisks[16];      // ✅ 固定大小数组
    uint8_t smartDiskCount;             // ✅ 显式计数
};
```

#### 修复要点
1. 使用固定大小数组替代std::vector
2. 添加显式计数字段
3. 确保结构体使用#pragma pack(push,1)对齐
4. 避免任何动态内存分配

### 27.2 线程安全问题
#### 问题描述
多线程访问共享内存时出现数据竞争、读写冲突等问题。

#### 解决方案
```cpp
class ThreadSafeSharedMemory {
private:
    std::shared_mutex rw_mutex_;           // 读写锁
    std::atomic<uint32_t> write_sequence_; // 写序号
    std::atomic<bool> write_in_progress_;  // 写入标志
    
public:
    // 读操作（共享锁）
    template<typename T>
    bool ReadData(T& data, std::function<bool(const SharedMemoryBlock&, T&)> reader) {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        
        // 检查写入状态
        if (write_in_progress_.load()) {
            return false;
        }
        
        return reader(*shared_memory_, data);
    }
    
    // 写操作（独占锁）
    bool WriteData(std::function<bool(SharedMemoryBlock&)> writer) {
        std::unique_lock<std::shared_mutex> lock(rw_mutex_);
        
        // 标记写入开始
        write_in_progress_ = true;
        write_sequence_ |= 1; // 设置奇数
        
        try {
            bool success = writer(*shared_memory_);
            
            // 更新哈希
            UpdateHash();
            
            // 标记写入完成
            write_sequence_ += 1; // 设置偶数
            write_in_progress_ = false;
            
            return success;
        } catch (...) {
            // 写入失败，恢复状态
            write_in_progress_ = false;
            write_sequence_ += 1; // 恢复偶数
            throw;
        }
    }
    
private:
    void UpdateHash() {
        // 计算SHA256哈希
        shared_memory_->sharedmemHash = CalculateSHA256(
            reinterpret_cast<const uint8_t*>(shared_memory_),
            sizeof(SharedMemoryBlock) - sizeof(shared_memory_->sharedmemHash)
        );
    }
};
```

#### 修复要点
1. 使用读写锁分离读写操作
2. 原子操作管理写入状态
3. 奇偶序列确保数据一致性
4. 异常安全保证状态恢复

### 27.3 内存对齐问题
#### 问题描述
不同平台和编译器的内存对齐规则不一致，导致结构体大小和偏移不匹配。

#### 解决方案
```cpp
// 统一内存对齐策略
#pragma pack(push, 1)  // 1字节对齐

struct AlignedStruct {
    uint32_t value1;     // 4字节
    uint8_t value2;      // 1字节
    uint16_t value3;     // 2字节
    uint64_t value4;     // 8字节
    // 总大小：15字节（无填充）
};

#pragma pack(pop)

// 编译时检查
static_assert(sizeof(AlignedStruct) == 15, "Size mismatch");
static_assert(offsetof(AlignedStruct, value1) == 0, "Offset mismatch");
static_assert(offsetof(AlignedStruct, value2) == 4, "Offset mismatch");
static_assert(offsetof(AlignedStruct, value3) == 5, "Offset mismatch");
static_assert(offsetof(AlignedStruct, value4) == 7, "Offset mismatch");
```

### 27.4 跨平台类型问题
#### 问题描述
不同平台的基础类型大小不一致，导致数据解析错误。

#### 解决方案
```cpp
// 使用固定宽度类型
#include <cstdint>
#include <cstddef>

struct PortableStruct {
    int32_t  int32_value;    // 始终4字节
    uint64_t uint64_value;   // 始终8字节
    int16_t  int16_value;    // 始终2字节
    uint8_t  uint8_value;    // 始终1字节
    size_t   size_value;     // ❌ 跨平台不一致
    
    // 修复：使用固定宽度类型
    uint64_t size_fixed;     // ✅ 始终8字节
};

// 类型别名确保一致性
using timestamp_t = uint64_t;  // Unix时间戳（毫秒）
using temperature_t = int16_t;  // 温度*10（0.1°C）
using percentage_t = int16_t;   // 百分比*10
```

### 27.5 字符串处理问题
#### 问题描述
UTF-8编码、字符串长度限制、空字符处理等问题。

#### 解决方案
```cpp
class SafeStringHandler {
public:
    // 安全的字符串复制
    static bool SafeCopy(char* dest, size_t dest_size, const std::string& src) {
        if (!dest || dest_size == 0) return false;
        
        size_t copy_len = std::min(src.length(), dest_size - 1);
        std::memcpy(dest, src.c_str(), copy_len);
        dest[copy_len] = '\0';
        
        return src.length() < dest_size;
    }
    
    // UTF-8验证
    static bool IsValidUTF8(const std::string& str) {
        int bytes_needed = 0;
        for (unsigned char c : str) {
            if (bytes_needed == 0) {
                if ((c & 0x80) == 0) continue; // ASCII
                else if ((c & 0xE0) == 0xC0) bytes_needed = 1;
                else if ((c & 0xF0) == 0xE0) bytes_needed = 2;
                else if ((c & 0xF8) == 0xF0) bytes_needed = 3;
                else return false; // 无效UTF-8
            } else {
                if ((c & 0xC0) != 0x80) return false; // 无效后续字节
                bytes_needed--;
            }
        }
        return bytes_needed == 0;
    }
    
    // 字符串截断（UTF-8安全）
    static std::string TruncateUTF8(const std::string& str, size_t max_bytes) {
        if (str.length() <= max_bytes) return str;
        
        size_t truncate_pos = max_bytes;
        // 确保不在UTF-8多字节字符中间截断
        while (truncate_pos > 0 && (static_cast<unsigned char>(str[truncate_pos]) & 0x80) != 0) {
            truncate_pos--;
        }
        
        return str.substr(0, truncate_pos);
    }
};
```

### 27.6 错误处理改进
#### 原有问题
错误处理不一致，缺少详细的错误信息。

#### 改进方案
```cpp
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_ARGUMENT,
    PERMISSION_DENIED,
    RESOURCE_NOT_FOUND,
    DEVICE_NOT_SUPPORTED,
    TIMEOUT,
    INTERNAL_ERROR,
    PLATFORM_ERROR
};

class Error {
private:
    ErrorCode code_;
    std::string message_;
    std::string hint_;
    std::string detail_;
    
public:
    Error(ErrorCode code, const std::string& message, 
          const std::string& hint = "", const std::string& detail = "")
        : code_(code), message_(message), hint_(hint), detail_(detail) {}
    
    ErrorCode GetCode() const { return code_; }
    const std::string& GetMessage() const { return message_; }
    const std::string& GetHint() const { return hint_; }
    const std::string& GetDetail() const { return detail_; }
    
    bool IsSuccess() const { return code_ == ErrorCode::SUCCESS; }
    
    // JSON序列化
    nlohmann::json ToJson() const {
        nlohmann::json j;
        j["code"] = static_cast<int>(code_);
        j["message"] = message_;
        if (!hint_.empty()) j["hint"] = hint_;
        if (!detail_.empty()) j["detail"] = detail_;
        return j;
    }
};

// 结果类型（类似Rust的Result）
template<typename T>
class Result {
private:
    std::variant<T, Error> value_;
    
public:
    Result(T&& value) : value_(std::move(value)) {}
    Result(const Error& error) : value_(error) {}
    
    bool IsSuccess() const {
        return std::holds_alternative<T>(value_);
    }
    
    const T& GetValue() const {
        return std::get<T>(value_);
    }
    
    const Error& GetError() const {
        return std::get<Error>(value_);
    }
    
    // 链式操作
    template<typename F>
    auto Map(F&& func) -> Result<decltype(func(std::declval<T>()))> {
        if (IsSuccess()) {
            return Result<decltype(func(GetValue()))>(func(GetValue()));
        } else {
            return Result<decltype(func(GetValue()))>(GetError());
        }
    }
};
```

---

## 28. 未来跨平台抽象占位（Provider 接口）

### 28.1 接口扩展规划
```cpp
// 未来扩展的Provider接口
class IHardwareProviderV2 : public IHardwareProvider {
public:
    // 网络信息
    virtual std::vector<NetworkAdapter> GetNetworkAdapters() = 0;
    virtual NetworkStats GetNetworkStats(const std::string& interface) = 0;
    
    // 电源管理
    virtual BatteryInfo GetBatteryInfo() = 0;
    virtual PowerPlan GetPowerPlan() = 0;
    
    // 传感器扩展
    virtual std::vector<FanInfo> GetFans() = 0;
    virtual std::vector<Sensor> GetAllSensors() = 0;
    
    // 虚拟化检测
    virtual VirtualizationInfo GetVirtualizationInfo() = 0;
    
    // 安全信息
    virtual SecurityStatus GetSecurityStatus() = 0;
};
```

---

## 29. 收尾与后续触发点
实施阶段变更 → 先改文档再写代码。  
阶段完成写里程碑日志：`INFO milestone_reached phase=<n>`。  
进入编码需明确文字指令："要代码：<模块>"。

---

## 30. 附：偏移与结构校验章节（整块 SHA256 + diff 机制）
详细说明偏移校验流程、SHA256计算方法、结构变更diff机制，包括字段变更、版本升级、兼容性断点检测。

---

## 31. MCP 服务器构想（订阅过滤 + 限速行为）
MCP服务器支持客户端订阅过滤（channel[]、minIntervalMs），限速行为（RATE_LIMIT），事件通道推送，回放窗口管理。

---

## 32. 自检与扩展分析（sys.selftest / SMART 评分趋势 / sys.metrics / 性能优化 / LLM 预备 / critical 分类）
支持sys.selftest命令，输出critical与non-critical检查，SMART评分趋势分析，性能指标采集，预留LLM扩展接口。

### 32.14 SMART Aging Curve（老化曲线草图 + 配置化）
Aging Curve采样点条件、斜率公式、置信度分类（high/medium/low），参数配置化（agingWindow/agingThresholdScore），低置信度时暂停趋势追加。

---

## 33. 事件回放模拟（命名/保留策略）
快照文件命名：history/snapshot_YYYYMMDD_HH.json，事件文件：history/events_YYYYMMDD.jsonl，默认保留7天，最大回放窗口24h，超限拒绝回放。

---

## 34. 硬件扩展补充（字段与无效值约定）
列举所有扩展硬件字段、无效值约定（-1/空数组），采集来源待定字段标记，NVMe/内存/CPU特征/网卡采集方案。

---

## 35. CLI 设计与实施（含 degradeMode 行为 & DATA_NOT_READY 格式）
CLI命令矩阵，degradeMode触发退出码4，hashMismatch警告但不终止，数据不足统一输出DATA_NOT_READY，参数与退出码说明。

---

## 36. 单位与精度规范
温度0.1°C，CPU使用率*10，内存MB，计数类-1不可用，时间戳Unix ms。

---

## 37. 并发与线程安全模型
单写多读依赖writeSequence奇偶协议，自检遇奇数用上次快照，SMART异步独立线程，CLI只读映射。

---

## 38. 版本与兼容策略（abiVersion 映射）
abiVersion结构变化递增，格式0xMMMMmmpp，末尾字节可对应文档序号，salt字段引入需升级abiVersion。

---

## 39. 历史与保留策略（快照/事件/Aging 持久化）
快照每整点一次，事件日志每日分文件，Aging曲线点暂不持久化，后续可写disk_age_history.json。

---

## 40. 安全与权限初步（角色/RBAC/哈希链占位）
角色：READ_SYS/CONTROL/ADMIN/HIGH_RISK，危险操作需ADMIN+二次确认，日志哈希链prevHash计划。

---

## 41. 构建与工具脚本命名（偏移/自检/打包）
工具命名：generate_offsets.exe、verify_sharedmem.exe、selftest_dump.exe、release_pack.ps1。各工具输入输出与使用场景说明。

---

## 跨平台版本更新摘要

### 主要变更
1. **版本升级**：v0.14 → v0.15，abiVersion更新为0x00010015
2. **跨平台架构**：新增第38-40章，完整的跨平台设计和实现
3. **关键问题修复**：第27章详细总结所有已知问题的解决方案
4. **配置扩展**：core.json新增跨平台配置字段
5. **接口增强**：Provider接口支持平台能力检测和降级策略

### 新增特性
- 统一的跨平台硬件抽象层
- 平台特定的性能优化
- 完整的错误处理和结果类型系统
- 线程安全的共享内存访问
- 零拷贝数据传输优化
- UTF-8安全的字符串处理

### 兼容性保证
- 保持与v0.14的结构兼容性（仅尾部扩展）
- 降级模式确保旧版本客户端可用
- 配置向后兼容，新增字段有默认值
- 平台检测和自动降级机制

### 实施建议
1. 优先实现跨平台抽象层（第38章）
2. 按平台逐步实现Provider（第39章）
3. 应用性能优化（第40章）
4. 解决关键问题（第27章）
5. 完善测试覆盖率

---

## 下一步触发指令示例
- 要代码：跨平台Provider接口
- 要代码：Windows Provider实现
- 要代码：Linux Provider实现  
- 要代码：macOS Provider实现
- 要代码：共享内存线程安全
- 要代码：错误处理系统
- 要代码：CLI跨平台版本
- 要代码：构建系统配置

如需"精简版文档"或"差异版"也可直接说明。

（完）