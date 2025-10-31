#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <mutex>
#include <fstream>
#include <chrono>
#include <initializer_list>
#include <windows.h>
#if __has_include(<format>)
  #include <format>
  #define LOGGER_HAS_STD_FORMAT 1
#else
  #define LOGGER_HAS_STD_FORMAT 0
#endif

// Severity levels
enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,
    LOG_CRITICAL = 5,
    LOG_FATAL = 6
};

// Console colors (Windows attributes)
enum class ConsoleColor : WORD {
    DEFAULT       = 7,
    TRACE_COLOR   = 13,  // magenta
    DEBUG_COLOR   = 9,   // blue
    INFO_COLOR    = 10,  // green
    WARN_COLOR    = 14,  // yellow
    ERROR_COLOR   = 12,  // light red
    CRITICAL_COLOR= 12,  // light red
    FATAL_COLOR   = 4    // dark red
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string category;   // component / module name
    std::string message;    // already UTF-8
    std::string file;       // source file
    int line = 0;           // source line
};

class Logger {
public:
    // Initialization (simple and with rotation)
    static bool Initialize(const std::string& filePath);
    static bool InitializeWithRotation(const std::string& baseFilePath, size_t maxFileSizeBytes, int maxFiles);

    // Configuration
    static void SetLogLevel(LogLevel level);
    static LogLevel GetLogLevel();
    static void EnableConsole(bool enable);
    static void SetRingBufferSize(size_t capacity); // in-memory recent logs for UI

    // Generic logging API
    static void Log(LogLevel level, std::string_view category, std::string_view message,
                    std::string_view file = {}, int line = 0);

    // Key/Value enriched logging
    static void LogKV(LogLevel level, std::string_view category, std::string_view message,
                      std::initializer_list<std::pair<std::string_view, std::string_view>> kv,
                      std::string_view file = {}, int line = 0);

#if LOGGER_HAS_STD_FORMAT
    template<typename... Args>
    static void Logf(LogLevel level, std::string_view category, std::string_view fmt,
                     std::string_view file, int line, Args&&... args) {
        if (level < currentLevel) return;
        try {
            std::string msg = std::vformat(fmt, std::make_format_args(args...));
            Log(level, category, msg, file, line);
        } catch (...) {
            Log(level, category, std::string("<Logf format error> ") + std::string(fmt), file, line);
        }
    }
#endif

    // Backward compatible helpers (category defaults to "default")
    static void Trace(const std::string& m); static void Debug(const std::string& m);
    static void Info(const std::string& m);  static void Warn(const std::string& m);
    static void Error(const std::string& m); static void Critical(const std::string& m);
    static void Fatal(const std::string& m);

    // Wide string helpers
    static void Trace(const std::wstring& m); static void Debug(const std::wstring& m);
    static void Info(const std::wstring& m);  static void Warn(const std::wstring& m);
    static void Error(const std::wstring& m); static void Critical(const std::wstring& m);
    static void Fatal(const std::wstring& m);

    // Retrieval for UI / diagnostics
    static std::vector<LogEntry> GetRecentEntries(); // snapshot copy
    static bool IsInitialized();

private:
    static std::mutex mutex;
    static std::ofstream stream;
    static bool consoleEnabled;
    static LogLevel currentLevel;
    static HANDLE consoleHandle;

    // Rotation
    static size_t maxFileSize;
    static int maxFileCount;
    static std::string basePath; // base file path without index

    // Ring buffer
    static std::vector<LogEntry> ring;
    static size_t ringCapacity;
    static size_t ringIndex;

    // Internal helpers
    static void WriteInternal(const LogEntry& entry);
    static void RotateIfNeeded();
    static std::string LevelToString(LogLevel l);
    static ConsoleColor LevelToColor(LogLevel l);
    static std::string NormalizeToUTF8(std::string_view input);
    static std::string WStringToUtf8(const std::wstring& w);
    static void PushRing(const LogEntry& e);
};

// Macros capturing source location & formatting
#if LOGGER_HAS_STD_FORMAT
#define LOG_T(cat, fmt, ...) Logger::Logf(LOG_TRACE,    cat, fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_D(cat, fmt, ...) Logger::Logf(LOG_DEBUG,    cat, fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_I(cat, fmt, ...) Logger::Logf(LOG_INFO,     cat, fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_W(cat, fmt, ...) Logger::Logf(LOG_WARNING,  cat, fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_E(cat, fmt, ...) Logger::Logf(LOG_ERROR,    cat, fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_C(cat, fmt, ...) Logger::Logf(LOG_CRITICAL, cat, fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_F(cat, fmt, ...) Logger::Logf(LOG_FATAL,    cat, fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_T(cat, msg) Logger::Log(LOG_TRACE,    cat, msg, __FILE__, __LINE__)
#define LOG_D(cat, msg) Logger::Log(LOG_DEBUG,    cat, msg, __FILE__, __LINE__)
#define LOG_I(cat, msg) Logger::Log(LOG_INFO,     cat, msg, __FILE__, __LINE__)
#define LOG_W(cat, msg) Logger::Log(LOG_WARNING,  cat, msg, __FILE__, __LINE__)
#define LOG_E(cat, msg) Logger::Log(LOG_ERROR,    cat, msg, __FILE__, __LINE__)
#define LOG_C(cat, msg) Logger::Log(LOG_CRITICAL, cat, msg, __FILE__, __LINE__)
#define LOG_F(cat, msg) Logger::Log(LOG_FATAL,    cat, msg, __FILE__, __LINE__)
#endif
