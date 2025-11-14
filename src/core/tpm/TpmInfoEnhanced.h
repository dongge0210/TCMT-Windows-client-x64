#pragma once
#include "TpmInfo.h"
#include <memory>
#include <vector>
#include <string>

#ifdef PLATFORM_WINDOWS
    class WmiManager;
#endif

// 增强的TPM信息结构，包含tpm2-tss库提供的详细信息
struct TpmDataEnhanced : public TpmInfo::TpmData {
    // 基础TPM信息
    bool tpmPresent = false;                  // TPM是否存在
    
    // TPM 2.0 特有信息
    uint32_t pcrCount = 0;                    // PCR寄存器数量
    uint32_t pcrSelectSize = 0;               // PCR选择大小
    std::vector<uint8_t> pcrValues[24];       // PCR值数组（最多24个）
    
    // 算法支持
    bool supports_sha1 = false;
    bool supports_sha256 = false;
    bool supports_sha384 = false;
    bool supports_sha512 = false;
    
    // TPM属性
    uint32_t manufacturer = 0;                 // 制造商ID (TCG分配)
    uint32_t firmwareVersion1 = 0;            // 固件版本1
    uint32_t firmwareVersion2 = 0;            // 固件版本2
    uint32_t tpmFamily = 0;                   // TPM家族
    uint32_t specificationLevel = 0;           // 规范级别
    uint32_t specificationRevision = 0;        // 规范修订版本
    
    // 永久性状态
    bool lockoutCounter = false;               // 锁定计数器
    uint32_t maxAuthFail = 0;                 // 最大认证失败次数
    uint32_t lockoutInterval = 0;              // 锁定间隔
    uint32_t lockoutRecovery = 0;             // 锁定恢复时间
    
    // 时间信息
    uint64_t clock = 0;                       // TPM时钟
    uint32_t resetCount = 0;                  // 重置计数
    uint64_t restartCount = 0;                // 重启计数
    
    // 安全属性
    bool supportsEncryption = false;           // 支持加密
    bool supportsSigning = false;              // 支持签名
    bool supportsAuthority = false;            // 支持授权
    
    // 完整性状态
    bool selfTestPassed = false;               // 自检通过
    std::string lastSelfTestResult;           // 最后自检结果
    std::string healthStatus;                 // 健康状态描述
};

class TpmInfoEnhanced : public TpmInfo {
public:
#ifdef PLATFORM_WINDOWS
    TpmInfoEnhanced(WmiManager& manager);
#else
    TpmInfoEnhanced();
#endif
    ~TpmInfoEnhanced();
    
    // 获取增强的TPM数据
    const TpmDataEnhanced& GetEnhancedTpmData() const;
    
    // 执行TPM自检
    bool PerformSelfTest();
    
    // 获取PCR值
    bool GetPCRValues(uint32_t pcrIndex, std::vector<uint8_t>& values);
    
    // 获取TPM能力信息
    bool GetCapabilities();
    
    // 检查TPM健康状况
    bool CheckHealthStatus();
    
private:
    // 使用tpm2-tss库进行高级检测
    void DetectTpmViaTpm2Tss();
    
    // 获取TPM属性
    bool GetTpmProperties();
    
    // 执行自检命令
    bool ExecuteSelfTest(uint8_t testToRun = 0xFF); // 0xFF = 全部测试
    
    // 读取PCR值
    bool ReadPCR(uint32_t pcrIndex, uint32_t algorithm, std::vector<uint8_t>& value);
    
    // 获取支持算法
    bool GetSupportedAlgorithms();
    
    TpmDataEnhanced enhancedTpmData;
    bool useTpm2Tss = false;  // 是否使用tpm2-tss库
};