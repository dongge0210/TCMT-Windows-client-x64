#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>

/**
 * Diagnostics pipe snapshot structure
 * Must be kept in sync with the C# DiagnosticsPipeSnapshot
 */
struct DiagnosticsPipeSnapshot {
    uint32_t timestamp;
    uint32_t writeSequence;
    uint32_t abiVersion;
    
    struct {
        uint32_t tempSensors;
        uint32_t tempSensorCount;
        uint32_t smartDisks;
        uint32_t smartDiskCount;
        uint32_t futureReserved;
        uint32_t sharedmemHash;
        uint32_t extensionPad;
    } offsets;
    
    std::vector<std::string> logs;
};

/**
 * Diagnostics pipe client class
 * Responsible for connecting to the C++ backend diagnostics pipe and receiving real-time diagnostic data
 */
class DiagnosticsPipeClient {
private:
    HANDLE hPipe;
    std::atomic<bool> isRunning;
    std::atomic<bool> isConnected;
    std::thread workerThread;
    std::string lastError;
    
    static const std::string PIPE_NAME;
    
    using SnapshotCallback = std::function<void(const DiagnosticsPipeSnapshot&)>;
    SnapshotCallback onSnapshotReceived;
    
    using ErrorCallback = std::function<void(const std::string&)>;
    ErrorCallback onError;

public:
    /**
     * Constructor
     */
    DiagnosticsPipeClient();
    
    /**
     * Destructor
     */
    ~DiagnosticsPipeClient();
    
    /**
     * Start the diagnostics pipe client
     * @return true if successful, false otherwise
     */
    bool Start();
    
    /**
     * Stop the diagnostics pipe client
     */
    void Stop();
    
    /**
     * Check if connected
     * @return connection status
     */
    bool IsConnected() const;
    
    /**
     * Get the last error message
     * @return error message string
     */
    std::string GetLastErrorMessage() const;
    
    /**
     * Set the snapshot received callback
     * @param callback callback function
     */
    void SetSnapshotCallback(SnapshotCallback callback);
    
    /**
     * Set the error callback
     * @param callback callback function
     */
    void SetErrorCallback(ErrorCallback callback);

private:
    /**
     * Worker thread function
     */
    void WorkerThread();
    
    /**
     * Connect to the pipe
     * @return true if successful, false otherwise
     */
    bool ConnectToPipe();
    
    /**
     * Read data from the pipe
     * @return true if successful, false otherwise
     */
    bool ReadFromPipe();
    
    /**
     * Parse diagnostic data in JSON format
     * @param jsonData JSON data string
     * @param snapshot output snapshot structure
     * @return true if successful, false otherwise
     */
    bool ParseDiagnosticData(const std::string& jsonData, DiagnosticsPipeSnapshot& snapshot);
    
    /**
     * Parse log entries
     * @param logsArray log array JSON
     * @param snapshot output snapshot structure
     * @return true if successful, false otherwise
     */
    bool ParseLogEntries(const std::string& logsArray, DiagnosticsPipeSnapshot& snapshot);
    
    /**
     * Wait for the pipe to become available
     * @param timeoutMs timeout in milliseconds
     * @return true if successful, false if timeout
     */
    bool WaitForPipeAvailable(int timeoutMs = 5000);
    
    /**
     * Handle connection error
     * @param errorMessage error message
     */
    void HandleError(const std::string& errorMessage);
    
    /**
     * Cleanup resources
     */
    void Cleanup();
    
    /**
     * Simple JSON parsing helper function
     * @param json JSON string
     * @param key key name
     * @return value string
     */
    std::string ExtractJsonValue(const std::string& json, const std::string& key);
    
    /**
     * Extract JSON array
     * @param json JSON string
     * @param arrayKey array key name
     * @return array content string
     */
    std::string ExtractJsonArray(const std::string& json, const std::string& arrayKey);
    
    /**
     * Split string
     * @param str string to split
     * @param delimiter delimiter character
     * @return vector of split strings
     */
    std::vector<std::string> SplitString(const std::string& str, char delimiter);
};