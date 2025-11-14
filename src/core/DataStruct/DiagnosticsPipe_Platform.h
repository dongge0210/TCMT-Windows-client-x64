#pragma once

// Platform-specific diagnostics pipe implementation

#ifdef PLATFORM_WINDOWS
#include <windows.h>

// Windows-specific types
typedef unsigned char BYTE;

// Windows-specific functions
bool WriteFrame(HANDLE hPipe, const std::string& data) {
    DWORD bytesWritten;
    return WriteFile(hPipe, data.c_str(), static_cast<DWORD>(data.length()), &bytesWritten, nullptr) && bytesWritten == data.length();
}

std::string FallbackFormatWindowsErrorMessage(DWORD errorCode) {
    // Windows-specific error formatting
    return "Windows error code: " + std::to_string(errorCode);
}

#else
// Non-Windows platforms - placeholder implementations
#include <string>
#include <chrono>
#include <thread>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef int HANDLE;
const int INVALID_HANDLE_VALUE = -1;
const DWORD PIPE_ACCESS_OUTBOUND = 0;
const DWORD PIPE_TYPE_BYTE = 0;
const DWORD PIPE_READMODE_BYTE = 0;
const DWORD PIPE_WAIT = 0;
const DWORD _TRUNCATE = 0;

inline bool WriteFrame(HANDLE hPipe, const std::string& data) {
    // Placeholder for non-Windows platforms
    return false;
}

inline std::string FallbackFormatWindowsErrorMessage(DWORD errorCode) {
    return "Error code: " + std::to_string(errorCode) + " (non-Windows platform)";
}

inline void Sleep(DWORD milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

inline int sem_post(sem_t* semaphore) {
    return ::sem_post(semaphore);
}

inline int strncpy_s_impl(char* dest, size_t destSize, const char* src, size_t count) {
    if (dest && src && destSize > 0) {
        size_t copyCount = (count < destSize - 1) ? count : destSize - 1;
        strncpy(dest, src, copyCount);
        dest[copyCount] = '\0';
        return 0;
    }
    return -1;
}

#endif