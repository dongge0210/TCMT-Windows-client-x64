# TCMT Windows Client 架构 / 协议 / 共享内存 / 功能规划总文档（全量更新版）
版本：v0.14  
更新时间：2025-10-21  
说明：本文件为“全量”文档。包含原始章节 1~27 + 扩展 28(含 28.14) / 29 / 30 / 31，并一次性补充全部缺失细节（结构字段、位标、哈希规范、刷新间隔、偏移生成、错误分类、并发/性能/版本策略等）。当前仅文档，不含任何实现代码。后续如需代码请显式指令：“要代码：XXX”。

---
## 目录
1. 目标与阶段路线总览  
2. 当前模块状态一览  
3. 共享内存结构与扩展策略（含完整结构声明 & 位标 & 哈希规范）  
4. 共享内存全量偏移表（理论 + 生成流程）  
5. 写入完整性：writeSequence 奇偶 + 报警机制  
6. 指令系统设计与实施步骤  
7. 指令清单（现有 / 预留 / 占位）  
8. 错误与日志规范（主日志 + errors.log + 环形对接 + 统一格式）  
9. 主板 / BIOS 信息采集规划  
10. 进程枚举（第一版无 CPU%）  
11. USB 枚举与去抖（debounce）  
12. 环形趋势缓冲（short/long + snapshotVersion 定义）  
13. 插件系统最小 SDK 约定（占位，低优先级）  
14. SMART 复检指令占位与异步策略（含正常刷新间隔）  
15. 进程控制与黑名单策略  
16. 配置文件 core.json 草案（新增 agingWindow / agingThresholdScore 等）  
17. 风险与回退策略  
18. 验收与测试步骤总表  
19. 原生温度采集（NVAPI / ADL / WMI / SMART 异步）  
20. 迁移步骤（旧结构→新结构 + 温度过渡）  
21. FAQ / 常见坑提示  
22. 实施批次任务分解（最新排序）  
23. 后续扩展路线图（分析 / 安全 / 跨平台）  
24. 未来跨平台抽象占位（Provider 接口）  
25. 收尾与后续触发点  
26. 附：偏移与结构校验章节（含整块 SHA256 + diff 机制）  
27. MCP 服务器构想（最低优先级 + 订阅过滤 + 限速行为）  
28. 自检与扩展分析（sys.selftest / SMART 评分趋势 / sys.metrics / 性能优化 / LLM 预备 / critical 分类）  
28.14 SMART Aging Curve（老化曲线草图 + 配置化）  
29. 事件回放模拟（命名/保留策略）  
30. 硬件扩展补充（字段与无效值约定）  
31. CLI 设计与实施（含 degradeMode 行为 & DATA_NOT_READY 格式）  
32. 单位与精度规范（新增）  
33. 并发与线程安全模型（新增）  
34. 版本与兼容策略（abiVersion 映射）  
35. 历史与保留策略（快照/事件/老化曲线持久化）  
36. 安全与权限初步（角色/RBAC/哈希链占位）  
37. 构建与工具脚本命名（偏移/自检/打包）  

---
## 3. 共享内存结构与扩展策略（补充细节）
### 3.1 结构演进准则
- 仅尾部追加，不在中间插入；移除字段需保留占位长度或 bump abiVersion + 标记弃用。  
- 所有字段必须为 POD，禁止 std::string、指针、动态分配。  
- 对齐强制 `#pragma pack(push,1)`。  

