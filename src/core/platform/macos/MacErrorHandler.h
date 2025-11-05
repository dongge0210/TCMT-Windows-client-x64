#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <functional>
#include <thread>

#ifdef PLATFORM_MACOS

// 错误代码枚举
enum class MacErrorCode {
    SUCCESS = 0,
    INITIALIZATION_FAILED,
    DATA_COLLECTION_FAILED,
    PERMISSION_DENIED,
    RESOURCE_UNAVAILABLE,
    TIMEOUT,
    INVALID_PARAMETER,
    UNKNOWN_ERROR
};

// 错误信息结构
struct MacErrorInfo {
    MacErrorCode code;
    std::string message;
    std::string context;
    std::chrono::steady_clock::time_point timestamp;
    int retryCount;
    
    MacErrorInfo() : code(MacErrorCode::SUCCESS), retryCount(0) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// 错误处理器类
class MacErrorHandler {
public:
    MacErrorHandler();
    virtual ~MacErrorHandler() = default;
    
    // 错误报告
    void ReportError(MacErrorCode code, const std::string& message, const std::string& context = "");
    void ReportError(const std::string& message, const std::string& context = "");
    
    // 错误查询
    bool HasError() const;
    MacErrorInfo GetLastError() const;
    std::vector<MacErrorInfo> GetErrorHistory() const;
    
    // 错误处理
    void ClearError();
    void ClearAllErrors();
    
    // 重试机制
    template<typename T>
    bool ExecuteWithRetry(T& result, std::function<bool()> operation, int maxRetries = 3);
    
    // 恢复策略
    bool AttemptRecovery();
    void SetRecoveryCallback(std::function<bool()> callback);
    
    // 错误统计
    int GetErrorCount(MacErrorCode code) const;
    double GetErrorRate() const;
    
private:
    void AddToHistory(const MacErrorInfo& error);
    bool ShouldRetry(const MacErrorInfo& error, int maxRetries) const;
    
    std::vector<MacErrorInfo> m_errorHistory;
    MacErrorInfo m_lastError;
    std::function<bool()> m_recoveryCallback;
    static const size_t MAX_ERROR_HISTORY = 100;
};

// 模板方法实现
template<typename T>
bool MacErrorHandler::ExecuteWithRetry(T& result, std::function<bool()> operation, int maxRetries) {
    for (int attempt = 0; attempt <= maxRetries; ++attempt) {
        try {
            if (operation()) {
                return true;
            }
        } catch (...) {
            if (attempt == maxRetries) {
                ReportError(MacErrorCode::UNKNOWN_ERROR, "Operation failed after retries", "ExecuteWithRetry");
                return false;
            }
            // 短暂延迟后重试
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (attempt + 1)));
        }
    }
    return false;
}

#endif // PLATFORM_MACOS