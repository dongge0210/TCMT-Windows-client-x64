#include "MacErrorHandler.h"
#include <algorithm>
#include <thread>
#include <chrono>

#ifdef PLATFORM_MACOS

MacErrorHandler::MacErrorHandler() {
    m_errorHistory.reserve(MAX_ERROR_HISTORY);
}

void MacErrorHandler::ReportError(MacErrorCode code, const std::string& message, const std::string& context) {
    MacErrorInfo error;
    error.code = code;
    error.message = message;
    error.context = context;
    error.timestamp = std::chrono::steady_clock::now();
    error.retryCount = 0;
    
    AddToHistory(error);
    m_lastError = error;
}

void MacErrorHandler::ReportError(const std::string& message, const std::string& context) {
    ReportError(MacErrorCode::UNKNOWN_ERROR, message, context);
}

bool MacErrorHandler::HasError() const {
    return m_lastError.code != MacErrorCode::SUCCESS;
}

MacErrorInfo MacErrorHandler::GetLastError() const {
    return m_lastError;
}

std::vector<MacErrorInfo> MacErrorHandler::GetErrorHistory() const {
    return m_errorHistory;
}

void MacErrorHandler::ClearError() {
    m_lastError = MacErrorInfo();
}

void MacErrorHandler::ClearAllErrors() {
    m_errorHistory.clear();
    m_lastError = MacErrorInfo();
}

bool MacErrorHandler::AttemptRecovery() {
    if (m_recoveryCallback) {
        return m_recoveryCallback();
    }
    
    // 默认恢复策略
    switch (m_lastError.code) {
        case MacErrorCode::INITIALIZATION_FAILED:
            // 尝试重新初始化
            return false;
            
        case MacErrorCode::DATA_COLLECTION_FAILED:
            // 等待一段时间后重试
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return true;
            
        case MacErrorCode::PERMISSION_DENIED:
            // 权限错误无法自动恢复
            return false;
            
        case MacErrorCode::RESOURCE_UNAVAILABLE:
            // 等待资源可用
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return true;
            
        default:
            return false;
    }
}

void MacErrorHandler::SetRecoveryCallback(std::function<bool()> callback) {
    m_recoveryCallback = callback;
}

int MacErrorHandler::GetErrorCount(MacErrorCode code) const {
    return std::count_if(m_errorHistory.begin(), m_errorHistory.end(),
                           [code](const MacErrorInfo& error) { return error.code == code; });
}

double MacErrorHandler::GetErrorRate() const {
    if (m_errorHistory.empty()) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto oneHourAgo = now - std::chrono::hours(1);
    
    int recentErrors = std::count_if(m_errorHistory.begin(), m_errorHistory.end(),
                                   [oneHourAgo](const MacErrorInfo& error) {
                                       return error.timestamp > oneHourAgo;
                                   });
    
    return static_cast<double>(recentErrors) / m_errorHistory.size() * 100.0;
}

void MacErrorHandler::AddToHistory(const MacErrorInfo& error) {
    m_errorHistory.push_back(error);
    
    // 保持历史记录在限制范围内
    if (m_errorHistory.size() > MAX_ERROR_HISTORY) {
        m_errorHistory.erase(m_errorHistory.begin());
    }
}

bool MacErrorHandler::ShouldRetry(const MacErrorInfo& error, int maxRetries) const {
    return error.retryCount < maxRetries && 
           error.code != MacErrorCode::PERMISSION_DENIED &&
           error.code != MacErrorCode::INVALID_PARAMETER;
}

#endif // PLATFORM_MACOS