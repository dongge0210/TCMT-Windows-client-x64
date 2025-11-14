#include "DataUpdateManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>

DataUpdateManager::DataUpdateManager(int updateIntervalMs) 
    : isRunning(false), isConnected(false), consecutiveErrors(0),
      UPDATE_INTERVAL(updateIntervalMs), MAX_CONSECUTIVE_ERRORS(5) {
    
    // Create shared memory reader and diagnostic pipe client
    memoryReader = std::make_unique<SharedMemoryReader>();
    pipeClient = std::make_unique<DiagnosticsPipeClient>();
    
    // Set diagnostic pipe callback
    pipeClient->SetSnapshotCallback(
        [this](const DiagnosticsPipeSnapshot& snapshot) {
            HandleDiagnosticSnapshot(snapshot);
        }
    );
    
    pipeClient->SetErrorCallback(
        [this](const std::string& error) {
            HandleDiagnosticError(error);
        }
    );
}

DataUpdateManager::~DataUpdateManager() {
    Stop();
}

bool DataUpdateManager::Start() {
    if (isRunning) {
        return true;
    }
    
    lastError.clear();
    consecutiveErrors = 0;
    
    if (!InitializeConnections()) {
        lastError = "Failed to initialize connections";
        return false;
    }
    
    isRunning = true;
    
    // Start diagnostic pipe client
    if (!pipeClient->Start()) {
        lastError = "Failed to start diagnostic pipe client: " + pipeClient->GetLastErrorMessage();
        isRunning = false;
        return false;
    }
    
    // Perform initial data update
    UpdateData();
    
    std::cout << "Data update manager started successfully" << std::endl;
    return true;
}

void DataUpdateManager::Stop() {
    if (!isRunning) {
        return;
    }
    
    isRunning = false;
    
    // Stop diagnostic pipe client
    if (pipeClient) {
        pipeClient->Stop();
    }
    
    // Cleanup connections
    CleanupConnections();
    
    std::cout << "Data update manager stopped" << std::endl;
}

bool DataUpdateManager::UpdateData() {
    if (!memoryReader) {
        HandleError("Shared memory reader not initialized");
        return false;
    }
    
    try {
        // Read raw system info
        SystemInfo rawInfo;
        if (!memoryReader->ReadSystemInfo(rawInfo)) {
            HandleError("Failed to read shared memory data: " + memoryReader->GetLastErrorMessage());
            IncrementErrorCount();
            return false;
        }
        
        // Convert to CLI-friendly data structure
        CLISystemInfo newInfo;
        if (!SharedMemoryDataParser::ConvertToCLISystemInfo(rawInfo, newInfo)) {
            HandleError("Failed to convert system data");
            IncrementErrorCount();
            return false;
        }
        
        // Update current data
        currentData = newInfo;
        currentData.lastUpdateTime = GetCurrentTimestamp();
        
        // If previously disconnected, now connected successfully
        if (!isConnected) {
            UpdateConnectionStatus(true);
        }
        
        // Reset error count
        ResetErrorCount();
        
        // Trigger data update callback
        HandleDataUpdate();
        
        return true;
    }
    catch (const std::exception& e) {
        HandleError("Exception occurred while updating data: " + std::string(e.what()));
        IncrementErrorCount();
        return false;
    }
}

CLISystemInfo DataUpdateManager::GetCurrentData() const {
    return currentData;
}

bool DataUpdateManager::IsConnected() const {
    return isConnected;
}

std::string DataUpdateManager::GetLastErrorMessage() const {
    return lastError;
}

std::string DataUpdateManager::GetConnectionStatus() const {
    if (isConnected) {
        return "Connected";
    } else {
        return "Connection failed: " + lastError;
    }
}

std::string DataUpdateManager::GetDiagnosticInfo() const {
    std::ostringstream oss;
    
    if (memoryReader && memoryReader->IsConnected()) {
        oss << memoryReader->GetDiagnosticsInfo();
    } else {
        oss << "Shared memory not connected\n";
    }

    if (pipeClient && pipeClient->IsConnected()) {
        oss << "Diagnostic pipe: Connected\n";
    } else {
        oss << "Diagnostic pipe: Not connected\n";
    }

    oss << "Consecutive error count: " << consecutiveErrors.load() << "/" << MAX_CONSECUTIVE_ERRORS << "\n";
    oss << "Last update time: " << currentData.lastUpdateTime << "\n";
    return oss.str();
}

std::vector<std::string> DataUpdateManager::GetRecentDiagnosticLogs() const {
    return recentDiagnosticLogs;
}

void DataUpdateManager::SetDataUpdateCallback(DataUpdateCallback callback) {
    onDataUpdate = callback;
}

void DataUpdateManager::SetConnectionStatusCallback(ConnectionStatusCallback callback) {
    onConnectionStatusChange = callback;
}

void DataUpdateManager::SetErrorCallback(ErrorCallback callback) {
    onError = callback;
}

