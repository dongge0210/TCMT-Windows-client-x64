#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <algorithm> // Added for std::transform
#include <windows.h> // For console color support
#include "WinUtils.h" // for UTF-16/UTF-8 conversion helpers

// 日志等级枚举
enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,
    LOG_CRITICAL = 5,
    LOG_FATAL = 6
};

// 控制台颜色枚举
enum class ConsoleColor {
    BLACK = 0,           // 黑色
    DARK_BLUE = 1,       // 深蓝色
    DARK_GREEN = 2,      // 深绿色
    DARK_CYAN = 3,       // 深青色
    DARK_RED = 4,        // 深红色
    DARK_MAGENTA = 5,    // 深洋红色
    DARK_YELLOW = 6,     // 深黄色
    LIGHT_GRAY = 7,      // 浅灰色
    DARK_GRAY = 8,       // 深灰色
    LIGHT_BLUE = 9,      // 亮蓝色
    LIGHT_GREEN = 10,    // 亮绿色
    LIGHT_CYAN = 11,     // 亮青色
    LIGHT_RED = 12,      // 亮红色
    LIGHT_MAGENTA = 13,  // 亮洋红色
    YELLOW = 14,         // 黄色
    WHITE = 15,          // 白色
    
    // 特殊用途别名
    PURPLE = 13,         // 紫色 - TRACE & DEBUG (使用亮洋红色)
    GREEN = 10,          // 绿色 - INFO
    ORANGE = 12,         // 橙色 - ERROR (使用亮红色)
    RED = 12             // 红色 - CRITICAL
};

class Logger {
private:
    static std::ofstream logFile;
    static std::mutex logMutex;
    static bool consoleOutputEnabled; // Flag for console output
    static LogLevel currentLogLevel; // 当前日志等级过滤器
    static HANDLE hConsole; // 控制台句柄
    static void WriteLog(const std::string& level, const std::string& message, LogLevel msgLevel, ConsoleColor color);
    static std::wstring ConvertToWideString(const std::string& utf8Str); // Helper for UTF-8 to wide string conversion
    static void SetConsoleColor(ConsoleColor color); // 设置控制台颜色
    static void ResetConsoleColor(); // 重置控制台颜色
    // 将可能为 ANSI/本地代码页 的字符串规范化为 UTF-8 编码
    static std::string NormalizeToUTF8(const std::string& input);

public:
    static void Initialize(const std::string& logFilePath);
    static void EnableConsoleOutput(bool enable); // Method to enable/disable console output
    static void SetLogLevel(LogLevel level); // 设置日志等级过滤器
    static LogLevel GetLogLevel(); // 获取当前日志等级
    static bool IsInitialized(); // 检查Logger是否已初始化
    
    // Log level methods (ordered by severity: Trace < Debug < Info < Warning < Error < Critical < Fatal)
    static void Trace(const std::string& message);   // 最详细的信息，通常只在调试时使用 (白色)
    static void Debug(const std::string& message);   // 调试信息 (绿色)
    static void Info(const std::string& message);    // 一般信息 (绿色)
    static void Warn(const std::string& message);    // 警告信息 (黄色)
    static void Error(const std::string& message);   // 错误信息 (橙色)
    static void Critical(const std::string& message); // 严重错误 (红色)
    static void Fatal(const std::string& message);   // 致命错误 (深红色)

    // 便捷重载：直接传 wide 字符串（用于从 WinAPI/WMI 返回的 UTF-16 字符串）
    static void Trace(const std::wstring& message) { Trace(WinUtils::WstringToUtf8(message)); }
    static void Debug(const std::wstring& message) { Debug(WinUtils::WstringToUtf8(message)); }
    static void Info(const std::wstring& message)  { Info(WinUtils::WstringToUtf8(message)); }
    static void Warn(const std::wstring& message)  { Warn(WinUtils::WstringToUtf8(message)); }
    static void Error(const std::wstring& message) { Error(WinUtils::WstringToUtf8(message)); }
    static void Critical(const std::wstring& message) { Critical(WinUtils::WstringToUtf8(message)); }
    static void Fatal(const std::wstring& message) { Fatal(WinUtils::WstringToUtf8(message)); }
};