### 3.2 SharedMemoryBlock 完整声明（示意：最终以实际头文件为准）
```
struct TemperatureSensor {
  char name[32];          // UTF-8 传感器名称，不足填 0
  int16_t valueC_x10;     // 温度 *10，单位 0.1°C；不可用 -1
  uint8_t flags;          // bit0=valid, bit1=urgentLast, 其它保留
};

struct SmartDiskScore {
  char diskId[32];        // 物理盘标识
  int16_t score;          // 0-100，-1 表示不可用
  int32_t hoursOn;        // 通电小时数
  int16_t wearPercent;    // 0-100，-1 不可用
  uint16_t reallocated;   // 重映射扇区数
  uint16_t pending;       // 待映射扇区数
  uint16_t uncorrectable; // 不可恢复扇区
  int16_t temperatureC;   // 当前盘温度，-1 不可用
  uint8_t recentGrowthFlags; // bit0=reallocated增, bit1=wear突增
};

struct SharedMemoryBlock {
  // Header
  uint32_t abiVersion;            // 例如 0x00010014 -> 主版1 次版0 修订0 文档14
  uint32_t writeSequence;         // 奇偶协议；启动为 0（偶数表示初始稳定）
  uint32_t snapshotVersion;       // 每次完整硬件刷新成功后 +1
  uint32_t reservedHeader;        // 预留（对齐）

  // CPU / Memory 基础 (占位示意)
  uint16_t cpuLogicalCores;       // 逻辑核心数
  int16_t  cpuUsagePercent_x10;   // CPU 使用 *10；未实现 -1
  uint64_t memoryTotalMB;         // 总内存 MB
  uint64_t memoryUsedMB;          // 已用内存 MB（不含文件缓存）

  // Temperatures
  TemperatureSensor tempSensors[32]; // 32 传感器最大；不足填 name[0]=0
  uint16_t tempSensorCount;       // 实际有效传感器数量

  // SMART Scores
  SmartDiskScore smartDisks[16];  // 最多 16 物理盘
  uint8_t smartDiskCount;         // 有效盘个数

  // 主板 / BIOS
  char baseboardManufacturer[128];
  char baseboardProduct[64];
  char baseboardVersion[64];
  char baseboardSerial[64];       // 受 maskBaseboardSerial 影响
  char biosVendor[64];
  char biosVersion[64];
  char biosDate[32];
  uint8_t secureBootEnabled;      // 0/1
  uint8_t tpmPresent;             // 0/1
  uint16_t memorySlotsTotal;      // 插槽总数
  uint16_t memorySlotsUsed;       // 已用插槽数

  // Flags & Hash
  uint8_t futureReserved[64];     // bit0=degradeMode, bit1=hashMismatch, bit2=sequenceStallWarn(预留), 其它=0
  uint8_t sharedmemHash[32];      // SHA256(整块除 hash 自身)

  // 结尾扩展预留
  uint8_t extensionPad[128];      // 后续字段追加
};
```
说明：最终字段顺序与大小以编译头文件生成的 offsets JSON 为准。以上为文档参考结构。  

### 3.3 futureReserved 位标定义
| 位 | 名称 | 含义 | 触发条件 | 日志前缀 |
|----|------|------|----------|----------|
| bit0 | degradeMode | 结构/哈希/版本不匹配降级 | size/hash/version mismatch | sharedmem_degrade_mode |
| bit1 | hashMismatch | sha256 与偏移 JSON 不符（但 abiVersion 匹配） | 校验时发现差异 | sharedmem_hash_mismatch |
| bit2 | sequenceStallWarn | 连续奇数序列超过阈值触发一次 | writeSequence 奇数连续 N 次 | sequence_stall |
| bit3-7 | 保留 | 后续扩展 | - | - |

### 3.4 writeSequence 行为与溢出
- 32 位无符号递增；溢出后回到 0（仍视为偶数稳定）。  
- 读端只看奇偶，不做大小比较。  
- 初始值 0；写开始 → seq=1；写结束 → seq=2。  

### 3.5 sharedmemSha256 计算规范
- 范围：从结构起始地址到 `sizeof(SharedMemoryBlock)` 排除 `sharedmemHash` 32 字节自身（即 hash 之前先置 0 再算）。  
- 算法：SHA256，无盐（v0.14）。盐计划：v0.15 引入 16 字节随机 salt 字段并同步 abiVersion。  
- 生成时机：主程序启动完成结构初始化后计算一次；偏移 JSON 写入后不再重复。  
- CLI offsets verify 默认重新计算（可后续加 --fast 只比对缓存）。  

### 3.6 snapshotVersion 定义
- 由硬件刷新协调模块维护，每次完整刷新（至少一项：CPU 使用 / 温度集合 / SMART 属性 更新成功）后 +1。  
- 若单项刷新失败（如 SMART 暂不可用）但其他刷新成功仍 +1。  
- 趋势缓冲暂停条件：连续读取到相同 snapshotVersion ≥5 次 → 不追加新点（pause 标记）。  

### 3.7 并发与访问（结构级）
- 单写线程（硬件刷新） + 多读线程（UI / CLI）。  
- 写入流程：标记奇数 → 写全部字段 → 写 sharedmemHash → seq++偶数。  
- 自检线程遇到奇数：跳过当前新数据，使用上次稳定快照。  

### 3.8 结构升级流程强化
1. 尾部追加字段 → 调整 extensionPad 缩短或移除。  
2. 更新 abiVersion。  
3. 重生成 offsets JSON（含新哈希）。  
4. 更新文档结构声明与位标说明。  
5. CLI / 自检校验通过后发布。  

