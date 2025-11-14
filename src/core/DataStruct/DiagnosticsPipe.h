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
#include <cstring>
#include "DiagnosticsPipe_Platform.h"

#ifdef PLATFORM_MACOS
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

// Platform-specific function declarations
#ifdef PLATFORM_WINDOWS
inline bool WriteFrame(HANDLE hPipe, const std::string& data);
inline std::string FallbackFormatWindowsErrorMessage(DWORD errorCode);
#else
// Non-Windows declarations already in DiagnosticsPipe_Platform.h
#endif

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

// Platform-specific WriteFrame implementation moved to DiagnosticsPipe_Platform.h

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
#ifdef PLATFORM_WINDOWS
            HANDLE hPipe = CreateNamedPipeA("\\\\.\\pipe\\SysMonDiag", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 4096, 4096, 0, nullptr);
            if (hPipe == INVALID_HANDLE_VALUE) {
                Sleep(2000);
                continue;
            }
            if (!ConnectNamedPipe(hPipe, nullptr)) {
                CloseHandle(hPipe);
                Sleep(2000);
                continue;
            }
            // Create a separate thread for reading client commands
            std::thread readThread([hPipe]() {
                char buffer[1024];
                while (g_run) {
                    DWORD bytesRead;
                    if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
                        if (bytesRead > 0) {
                            buffer[bytesRead] = '\0';
                            // Process client command here
                            // For now, just log the received command
                            Logger::Info("收到客户端命令: " + std::string(buffer));
                        }
                    } else {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
            
            while (g_run) {
                auto json = BuildJson();
                if (!WriteFrame(hPipe, json)) break;
                Sleep(1000);
            }
            
            if (readThread.joinable()) {
                g_run = false;  // Signal read thread to exit
                readThread.join();
            }
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
#elif defined(PLATFORM_MACOS)
            // macOS implementation using Unix domain socket
            const char* socketPath = "/tmp/tcmt_diag.sock";
            
            // Remove existing socket file
            unlink(socketPath);
            
            // Create socket
            int serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
            if (serverSocket == -1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Set up socket address
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);
            
            // Bind socket
            if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                close(serverSocket);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Listen for connections
            if (listen(serverSocket, 1) == -1) {
                close(serverSocket);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Accept connection
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == -1) {
                close(serverSocket);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Send data while running
            while (g_run) {
                auto json = BuildJson();
                ssize_t sent = send(clientSocket, json.c_str(), json.length(), 0);
                if (sent != (ssize_t)json.length()) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            
            close(clientSocket);
            close(serverSocket);
#elif defined(PLATFORM_LINUX)
            // Linux implementation using Unix domain socket
            const char* socketPath = "/tmp/tcmt_diag.sock";
            
            // Remove existing socket file
            unlink(socketPath);
            
            // Create socket
            int serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
            if (serverSocket == -1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Set up socket address
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);
            
            // Bind socket
            if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                close(serverSocket);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Listen for connections
            if (listen(serverSocket, 1) == -1) {
                close(serverSocket);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Accept connection
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == -1) {
                close(serverSocket);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            
            // Create a separate thread for reading client commands
            std::thread readThread([clientSocket]() {
                char buffer[1024];
                while (g_run) {
                    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        // Process client command here
                        // For now, just log the received command
                        Logger::Info("收到客户端命令: " + std::string(buffer));
                    } else if (bytesRead == 0) {
                        // Client disconnected
                        break;
                    } else {
                        // Error occurred
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
            
            // Send data while running
            while (g_run) {
                auto json = BuildJson();
                ssize_t sent = send(clientSocket, json.c_str(), json.length(), MSG_NOSIGNAL);
                if (sent != (ssize_t)json.length()) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            
            if (readThread.joinable()) {
                g_run = false;  // Signal read thread to exit
                readThread.join();
            }
            
            close(clientSocket);
            close(serverSocket);
#endif
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
