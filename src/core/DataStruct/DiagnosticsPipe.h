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

// Forward declaration of SharedMemoryBlock and accessor
struct SharedMemoryBlock;
SharedMemoryBlock* GetSharedMemoryBlockExtern();

// Define GET_BUFFER_PTR macro
#ifdef SHARED_MEMORY_MANAGER_INCLUDED
    #define GET_BUFFER_PTR SharedMemoryManager::GetBuffer()
#else
    #define GET_BUFFER_PTR GetSharedMemoryBlockExtern()
#endif

// Utility functions
inline std::string JsonEscape(const std::string& input) {
    std::string escaped;
    for (char c : input) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

inline bool WriteFrame(HANDLE hPipe, const std::string& data) {
    DWORD bytesWritten;
    return WriteFile(hPipe, data.c_str(), static_cast<DWORD>(data.length()), &bytesWritten, nullptr) && bytesWritten == data.length();
}

// Header-only diagnostics pipe implementation
namespace DiagnosticsPipe {
    static std::atomic<bool> g_run{false};
    static std::thread g_thread;
    static std::mutex g_logMutex;
    static std::queue<std::string> g_logQueue;

    inline void AppendLog(const std::string& line) {
        std::lock_guard<std::mutex> lk(g_logMutex);
        g_logQueue.push(line);
    }

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
        // offsets metadata
        constexpr int OFF_tempSensors = 36;
        constexpr int OFF_tempSensorCount = 1156;
        constexpr int OFF_smartDisks = 1158;
        constexpr int OFF_smartDiskCount = 1942;
        constexpr int OFF_futureReserved = 2429;
        constexpr int OFF_sharedmemHash = 2493;
        constexpr int OFF_usbDevices = 2525;
        constexpr int OFF_usbDeviceCount = 3093;
        constexpr int OFF_extensionPad = 3094;
        constexpr int expectedSize = 3212;

        ss << ",\"expectedSize\":" << expectedSize
            << ",\"offsets\":{"
            << "\"tempSensors\":" << OFF_tempSensors << ','
            << "\"tempSensorCount\":" << OFF_tempSensorCount << ','
            << "\"smartDisks\":" << OFF_smartDisks << ','
            << "\"smartDiskCount\":" << OFF_smartDiskCount << ','
            << "\"futureReserved\":" << OFF_futureReserved << ','
            << "\"sharedmemHash\":" << OFF_sharedmemHash << ','
            << "\"usbDevices\":" << OFF_usbDevices << ','
            << "\"usbDeviceCount\":" << OFF_usbDeviceCount << ','
            << "\"extensionPad\":" << OFF_extensionPad
            << '}';

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

//����ԭ�к�������
inline void StartDiagnosticsPipeThread() { DiagnosticsPipe::Start(); }
inline void StopDiagnosticsPipeThread() { DiagnosticsPipe::Stop(); }
inline void DiagnosticsPipeAppendLog(const std::string& line) { DiagnosticsPipe::AppendLog(line); }
