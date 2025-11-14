#include "Logger.h"
#include <codecvt>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <filesystem>

std::mutex Logger::mutex;
std::ofstream Logger::stream;
bool Logger::consoleEnabled = true;
LogLevel Logger::currentLevel = LL_DEBUG;

#ifdef PLATFORM_WINDOWS
HANDLE Logger::consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#elif defined(PLATFORM_MACOS)
bool Logger::useSyslog = false;
FILE* Logger::consoleHandle = stdout;
#else
bool Logger::useSyslog = false;
FILE* Logger::consoleHandle = stdout;
#endif
size_t Logger::maxFileSize = 0;
int Logger::maxFileCount = 0;
std::string Logger::basePath;
std::vector<LogEntry> Logger::ring;
size_t Logger::ringCapacity = 0;
size_t Logger::ringIndex = 0;

static bool IsValidUTF8(const std::string& s) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(s.data());
    size_t len = s.size();
    for (size_t i = 0; i < len; ) {
        unsigned char c = bytes[i];
        size_t seq = 0;
        if (c <= 0x7F) seq = 1;
        else if ((c >> 5) == 0x6) seq = 2;
        else if ((c >> 4) == 0xE) seq = 3;
        else if ((c >> 3) == 0x1E) seq = 4;
        else return false;
        if (i + seq > len) return false;
        for (size_t j = 1; j < seq; ++j) if ((bytes[i + j] & 0xC0) != 0x80) return false;
        i += seq;
    }
    return true;
}

std::string Logger::NormalizeToUTF8(std::string_view input) {
    if (input.empty()) return std::string();
    std::string s(input);
    if (IsValidUTF8(s)) return s;
    
#ifdef PLATFORM_WINDOWS
    int wideLen = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), nullptr, 0);
    if (wideLen <= 0) return s;
    std::wstring wide(wideLen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), wide.data(), wideLen);
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0) return s;
    std::string out(utf8Len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), out.data(), utf8Len, nullptr, nullptr);
    return out;
#else
    // For non-Windows platforms, assume input is already UTF-8
    return s;
#endif
}

std::string Logger::WStringToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    
#ifdef PLATFORM_WINDOWS
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), len, nullptr, nullptr);
    return s;
#else
    // For non-Windows platforms, use std::codecvt or similar conversion
    std::mbstate_t state = std::mbstate_t();
    const wchar_t* src = w.c_str();
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (len == static_cast<size_t>(-1)) return {};
    std::string s(len, '\0');
    std::wcsrtombs(&s[0], &src, len, &state);
    return s;
#endif
}

bool Logger::Initialize(const std::string& filePath) {
    return Initialize(filePath, false);
}

bool Logger::Initialize(const std::string& filePath, bool enableSyslog) {
    std::lock_guard<std::mutex> lock(mutex);
    
#ifdef PLATFORM_MACOS
    useSyslog = enableSyslog;
    if (useSyslog) {
        openlog("TCMT", LOG_PID | LOG_CONS, LOG_USER);
    }
#endif
    
    stream.open(filePath, std::ios::binary | std::ios::app);
    if (!stream.is_open()) return false;
    if (stream.tellp() == 0) {
        const unsigned char bom[] = {0xEF,0xBB,0xBF};
        stream.write(reinterpret_cast<const char*>(bom), sizeof(bom));
    }
    basePath = filePath;
    return true;
}

bool Logger::InitializeWithRotation(const std::string& baseFilePath, size_t maxFileSizeBytes, int maxFiles) {
    if (!Initialize(baseFilePath, false)) return false;
    maxFileSize = maxFileSizeBytes;
    maxFileCount = maxFiles;
    return true;
}

void Logger::SetLogLevel(LogLevel level) { currentLevel = level; }
LogLevel Logger::GetLogLevel() { return currentLevel; }
void Logger::EnableConsole(bool enable) { consoleEnabled = enable; }
bool Logger::IsInitialized() { return stream.is_open(); }
void Logger::SetRingBufferSize(size_t capacity) {
    std::lock_guard<std::mutex> lock(mutex);
    ringCapacity = capacity;
    ring.clear();
    ringIndex = 0;
}

