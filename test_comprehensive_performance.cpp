#include "src/core/platform/macos/MacCpuInfo.h"
#include "src/core/platform/macos/MacMemoryInfo.h"
#include "src/core/platform/macos/MacGpuInfo.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

class PerformanceMonitor {
public:
    PerformanceMonitor() {
        m_cpuInfo = std::make_unique<MacCpuInfo>();
        m_memoryInfo = std::make_unique<MacMemoryInfo>();
        m_gpuInfo = std::make_unique<MacGpuInfo>();
        
        m_initialized = Initialize();
    }
    
    bool IsInitialized() const { return m_initialized; }
    
    void RunContinuousTest(int durationSeconds = 60) {
        if (!m_initialized) {
            std::cout << "❌ 初始化失败，无法运行测试" << std::endl;
            return;
        }
        
        std::cout << "🚀 开始综合性能测试 (运行 " << durationSeconds << " 秒)" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        auto startTime = std::chrono::steady_clock::now();
        int iteration = 0;
        
        while (std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count() < durationSeconds) {
            
            iteration++;
            std::cout << "\n📊 第 " << iteration << " 次采样" << std::endl;
            std::cout << std::string(50, '-') << std::endl;
            
            // CPU 性能监控
            if (m_cpuInfo && m_cpuInfo->Update()) {
                std::cout << "💻 CPU 信息:" << std::endl;
                std::cout << "   使用率: " << std::fixed << std::setprecision(1) << m_cpuInfo->GetTotalUsage() << "%" << std::endl;
                std::cout << "   当前频率: " << std::fixed << std::setprecision(0) << m_cpuInfo->GetCurrentFrequency() << " MHz" << std::endl;
                std::cout << "   温度: " << std::fixed << std::setprecision(1) << m_cpuInfo->GetTemperature() << "°C" << std::endl;
                std::cout << "   功耗: " << std::fixed << std::setprecision(1) << m_cpuInfo->GetPowerUsage() << "W" << std::endl;
                
                // 核心详情
                std::cout << "   核心数: " << m_cpuInfo->GetTotalCores() 
                          << " (性能:" << m_cpuInfo->GetPerformanceCores() 
                          << ", 效率:" << m_cpuInfo->GetEfficiencyCores() << ")" << std::endl;
            }
            
            // 内存性能监控
            if (m_memoryInfo && m_memoryInfo->Update()) {
                std::cout << "🧠 内存信息:" << std::endl;
                std::cout << "   总内存: " << std::fixed << std::setprecision(2) 
                          << m_memoryInfo->GetTotalPhysicalMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
                std::cout << "   可用内存: " << std::fixed << std::setprecision(2) 
                          << m_memoryInfo->GetAvailablePhysicalMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
                std::cout << "   使用率: " << std::fixed << std::setprecision(1) 
                          << m_memoryInfo->GetPhysicalMemoryUsage() << "%" << std::endl;
                std::cout << "   内存压力: " << std::fixed << std::setprecision(1) 
                          << m_memoryInfo->GetMemoryPressure() << "%" << std::endl;
                std::cout << "   状态: " << m_memoryInfo->GetMemoryStatusDescription() << std::endl;
                
                // 缓存详情
                std::cout << "   缓存内存: " << std::fixed << std::setprecision(2) 
                          << m_memoryInfo->GetCachedMemory() / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
                std::cout << "   内存效率: " << std::fixed << std::setprecision(1) 
                          << m_memoryInfo->GetMemoryEfficiency() << "%" << std::endl;
            }
            
            // GPU性能监控
            if (m_gpuInfo && m_gpuInfo->Update()) {
                std::cout << "🎮 GPU 信息:" << std::endl;
                std::cout << "   GPU名称: " << m_gpuInfo->GetName() << std::endl;
                std::cout << "   供应商: " << m_gpuInfo->GetVendor() << std::endl;
                std::cout << "   架构: " << m_gpuInfo->GetArchitecture() << std::endl;
                std::cout << "   GPU使用率: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetGpuUsage() * 100 << "%" << std::endl;
                std::cout << "   内存使用率: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetMemoryUsage() * 100 << "%" << std::endl;
                std::cout << "   当前频率: " << std::fixed << std::setprecision(0) << m_gpuInfo->GetCurrentFrequency() << " MHz" << std::endl;
                std::cout << "   温度: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetTemperature() << "°C" << std::endl;
                std::cout << "   功耗: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetPowerUsage() << "W" << std::endl;
                std::cout << "   风扇转速: " << std::fixed << std::setprecision(0) << m_gpuInfo->GetFanSpeed() << " RPM" << std::endl;
                
                // 性能指标
                std::cout << "   计算单元: " << m_gpuInfo->GetComputeUnits() << std::endl;
                std::cout << "   性能评级: " << std::fixed << std::setprecision(1) << m_gpuInfo->GetPerformanceRating() << "%" << std::endl;
            }
            
            // 等待一段时间
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
        std::cout << "\n✅ 综合性能测试完成!" << std::endl;
        GeneratePerformanceReport();
    }
    
    void GeneratePerformanceReport() {
        std::cout << "\n📋 性能报告" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        // 系统健康状态检查
        bool cpuHealthy = m_cpuInfo && m_cpuInfo->GetTemperature() < 80.0;
        bool memoryHealthy = m_memoryInfo && m_memoryInfo->GetPhysicalMemoryUsage() < 90.0;
        bool gpuHealthy = m_gpuInfo && m_gpuInfo->GetTemperature() < 80.0;
        
        std::cout << "🏥️ 系统健康状态:" << std::endl;
        std::cout << "   CPU: " << (cpuHealthy ? "✅ 健康" : "⚠️  需要关注") << std::endl;
        std::cout << "   内存: " << (memoryHealthy ? "✅ 健康" : "⚠️  需要关注") << std::endl;
        std::cout << "   GPU: " << (gpuHealthy ? "✅ 健康" : "⚠️  需要关注") << std::endl;
        
        // 性能建议
        std::cout << "\n💡 性能建议:" << std::endl;
        if (!cpuHealthy) {
            std::cout << "   - CPU温度过高，建议检查散热系统" << std::endl;
        }
        if (!memoryHealthy) {
            std::cout << "   - 内存使用率较高，建议关闭不必要的应用程序" << std::endl;
        }
        if (!gpuHealthy) {
            std::cout << "   - GPU温度过高，建议降低图形负载" << std::endl;
        }
        
        // 跨平台架构优势
        std::cout << "\n🌟 跨平台架构优势:" << std::endl;
        std::cout << "   ✓ 统一的接口设计" << std::endl;
        std::cout << "   ✓ 平台特定的实现" << std::endl;
        std::cout << "   ✓ 实时性能监控" << std::endl;
        std::cout << "   ✓ 智能错误处理" << std::endl;
        std::cout << "   ✓ 自动恢复机制" << std::endl;
    }
    
private:
    bool Initialize() {
        bool success = true;
        
        if (m_cpuInfo && !m_cpuInfo->Initialize()) {
            std::cout << "❌ CPU监控初始化失败" << std::endl;
            success = false;
        }
        
        if (m_memoryInfo && !m_memoryInfo->Initialize()) {
            std::cout << "❌ 内存监控初始化失败" << std::endl;
            success = false;
        }
        
        if (m_gpuInfo && !m_gpuInfo->Initialize()) {
            std::cout << "❌ GPU监控初始化失败" << std::endl;
            success = false;
        }
        
        return success;
    }
    
    std::unique_ptr<MacCpuInfo> m_cpuInfo;
    std::unique_ptr<MacMemoryInfo> m_memoryInfo;
    std::unique_ptr<MacGpuInfo> m_gpuInfo;
    bool m_initialized;
};

int main() {
    std::cout << "🔧 macOS 综合性能监控系统" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    PerformanceMonitor monitor;
    
    if (monitor.IsInitialized()) {
        // 运行30秒的综合测试
        monitor.RunContinuousTest(30);
    } else {
        std::cout << "❌ 监控系统初始化失败" << std::endl;
        return 1;
    }
    
    return 0;
}