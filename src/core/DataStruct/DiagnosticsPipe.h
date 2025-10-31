#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <windows.h>

// Header-only diagnostics pipe implementation
namespace DiagnosticsPipe {
    static std::atomic<bool> g_run{false};
    static std::thread g_thread;
    static std::mutex g_logMutex;
    static std::queue<std::string> g_logQueue;

    inline std::string JsonEscape(const std::string& in) {
        std::ostringstream o;
        o << '"';
        for (char c : in) {
            switch (c) {
                case '"': o << "\\\""; break;
                case '\\': o << "\\\\"; break;
                case '\n': o << "\\n"; break;
                case '\r': o << "\\r"; break;
                case '\t': o << "\\t"; break;
                default: if ((unsigned char)c < 0x20) { o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c << std::dec; } else o << c; break;
            }
        }
        o << '"'; return o.str();
    }

    inline void AppendLog(const std::string& line) {
        std::lock_guard<std::mutex> lk(g_logMutex);
        if (g_logQueue.size() > 200) g_logQueue.pop();
        g_logQueue.push(line);
    }

    inline bool WriteFrame(HANDLE hPipe, const std::string& json) {
        uint32_t len = (uint32_t)json.size();
        DWORD written = 0;
        if (!WriteFile(hPipe, &len, 4, &written, nullptr) || written != 4) return false;
        if (!WriteFile(hPipe, json.data(), len, &written, nullptr) || written != len) return false;
        return true;
    }

    // Forward declaration of SharedMemoryBlock and accessor
    struct SharedMemoryBlock; // real definition在其他头
    SharedMemoryBlock* GetSharedMemoryBlockExtern(); //由调用方提供包装函数或直接替换逻辑

    // 若已包含 SharedMemoryManager 可直接使用其静态方法
    #ifdef SHARED_MEMORY_MANAGER_INCLUDED
        #define GET_BUFFER_PTR SharedMemoryManager::GetBuffer()
    #else
        #define GET_BUFFER_PTR GetSharedMemoryBlockExtern()
    #endif

    inline std::string BuildJson() {
        auto* buf = GET_BUFFER_PTR;
        std::ostringstream ss;
        ss << "{";
        uint64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        ss << "\"timestamp\":" << ts;
        if (buf) {
            ss << ",\"abiVersion\":" << buf->abiVersion
               << ",\"writeSequence\":" << buf->writeSequence
               << ",\"snapshotVersion\":" << buf->snapshotVersion
               << ",\"cpuLogicalCores\":" << buf->cpuLogicalCores
               << ",\"memoryTotalMB\":" << buf->memoryTotalMB
               << ",\"memoryUsedMB\":" << buf->memoryUsedMB;
        }
        int OFF_tempSensors = 36;
        int OFF_tempSensorCount = 1156;
        int OFF_smartDisks = 1158;
        int OFF_smartDiskCount = 1942;
        int OFF_futureReserved = 2429;
        int OFF_sharedmemHash = 2493;
        int OFF_extensionPad = 2525;
        int expectedSize = 2653;
        ss << ",\"expectedSize\":" << expectedSize
           << ",\"offsets\":{\"tempSensors\":" << OFF_tempSensors
           << ",\"tempSensorCount\":" << OFF_tempSensorCount
           << ",\"smartDisks\":" << OFF_smartDisks
           << ",\"smartDiskCount\":" << OFF_smartDiskCount
           << ",\"futureReserved\":" << OFF_futureReserved
           << ",\"sharedmemHash\":" << OFF_sharedmemHash
           << ",\"extensionPad\":" << OFF_extensionPad << "}";
        std::vector<std::string> logs;
        {
            std::lock_guard<std::mutex> lk(g_logMutex);
            while (!g_logQueue.empty()) {
                logs.push_back(g_logQueue.front());
                g_logQueue.pop();
            }
        }
        ss << ",\"logs\":[";
        for (size_t i = 0; i < logs.size(); ++i) {
            if (i) ss << ',';
            ss << JsonEscape(logs[i]);
        }
        ss << "]}";
        return ss.str();
    }

    inline void ThreadFunc() {
        while (g_run) {
            HANDLE hPipe = CreateNamedPipeA("\\\\.\\pipe\\SysMonDiag", PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 4096, 4096, 0, nullptr);
            if (hPipe == INVALID_HANDLE_VALUE) {
                Sleep(2000);
                continue;
            }
            if (!ConnectNamedPipe(hPipe, nullptr)) {
                CloseHandle(hPipe);
                Sleep(2000);
                continue;
            }
            while (g_run) {
                auto json = BuildJson();
                if (!WriteFrame(hPipe, json)) break;
                Sleep(1000);
            }
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
        }
    }

    inline void Start() {
        if (g_run) return;
        g_run = true;
        g_thread = std::thread(ThreadFunc);
    }
    inline void Stop() {
        g_run = false;
        if (g_thread.joinable()) g_thread.join();
    }
}

//兼容原有函数名称
inline void StartDiagnosticsPipeThread() { DiagnosticsPipe::Start(); }
inline void StopDiagnosticsPipeThread() { DiagnosticsPipe::Stop(); }
inline void DiagnosticsPipeAppendLog(const std::string& line) { DiagnosticsPipe::AppendLog(line); }
