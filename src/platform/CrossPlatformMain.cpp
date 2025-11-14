// Cross-platform main program for TCMT system monitor
#include "../core/common/PlatformDefs.h"
#include "../core/common/CrossPlatformStubs.h"
#include "../core/DataStruct/SharedMemoryManager.h"
#include "../core/DataStruct/DiagnosticsPipe.h"
#include "../core/Utils/Logger.h"
#include "CrossPlatformHardwareInfo.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <atomic>
#include <exception>
#include <vector>
#include <string>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <sys/select.h>
#endif

// Global flag for graceful shutdown
std::atomic<bool> g_running(true);

// Signal handler for graceful shutdown
void SignalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    g_running = false;
}

// Cross-platform keyboard input check
bool CheckForKeyPress() {
#ifdef PLATFORM_WINDOWS
    return _kbhit() != 0;
#else
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
#endif
}

// Cross-platform keyboard input get
char GetKeyPress() {
#ifdef PLATFORM_WINDOWS
    return _getch();
#else
    char buf = 0;
    if (read(STDIN_FILENO, &buf, 1) > 0) {
        return buf;
    }
    return 0;
#endif
}

// Set non-blocking console mode
void SetNonBlockingMode(bool enable) {
#ifndef PLATFORM_WINDOWS
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    
    if (enable) {
        ttystate.c_lflag &= ~(ICANON | ECHO);
        ttystate.c_cc[VMIN] = 1;
    } else {
        ttystate.c_lflag |= ICANON | ECHO;
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
#endif
}

// Cross-platform system information collector
class CrossPlatformSystemCollector {
private:
    bool initialized;
    std::string lastError;
    std::unique_ptr<CrossPlatformHardwareInfo> hardwareInfo;
    
public:
    CrossPlatformSystemCollector() : initialized(false) {}
    
    bool Initialize() {
        try {
            // Initialize logger
            if (!Logger::Initialize("tcmt_monitor.log", false)) {
                lastError = "Failed to initialize logger";
                return false;
            }
            
            Logger::SetLogLevel(LL_INFO);
            Logger::Info("Cross-platform system monitor starting...");
            
            // Initialize hardware info collector
            hardwareInfo = std::make_unique<CrossPlatformHardwareInfo>();
            if (!hardwareInfo->Initialize()) {
                lastError = "Failed to initialize hardware info collector: " + hardwareInfo->GetLastError();
                return false;
            }
            
            // Initialize shared memory
            if (!SharedMemoryManager::InitSharedMemory()) {
                lastError = "Failed to initialize shared memory: " + SharedMemoryManager::GetLastError();
                return false;
            }
            
            // Diagnostics pipe not needed for basic functionality
            
            initialized = true;
            Logger::Info("System collector initialized successfully");
            return true;
        }
        catch (const std::exception& e) {
            lastError = "Initialization exception: " + std::string(e.what());
            return false;
        }
    }
    
    bool CollectSystemInfo() {
        if (!initialized) {
            lastError = "Collector not initialized";
            return false;
        }
        
        try {
            SystemInfo sysInfo;
            
            // Collect hardware information using the cross-platform collector
            if (!hardwareInfo->CollectSystemInfo(sysInfo)) {
                lastError = "Failed to collect system info: " + hardwareInfo->GetLastError();
                Logger::Error(lastError);
                return false;
            }
            
            // Write to shared memory
            SharedMemoryManager::WriteToSharedMemory(sysInfo);
            
            return true;
        }
        catch (const std::exception& e) {
            lastError = "Collection exception: " + std::string(e.what());
            Logger::Error(lastError);
            return false;
        }
    }
    
    void Cleanup() {
        if (initialized) {
            Logger::Info("Cleaning up system collector...");
            // DiagnosticsPipe cleanup not needed
            SharedMemoryManager::CleanupSharedMemory();
            
            if (hardwareInfo) {
                hardwareInfo->Cleanup();
                hardwareInfo.reset();
            }
            
            Logger::Shutdown();
            initialized = false;
        }
    }
    
    std::string GetLastError() const { return lastError; }
    bool IsInitialized() const { return initialized; }
};

// Main function
int main(int argc, char* argv[]) {
    std::cout << "TCMT Cross-Platform System Monitor" << std::endl;
    std::cout << "Press 'q' to quit, 's' to show status" << std::endl;
    
    // Set up signal handlers
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
#ifndef PLATFORM_WINDOWS
    SetNonBlockingMode(true);
#endif
    
    CrossPlatformSystemCollector collector;
    
    // Initialize the collector
    if (!collector.Initialize()) {
        std::cerr << "Failed to initialize system collector: " << collector.GetLastError() << std::endl;
        return 1;
    }
    
    // Main loop
    const int UPDATE_INTERVAL_MS = 1000; // 1 second
    int loopCount = 0;
    
    while (g_running) {
        auto startTime = std::chrono::steady_clock::now();
        
        // Collect and update system information
        if (!collector.CollectSystemInfo()) {
            std::cerr << "Error collecting system info: " << collector.GetLastError() << std::endl;
        }
        
        // Check for keyboard input
        if (CheckForKeyPress()) {
            char key = GetKeyPress();
            switch (key) {
                case 'q':
                case 'Q':
                    std::cout << "Quit requested by user" << std::endl;
                    g_running = false;
                    break;
                case 's':
                case 'S':
                    std::cout << "Status: Running (loop " << loopCount << ")" << std::endl;
                    break;
                default:
                    // Ignore other keys
                    break;
            }
        }
        
        loopCount++;
        
        // Calculate sleep time to maintain consistent update interval
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        auto sleepTime = std::chrono::milliseconds(UPDATE_INTERVAL_MS) - elapsed;
        
        if (sleepTime.count() > 0) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
    
    // Cleanup
    collector.Cleanup();
    
#ifndef PLATFORM_WINDOWS
    SetNonBlockingMode(false);
#endif
    
    std::cout << "TCMT System Monitor stopped" << std::endl;
    return 0;
}