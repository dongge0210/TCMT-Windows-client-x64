#pragma once
#include "TpmInfo.h"
#include <memory>
#include <vector>
#include <string>

// 跨平台TPM信息结构
struct CrossPlatformTpmData {
    // 基础TPM信息
    bool tpmPresent = false;                  // TPM是否存在
    std::string manufacturer;                  // 制造商名称
    std::string manufacturerId;               // 制造商ID
    std::string version;                       // 版本号
    std::string firmwareVersion;              // 固件版本
    std::string status;                        // 状态描述
    
    // TPM属性
    bool isEnabled = false;                    // 是否启用
    bool isActivated = false;                  // 是否激活
    bool isOwned = false;                      // 是否拥有
    bool isReady = false;                      // 是否就绪
    
    // 规范信息
    uint32_t specVersion = 0;                  // 规范版本
    uint32_t specRevision = 0;                 // 规范修订
    std::string specLevel;                     // 规范级别
    
    // 检测方法
    std::string detectionMethod;               // 检测方法
    bool wmiDetectionWorked = false;           // WMI检测是否工作
    bool tpm2TssDetectionWorked = false;        // tpm2-tss检测是否工作
    
    // 错误信息
    std::string errorMessage;                  // 错误消息
};

// 跨平台TPM信息类
class CrossPlatformTpmInfo {
private:
    CrossPlatformTpmData tpmData;
    bool initialized = false;
    
    // 平台特定检测方法
    bool DetectTpmOnWindows();
    bool DetectTpmOnMacOS();
    bool DetectTpmOnLinux();
    
    // 通用检测方法
    bool DetectTpmViaSystemCommand();
    bool CheckTpmDevice(const std::string& devicePath);
    
    // tpm2-tss相关
    bool DetectTpmViaTpm2Tss();
    bool tpm2TssAvailable = false;
    
public:
    CrossPlatformTpmInfo();
    ~CrossPlatformTpmInfo();
    
    // 初始化TPM检测
    bool Initialize();
    
    // 获取TPM数据
    const CrossPlatformTpmData& GetTpmData() const { return tpmData; }
    
    // 检查TPM是否存在
    bool HasTpm() const { return tpmData.tpmPresent; }
    
    // 执行自检
    bool PerformSelfTest();
    
    // 获取TPM能力
    bool GetCapabilities();
    
    // 检查健康状态
    bool CheckHealthStatus();
    
    // 重置TPM
    bool ResetTpm();
    
    // 清除TPM
    bool ClearTpm();
    
    // 获取PCR值
    std::vector<uint8_t> GetPcrValue(uint32_t pcrIndex);
    
    // 获取随机数
    std::vector<uint8_t> GetRandom(uint32_t size);
    
    // 密封数据
    bool SealData(const std::vector<uint8_t>& data, 
                  const std::vector<uint8_t>& pcrList,
                  std::vector<uint8_t>& sealedData);
    
    // 解封数据
    bool UnsealData(const std::vector<uint8_t>& sealedData,
                    const std::vector<uint8_t>& pcrList,
                    std::vector<uint8_t>& data);
};