#include <iostream>
#include <iomanip>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <mach/mach.h>
#include <thread>
#include <chrono>
#include <pwd.h>

// 简单的系统信息测试
void testBasicSystemInfo() {
    std::cout << "\n=== 基本系统信息测试 ===" << std::endl;
    
    // 获取系统信息
    struct utsname uname_info;
    if (uname(&uname_info) == 0) {
        std::cout << "操作系统: " << uname_info.sysname << std::endl;
        std::cout << "版本: " << uname_info.release << std::endl;
        std::cout << "构建: " << uname_info.version << std::endl;
        std::cout << "主机名: " << uname_info.nodename << std::endl;
        std::cout << "架构: " << uname_info.machine << std::endl;
    } else {
        std::cout << "✗ 获取系统信息失败" << std::endl;
    }
    
    // 获取主机名
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        std::cout << "主机名: " << hostname << std::endl;
    }
    
    // 获取用户名
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        std::cout << "用户名: " << pw->pw_name << std::endl;
    }
    
    // 获取运行时间
    struct timeval boottime;
    size_t size = sizeof(boottime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    
    if (sysctl(mib, 2, &boottime, &size, nullptr, 0) == 0) {
        auto now = std::chrono::system_clock::now();
        auto bootTime = std::chrono::system_clock::from_time_t(boottime.tv_sec);
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - bootTime);
        uint64_t uptime = duration.count();
        
        std::cout << "运行时间: " << uptime << " 秒 (" << (uptime / 3600) << " 小时)" << std::endl;
        
        uint64_t days = uptime / 86400;
        uint64_t hours = (uptime % 86400) / 3600;
        uint64_t minutes = (uptime % 3600) / 60;
        
        std::cout << "格式化运行时间: ";
        if (days > 0) std::cout << days << "天 ";
        if (hours > 0 || days > 0) std::cout << hours << "小时 ";
        std::cout << minutes << "分钟" << std::endl;
    } else {
        std::cout << "✗ 获取运行时间失败" << std::endl;
    }
    
    // 获取系统负载
    double loadAverage[3];
    if (getloadavg(loadAverage, 3) != -1) {
        std::cout << "系统负载 (1/5/15分钟): " 
                  << std::fixed << std::setprecision(2) 
                  << loadAverage[0] << ", " 
                  << loadAverage[1] << ", " 
                  << loadAverage[2] << std::endl;
    } else {
        std::cout << "✗ 获取系统负载失败" << std::endl;
    }
    
    // 获取内存信息
    uint64_t totalMemory = 0;
    size_t memSize = sizeof(totalMemory);
    if (sysctlbyname("hw.memsize", &totalMemory, &memSize, nullptr, 0) == 0) {
        std::cout << "总物理内存: " << (totalMemory / (1024 * 1024 * 1024)) << " GB" << std::endl;
        
        // 获取可用内存
        vm_statistics64_data_t vm_stat;
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        kern_return_t kr = host_statistics64(mach_host_self(), HOST_VM_INFO64,
                                               (host_info64_t)&vm_stat, &count);
        
        if (kr == KERN_SUCCESS) {
            uint64_t pageSize = vm_page_size;
            uint64_t freeMemory = vm_stat.free_count * pageSize;
            uint64_t inactiveMemory = vm_stat.inactive_count * pageSize;
            uint64_t availableMemory = freeMemory + inactiveMemory;
            uint64_t usedMemory = totalMemory - availableMemory;
            
            std::cout << "可用内存: " << (availableMemory / (1024 * 1024 * 1024)) << " GB" << std::endl;
            std::cout << "已用内存: " << (usedMemory / (1024 * 1024 * 1024)) << " GB" << std::endl;
            std::cout << "内存使用率: " << std::fixed << std::setprecision(1) 
                      << ((double)usedMemory / totalMemory * 100) << "%" << std::endl;
        } else {
            std::cout << "✗ 获取详细内存信息失败" << std::endl;
        }
    } else {
        std::cout << "✗ 获取总内存失败" << std::endl;
    }
    
    // 获取CPU核心数
    uint32_t cores = std::thread::hardware_concurrency();
    std::cout << "CPU核心数: " << cores << std::endl;
    
    std::cout << "✓ 基本系统信息测试完成" << std::endl;
}

int main() {
    std::cout << "=== macOS 简单系统信息检测测试 ===" << std::endl;
    std::cout << "测试基本的系统信息获取功能" << std::endl;
    
    try {
        testBasicSystemInfo();
        std::cout << "\n=== 所有测试完成 ===" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}