std::string Logger::LevelToString(LogLevel l) {
    switch(l){
        case LL_TRACE: return "TRACE"; case LL_DEBUG: return "DEBUG"; case LL_INFO: return "INFO";
        case LL_WARNING: return "WARN"; case LL_ERROR: return "ERROR"; case LL_CRITICAL: return "CRITICAL"; case LL_FATAL: return "FATAL";
    }
    return "UNKNOWN";
}

ConsoleColor Logger::LevelToColor(LogLevel l) {
    switch(l){
        case LL_TRACE: return ConsoleColor::TRACE_COLOR;
        case LL_DEBUG: return ConsoleColor::DEBUG_COLOR;
        case LL_INFO: return ConsoleColor::INFO_COLOR;
        case LL_WARNING: return ConsoleColor::WARN_COLOR;
        case LL_ERROR: return ConsoleColor::ERROR_COLOR;
        case LL_CRITICAL: return ConsoleColor::CRITICAL_COLOR;
        case LL_FATAL: return ConsoleColor::FATAL_COLOR;
    }
    return ConsoleColor::DEFAULT;
}

void Logger::RotateIfNeeded() {
    if (!maxFileSize || !stream.is_open()) return;
    auto size = stream.tellp();
    if (size < 0 || static_cast<size_t>(size) < maxFileSize) return;
    stream.close();
    // Shift old files
    for (int i = maxFileCount - 1; i >= 0; --i) {
        std::filesystem::path old = basePath + (i == 0 ? std::string() : ("." + std::to_string(i)));
        if (std::filesystem::exists(old)) {
            std::filesystem::path target = basePath + "." + std::to_string(i+1);
            if (i + 1 > maxFileCount) {
                std::error_code ec; std::filesystem::remove(old, ec);
            } else {
                std::error_code ec; std::filesystem::rename(old, target, ec);
            }
        }
    }
    stream.open(basePath, std::ios::binary | std::ios::trunc);
    const unsigned char bom[] = {0xEF,0xBB,0xBF};
    stream.write(reinterpret_cast<const char*>(bom), sizeof(bom));
}

void Logger::PushRing(const LogEntry& e) {
    if (!ringCapacity) return;
    if (ring.size() < ringCapacity) {
        ring.push_back(e);
    } else {
        ring[ringIndex] = e;
        ringIndex = (ringIndex + 1) % ringCapacity;
    }
}

void Logger::WriteInternal(const LogEntry& entry) {
    if (!stream.is_open()) return;
    RotateIfNeeded();
    auto tt = std::chrono::system_clock::to_time_t(entry.timestamp);
    std::tm tm{};
    
#ifdef PLATFORM_WINDOWS
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    
    std::ostringstream oss;
    oss << '[' << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "]";
    oss << '[' << LevelToString(entry.level) << "]";
    if (!entry.category.empty()) oss << '[' << entry.category << "]";
    if (!entry.file.empty()) oss << '[' << entry.file << ':' << entry.line << "]";
    oss << ' ' << entry.message << '\n';
    std::string line = oss.str();
    stream.write(line.c_str(), line.size());
    stream.flush();

    PushRing(entry);

    if (consoleEnabled && consoleHandle) {
#ifdef PLATFORM_WINDOWS
        SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(LevelToColor(entry.level)));
        // Convert UTF-8 to wide for proper output
        int wlen = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.size(), nullptr, 0);
        if (wlen > 0) {
            std::wstring wbuf(wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.size(), wbuf.data(), wlen);
            DWORD written; WriteConsoleW(consoleHandle, wbuf.c_str(), (DWORD)wlen, &written, NULL);
        }
        SetConsoleTextAttribute(consoleHandle, (WORD)ConsoleColor::DEFAULT);
