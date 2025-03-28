#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    static void Initialize(const std::string& logPath = "system_monitor.log");
    static void Error(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message); // ��� Warning ����
    static void Log(const std::string& level, const std::string& message); // ��� Log ��������
	static void Close(); // ��� Close ��������

private:
    static std::ofstream logFile;
    static std::mutex logMutex; // ��� logMutex ����
};
