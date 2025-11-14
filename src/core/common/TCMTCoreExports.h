#pragma once

#include <cstddef> // For size_t

#ifdef TCMT_CORE_DYNAMIC_EXPORTS
    #ifdef _WIN32
        #ifdef TCMT_CORE_BUILDING_DLL
            #define TCMT_CORE_API __declspec(dllexport)
        #else
            #define TCMT_CORE_API __declspec(dllimport)
        #endif
    #else
        #define TCMT_CORE_API
    #endif
#else
    #define TCMT_CORE_API
#endif

// 导出函数声明
TCMT_CORE_API bool TCMT_InitializeHardwareMonitor();
TCMT_CORE_API void TCMT_CleanupHardwareMonitor();
TCMT_CORE_API bool TCMT_CollectSystemInfo(struct SystemInfo* info);
TCMT_CORE_API const char* TCMT_GetLastError();
TCMT_CORE_API bool TCMT_IsInitialized();
TCMT_CORE_API bool TCMT_InitializeSharedMemory();
TCMT_CORE_API void TCMT_CleanupSharedMemory();
TCMT_CORE_API bool TCMT_WriteToSharedMemory(const struct SystemInfo& info);
TCMT_CORE_API bool TCMT_InitializeDiagnosticsPipe();
TCMT_CORE_API void TCMT_CleanupDiagnosticsPipe();
TCMT_CORE_API bool TCMT_WriteDiagnosticsFrame(const char* data, size_t size);