bool DataUpdateManager::TryReconnect() {
    std::cout << "Attempting to reconnect..." << std::endl;
    
    // Reset error count
    ResetErrorCount();
    
    // Cleanup existing connections
    CleanupConnections();
    
    // Reinitialize connections
    if (InitializeConnections()) {
    // attempt to update data
        if (UpdateData()) {
            std::cout << "Reconnection successful" << std::endl;
            return true;
        }
    }
    
    std::cout << "Reconnection failed: " << lastError << std::endl;
    return false;
}

void DataUpdateManager::HandleDataUpdate() {
    if (onDataUpdate) {
        onDataUpdate(currentData);
    }
}

void DataUpdateManager::HandleConnectionStatusChange(bool connected, const std::string& status) {
    if (onConnectionStatusChange) {
        onConnectionStatusChange(connected, status);
    }
}

void DataUpdateManager::HandleError(const std::string& error) {
    lastError = error;
    
    if (onError) {
        onError(error);
    }
    
    // Check if should disconnect
    if (ShouldDisconnect()) {
        UpdateConnectionStatus(false);
    }
}

void DataUpdateManager::HandleDiagnosticSnapshot(const DiagnosticsPipeSnapshot& snapshot) {
    try {
        // Update diagnostic info
        std::ostringstream oss;
        oss << "Diagnostic snapshot - Timestamp: " << snapshot.timestamp 
            << ", Write sequence: " << snapshot.writeSequence 
            << ", ABI version: 0x" << std::hex << snapshot.abiVersion;
        
        lastDiagnosticInfo = oss.str();
        
        // Process log entries
        for (const auto& log : snapshot.logs) {
            AddDiagnosticLog(log);
        }
        
        // Cleanup old logs
        CleanupOldDiagnosticLogs();
    }
    catch (const std::exception& e) {
        HandleError("Exception occurred while processing diagnostic snapshot: " + std::string(e.what()));
    }
}

void DataUpdateManager::HandleDiagnosticError(const std::string& error) {
    HandleError("Diagnostic pipe error: " + error);
}

bool DataUpdateManager::InitializeConnections() {
    // Initialize shared memory connection
    if (!memoryReader->Initialize()) {
        lastError = "Shared memory initialization failed: " + memoryReader->GetLastErrorMessage();
        return false;
    }
    
    // Validate shared memory layout
    if (!memoryReader->ValidateLayout()) {
        lastError = "Shared memory layout validation failed: " + memoryReader->GetLastErrorMessage();
        return false;
    }
    
    return true;
}

void DataUpdateManager::CleanupConnections() {
    if (memoryReader) {
        memoryReader->Cleanup();
    }
}

void DataUpdateManager::UpdateConnectionStatus(bool connected) {
    if (isConnected != connected) {
        isConnected = connected;
        
        std::string status = connected ? "Connected" : ("Connection lost: " + lastError);
        HandleConnectionStatusChange(connected, status);
        
        std::cout << "Connection status changed: " << status << std::endl;
    }
}

void DataUpdateManager::IncrementErrorCount() {
    int currentErrors = consecutiveErrors.fetch_add(1) + 1;
    
    std::cout << "Error count increased: " << currentErrors << "/" << MAX_CONSECUTIVE_ERRORS << std::endl;
}

void DataUpdateManager::ResetErrorCount() {
    consecutiveErrors.store(0);
}

bool DataUpdateManager::ShouldDisconnect() const {
    return consecutiveErrors.load() >= MAX_CONSECUTIVE_ERRORS;
}

void DataUpdateManager::AddDiagnosticLog(const std::string& log) {
    // Add timestamp
    std::string timestampedLog = "[" + GetCurrentTimestamp() + "] " + log;
    
    recentDiagnosticLogs.push_back(timestampedLog);
    
    // Limit log count
    const size_t MAX_LOGS = 100;
    if (recentDiagnosticLogs.size() > MAX_LOGS) {
        recentDiagnosticLogs.erase(recentDiagnosticLogs.begin(), 
                                 recentDiagnosticLogs.begin() + (recentDiagnosticLogs.size() - MAX_LOGS));
    }
}

void DataUpdateManager::CleanupOldDiagnosticLogs() {
    // Keep recent 50 logs
    const size_t MAX_LOGS = 50;
    if (recentDiagnosticLogs.size() > MAX_LOGS) {
        recentDiagnosticLogs.erase(recentDiagnosticLogs.begin(), 
                                 recentDiagnosticLogs.begin() + (recentDiagnosticLogs.size() - MAX_LOGS));
    }
}

std::string DataUpdateManager::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    char buf[32];
    struct tm tm_info;
#ifdef PLATFORM_WINDOWS
    localtime_s(&tm_info, &time_t);
#else
    localtime_r(&time_t, &tm_info);
#endif
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm_info);
    oss << buf;
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), ".%03lld", static_cast<long long>(ms.count()));
#else
    snprintf(buf, sizeof(buf), ".%03lld", static_cast<long long>(ms.count()));
#endif
    oss << buf;
    return oss.str();
}