#elif defined(PLATFORM_MACOS)
        // macOS: Use ANSI escape codes for colors and syslog if enabled
        if (useSyslog) {
            int priority = LOG_INFO;
            switch (entry.level) {
                case LL_TRACE: priority = LOG_DEBUG; break;
                case LL_DEBUG: priority = LOG_DEBUG; break;
                case LL_INFO: priority = LOG_INFO; break;
                case LL_WARNING: priority = LOG_WARNING; break;
                case LL_ERROR: priority = LOG_ERR; break;
                case LL_CRITICAL: priority = LOG_ERR; break;
                case LL_FATAL: priority = LOG_CRIT; break;
            }
            syslog(priority, "%s", line.c_str());
        } else {
            // Use ANSI color codes
            const char* colorCode = "\033[0m"; // Reset
            switch (LevelToColor(entry.level)) {
                case ConsoleColor::TRACE_COLOR: colorCode = "\033[35m"; break; // Magenta
                case ConsoleColor::DEBUG_COLOR: colorCode = "\033[34m"; break; // Blue
                case ConsoleColor::INFO_COLOR: colorCode = "\033[32m"; break;  // Green
                case ConsoleColor::WARN_COLOR: colorCode = "\033[33m"; break;  // Yellow
                case ConsoleColor::ERROR_COLOR: colorCode = "\033[91m"; break; // Light Red
                case ConsoleColor::CRITICAL_COLOR: colorCode = "\033[35m"; break; // Purple
                case ConsoleColor::FATAL_COLOR: colorCode = "\033[31m"; break; // Dark Red
                default: colorCode = "\033[0m"; break;
            }
            fprintf(consoleHandle, "%s%s\033[0m", colorCode, line.c_str());
            fflush(consoleHandle);
        }
#else
        // Other platforms: simple output
        fprintf(consoleHandle, "%s", line.c_str());
        fflush(consoleHandle);
#endif
    }
}

void Logger::Log(LogLevel level, std::string_view category, std::string_view message,
                 std::string_view file, int line) {
    if (level < currentLevel) return;
    LogEntry e;
    e.timestamp = std::chrono::system_clock::now();
    e.level = level;
    e.category = std::string(category);
    e.message = NormalizeToUTF8(message);
    e.file = std::string(file);
    e.line = line;
    std::lock_guard<std::mutex> lock(mutex);
    WriteInternal(e);
}

void Logger::LogKV(LogLevel level, std::string_view category, std::string_view message,
                   std::initializer_list<std::pair<std::string_view, std::string_view>> kv,
                   std::string_view file, int line) {
    if (level < currentLevel) return;
    std::ostringstream extra;
    extra << message;
    for (auto& p : kv) {
        extra << ' ' << p.first << '=' << p.second;
    }
    Log(level, category, extra.str(), file, line);
}

std::vector<LogEntry> Logger::GetRecentEntries() {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<LogEntry> out;
    if (!ringCapacity || ring.empty()) return out;
    // If full, return in chronological order
    if (ring.size() == ringCapacity) {
        for (size_t i = ringIndex; i < ring.size(); ++i) out.push_back(ring[i]);
        for (size_t i = 0; i < ringIndex; ++i) out.push_back(ring[i]);
    } else {
        out = ring; // not yet wrapped
    }
    return out;
}

// Backward compatibility simple wrappers (category="default")
#define SIMPLE_LOG_WRAPPER(fnName, lvl) \
    void Logger::fnName(const std::string& m){ Log(lvl, "default", m, {}, 0);} \
    void Logger::fnName(const std::wstring& m){ Log(lvl, "default", WStringToUtf8(m), {}, 0);} 

SIMPLE_LOG_WRAPPER(Trace, LL_TRACE)
SIMPLE_LOG_WRAPPER(Debug, LL_DEBUG)
SIMPLE_LOG_WRAPPER(Info, LL_INFO)
SIMPLE_LOG_WRAPPER(Warn, LL_WARNING)
SIMPLE_LOG_WRAPPER(Error, LL_ERROR)
SIMPLE_LOG_WRAPPER(Critical, LL_CRITICAL)
SIMPLE_LOG_WRAPPER(Fatal, LL_FATAL)

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex);
    
#ifdef PLATFORM_MACOS
    if (useSyslog) {
        closelog();
        useSyslog = false;
    }
#endif
    
    if (stream.is_open()) {
        stream.close();
    }
}