---
## 4. 共享内存全量偏移表（生成流程补充）
### 4.1 生成脚本/工具
- 工具名建议：`generate_offsets.exe`。  
- 输出：`docs/runtime/offsets_sharedmem_v0_14.json`。  
- 字段：名称 / offset / size / flags(valid/nullable/arrayLength)。  

### 4.2 示例（扩展）
```
{
 "abiVersion":"0x00010014",
 "sizeof":125818,
 "generatedTs":1697890000123,
 "sharedmemSha256":"<64hex>",
 "fields":[
   {"name":"abiVersion","offset":0,"size":4},
   {"name":"writeSequence","offset":4,"size":4},
   {"name":"snapshotVersion","offset":8,"size":4},
   {"name":"cpuLogicalCores","offset":16,"size":2},
   ...
 ]
}
```
### 4.3 校验优先级
version mismatch > size mismatch > hash mismatch > field offset mismatch。  
### 4.4 差异处理策略
- version mismatch：直接 degradeMode（bit0）。  
- size/hash mismatch：置 bit0 & bit1。  
- 字段偏移差异：记录 differences 列表 → 日志 WARN。  

---
## 14. SMART 异步正常刷新间隔补充
- 正常稳定周期：每 30 秒刷新 SMART 属性（可配置 smart.refreshIntervalSeconds）。  
- 温度仅：若盘温度单独可快速获取，可每 10 秒刷新（smart.tempIntervalSeconds）。  
- 重试 schedule（失败盘）：[1,5,10] 秒，超过 maxSmartRetries 标记该盘属性 stale。  

---
## 16. 配置文件 core.json 草案（新增字段）
新增：`smart.refreshIntervalSeconds`, `smart.tempIntervalSeconds`, `smart.agingWindow`, `smart.agingThresholdScore`。  
```
"smart": {
  "trendWindow": 5,
  "wearRapidGrowthPercent": 1,
  "refreshIntervalSeconds": 30,
  "tempIntervalSeconds": 10,
  "agingWindow": 10,
  "agingThresholdScore": 50
}
```

---
## 18. 自检检查分类补充
### 18.1 Critical Checks
- sharedmem_size / sharedmem_offsets / sharedmem_hash / write_sequence_flip / nvapi_init / adl_init。  
### 18.2 Non-Critical Checks
- smart_sample（单盘失败但其它成功） / baseboard_fields / trend_buffer_basic / 温度单传感器缺失。  
### 18.3 状态决策
- ≥1 critical fail → status=fail。  
- 仅 non-critical fail → status=partial_warn。  
- 全部通过 → status=ok。  

---
## 22. CLI degradeMode 行为补充
- writeSequence 奇数 ≠ degradeMode，仅提示“写入进行中”不改变退出码。  
- degradeMode(bit0)=true → sys-status 退出码=4。  
- hashMismatch(bit1)=true 但未触发 version mismatch → 同时显示警告行但仍退出码=4（合并处理）。  

---
## 28.14 SMART Aging Curve（配置化补充）
- trendWindow 使用 smart.agingWindow。  
- 阈值 projectedThresholdScore 使用 smart.agingThresholdScore。  
- 置信度：
  - high：点数≥agingWindow & |slope|≥0.002 & score 标准差<2  
  - medium：未满足 high 但点数≥3  
  - low：点数<3 或 |slope|<0.0005 或 标准差≥4  

---
## 29. 事件回放模拟补充：命名与保留策略
- 快照文件命名：`history/snapshot_YYYYMMDD_HH.json`（整点后首次稳定刷新写入）。  
- 事件文件：`history/events_YYYYMMDD.jsonl`。  
- 保留策略：默认保留 7 天（history.retentionDays=7），超期由后台清理任务删除。  
- 重建窗口最大跨度：24h；超出返回错误 RATE_LIMIT。  

---
## 30. 硬件扩展字段无效值约定
| 资源 | 字段 | 无效值 | 说明 |
|------|------|-------|-----|
| nvme | avgLatencyUs | -1 | 未采样或接口不支持 |
| nvme | percentUsed | -1 | 不可用（SATA） |
| memory.channels | avgFrequencyMHz | -1 | 读失败 |
| cpu.features | instructionSets | 空数组 | 未解析 | 
| gpu.driver | daysBehindEstimate | -1 | 未比对推荐版本 |
| net.adapters | rxRateMbps/txRateMbps | -1 | 未采样窗口不足 |  

---
## 31. CLI 设计补充：标准错误格式
- DATA_NOT_READY：`"error":"DATA_NOT_READY","detail":"SMART"` 或 `
