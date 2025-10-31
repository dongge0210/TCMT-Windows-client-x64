#include "Logger.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <filesystem>
#include <codecvt>

std::mutex Logger::mutex;
std::ofstream Logger::stream;
bool Logger::consoleEnabled = true;
LogLevel Logger::currentLevel = LOG_DEBUG;
HANDLE Logger::consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
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
    int wideLen = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), nullptr, 0);
    if (wideLen <= 0) return s;
    std::wstring wide(wideLen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), wide.data(), wideLen);
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0) return s;
    std::string out(utf8Len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), out.data(), utf8Len, nullptr, nullptr);
    return out;
}

std::string Logger::WStringToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), len, nullptr, nullptr);
    return s;
}

bool Logger::Initialize(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex);
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
    if (!Initialize(baseFilePath)) return false;
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
        case LOG_TRACE: return "TRACE"; case LOG_DEBUG: return "DEBUG"; case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARN"; case LOG_ERROR: return "ERROR"; case LOG_CRITICAL: return "CRITICAL"; case LOG_FATAL: return "FATAL";
    }
    return "UNKNOWN";
}

ConsoleColor Logger::LevelToColor(LogLevel l) {
    switch(l){
        case LOG_TRACE: return ConsoleColor::TRACE_COLOR;
        case LOG_DEBUG: return ConsoleColor::DEBUG_COLOR;
        case LOG_INFO: return ConsoleColor::INFO_COLOR;
        case LOG_WARNING: return ConsoleColor::WARN_COLOR;
        case LOG_ERROR: return ConsoleColor::ERROR_COLOR;
        case LOG_CRITICAL: return ConsoleColor::CRITICAL_COLOR;
        case LOG_FATAL: return ConsoleColor::FATAL_COLOR;
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
    std::tm tm{}; localtime_s(&tm, &tt);
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

    if (consoleEnabled && consoleHandle != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(LevelToColor(entry.level)));
        // Convert UTF-8 to wide for proper output
        int wlen = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.size(), nullptr, 0);
        if (wlen > 0) {
            std::wstring wbuf(wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.size(), wbuf.data(), wlen);
            DWORD written; WriteConsoleW(consoleHandle, wbuf.c_str(), (DWORD)wlen, &written, NULL);
        }
        SetConsoleTextAttribute(consoleHandle, (WORD)ConsoleColor::DEFAULT);
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

SIMPLE_LOG_WRAPPER(Trace, LOG_TRACE)
SIMPLE_LOG_WRAPPER(Debug, LOG_DEBUG)
SIMPLE_LOG_WRAPPER(Info, LOG_INFO)
SIMPLE_LOG_WRAPPER(Warn, LOG_WARNING)
SIMPLE_LOG_WRAPPER(Error, LOG_ERROR)
SIMPLE_LOG_WRAPPER(Critical, LOG_CRITICAL)
SIMPLE_LOG_WRAPPER(Fatal, LOG_FATAL)

