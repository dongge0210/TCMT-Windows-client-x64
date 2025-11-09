#pragma once

#include "SharedMemoryReader.h"
#include "DiagnosticsPipeClient.h"
#include "CLISystemInfo.h"
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>

/**
 * 数据更新管理器类
 * 负责协调共享内存读取、诊断管道监听和数据更新
 */
class DataUpdateManager {
private:
    std::unique_ptr<SharedMemoryReader> memoryReader;
    std::unique_ptr<DiagnosticsPipeClient> pipeClient;
    
    std::atomic<bool> isRunning;
    std::atomic<bool> isConnected;
    std::atomic<int> consecutiveErrors;
    
    CLISystemInfo currentData;
    std::string lastError;
    
    const std::chrono::milliseconds UPDATE_INTERVAL;
    const int MAX_CONSECUTIVE_ERRORS;
    
    using DataUpdateCallback = std::function<void(const CLISystemInfo&)>;
    using ConnectionStatusCallback = std::function<void(bool connected, const std::string& status)>;
    using ErrorCallback = std::function<void(const std::string& error)>;
    
    DataUpdateCallback onDataUpdate;
    ConnectionStatusCallback onConnectionStatusChange;
    ErrorCallback onError;
    
    std::string lastDiagnosticInfo;
    std::vector<std::string> recentDiagnosticLogs;

public:
    /**
     * 构造函数
     * @param updateIntervalMs 更新间隔（毫秒）
     */
    explicit DataUpdateManager(int updateIntervalMs = 500);
    
    /**
     * 析构函数
     */
    ~DataUpdateManager();
    
    /**
     * 启动数据更新管理器
     * @return 成功返回true，失败返回false
     */
    bool Start();
    
    /**
     * 停止数据更新管理器
     */
    void Stop();
    
    /**
     * 手动更新数据
     * @return 成功返回true，失败返回false
     */
    bool UpdateData();
    
    /**
     * 获取当前系统信息
     * @return 当前系统信息的副本
     */
    CLISystemInfo GetCurrentData() const;
    
    /**
     * 检查是否已连接
     * @return 连接状态
     */
    bool IsConnected() const;
    
    /**
     * 获取最后一次错误信息
     * @return 错误信息字符串
     */
    std::string GetLastErrorMessage() const;
    
    /**
     * 获取连接状态描述
     * @return 连接状态字符串
     */
    std::string GetConnectionStatus() const;
    
    /**
     * 获取诊断信息
     * @return 诊断信息字符串
     */
    std::string GetDiagnosticInfo() const;
    
    /**
     * 获取最近的诊断日志
     * @return 诊断日志数组
     */
    std::vector<std::string> GetRecentDiagnosticLogs() const;
    
    /**
     * 设置数据更新回调函数
     * @param callback 回调函数
     */
    void SetDataUpdateCallback(DataUpdateCallback callback);
    
    /**
     * 设置连接状态变化回调函数
     * @param callback 回调函数
     */
    void SetConnectionStatusCallback(ConnectionStatusCallback callback);
    
    /**
     * 设置错误回调函数
     * @param callback 回调函数
     */
    void SetErrorCallback(ErrorCallback callback);
    
    /**
     * 尝试重新连接
     * @return 成功返回true，失败返回false
     */
    bool TryReconnect();

private:
    /**
     * 处理数据更新
     */
    void HandleDataUpdate();
    
    /**
     * 处理连接状态变化
     * @param connected 连接状态
     * @param status 状态描述
     */
    void HandleConnectionStatusChange(bool connected, const std::string& status);
    
    /**
     * 处理错误
     * @param error 错误信息
     */
    void HandleError(const std::string& error);
    
    /**
     * 处理诊断管道快照
     * @param snapshot 诊断快照
     */
    void HandleDiagnosticSnapshot(const DiagnosticsPipeSnapshot& snapshot);
    
    /**
     * 处理诊断管道错误
     * @param error 错误信息
     */
    void HandleDiagnosticError(const std::string& error);
    
    /**
     * 初始化连接
     * @return 成功返回true，失败返回false
     */
    bool InitializeConnections();
    
    /**
     * 清理连接
     */
    void CleanupConnections();
    
    /**
     * 更新连接状态
     * @param connected 新的连接状态
     */
    void UpdateConnectionStatus(bool connected);
    
    /**
     * 增加错误计数
     */
    void IncrementErrorCount();
    
    /**
     * 重置错误计数
     */
    void ResetErrorCount();
    
    /**
     * 检查是否应该断开连接
     * @return 应该断开返回true
     */
    bool ShouldDisconnect() const;
    
    /**
     * 添加诊断日志
     * @param log 日志内容
     */
    void AddDiagnosticLog(const std::string& log);
    
    /**
     * 清理旧的诊断日志
     */
    void CleanupOldDiagnosticLogs();
    
    /**
     * 获取当前时间戳
     * @return 时间戳字符串
     */
    std::string GetCurrentTimestamp();
};