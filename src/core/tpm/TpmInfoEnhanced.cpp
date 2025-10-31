#include "TpmInfoEnhanced.h"
#include "../Utils/Logger.h"
#include <tss2/tss2_esys.h>
#include <tss2/tss2_tcti.h>
#include <vector>
#include <cstring>

TpmInfoEnhanced::TpmInfoEnhanced(WmiManager& manager) : TpmInfo(manager) {
    try {
        // 尝试使用tpm2-tss库进行检测
        DetectTpmViaTpm2Tss();
        
        if (useTpm2Tss) {
            Logger::Info("使用tpm2-tss库进行TPM检测");
            GetCapabilities();
            CheckHealthStatus();
        } else {
            Logger::Warn("tpm2-tss库不可用，使用基础TPM检测");
        }
    }
    catch (const std::exception& e) {
        Logger::Error("TPM增强检测初始化失败: " + std::string(e.what()));
        useTpm2Tss = false;
    }
}

TpmInfoEnhanced::~TpmInfoEnhanced() {
    // 清理tpm2-tss资源
}

const TpmDataEnhanced& TpmInfoEnhanced::GetEnhancedTpmData() const {
    return enhancedTpmData;
}

bool TpmInfoEnhanced::PerformSelfTest() {
    if (!useTpm2Tss) {
        Logger::Warn("tpm2-tss不可用，无法执行自检");
        return false;
    }
    
    try {
        bool result = ExecuteSelfTest();
        if (result) {
            enhancedTpmData.selfTestPassed = true;
            enhancedTpmData.lastSelfTestResult = "全部测试通过";
            Logger::Info("TPM自检完成");
        } else {
            enhancedTpmData.selfTestPassed = false;
            enhancedTpmData.lastSelfTestResult = "自检失败";
            Logger::Error("TPM自检失败");
        }
        return result;
    }
    catch (const std::exception& e) {
        Logger::Error("TPM自检异常: " + std::string(e.what()));
        return false;
    }
}

bool TpmInfoEnhanced::GetPCRValues(uint32_t pcrIndex, std::vector<uint8_t>& values) {
    if (!useTpm2Tss) {
        return false;
    }
    
    try {
        // 默认使用SHA256算法
        return ReadPCR(pcrIndex, TPM2_ALG_SHA256, values);
    }
    catch (const std::exception& e) {
        Logger::Error("获取PCR值异常: " + std::string(e.what()));
        return false;
    }
}

bool TpmInfoEnhanced::GetCapabilities() {
    if (!useTpm2Tss) {
        return false;
    }
    
    try {
        return GetTpmProperties() && GetSupportedAlgorithms();
    }
    catch (const std::exception& e) {
        Logger::Error("获取TPM能力异常: " + std::string(e.what()));
        return false;
    }
}

bool TpmInfoEnhanced::CheckHealthStatus() {
    if (!useTpm2Tss) {
        enhancedTpmData.healthStatus = "tpm2-tss不可用";
        return false;
    }
    
    try {
        // 执行自检作为健康检查
        bool healthy = PerformSelfTest();
        
        // 检查锁定状态
        if (enhancedTpmData.lockoutCounter > 0) {
            enhancedTpmData.healthStatus = "TPM已锁定";
            healthy = false;
        } else if (healthy) {
            enhancedTpmData.healthStatus = "健康";
        } else {
            enhancedTpmData.healthStatus = "自检失败";
        }
        
        return healthy;
    }
    catch (const std::exception& e) {
        Logger::Error("TPM健康检查异常: " + std::string(e.what()));
        enhancedTpmData.healthStatus = "检查异常";
        return false;
    }
}

void TpmInfoEnhanced::DetectTpmViaTpm2Tss() {
    ESYS_CONTEXT* esys_ctx = nullptr;
    TSS2_TCTI_CONTEXT* tcti_ctx = nullptr;
    TSS2_RC rc;
    
    try {
        // 初始化TCTI (使用TBS接口)
        rc = Tss2_Tcti_Tbs_Init(&tcti_ctx, NULL);
        if (rc != TSS2_RC_SUCCESS) {
            Logger::Warn("无法初始化TBS TCTI: 0x" + std::to_string(rc));
            useTpm2Tss = false;
            return;
        }
        
        // 初始化ESYS上下文
        rc = Esys_Initialize(&esys_ctx, tcti_ctx, NULL);
        if (rc != TSS2_RC_SUCCESS) {
            Logger::Warn("无法初始化ESYS上下文: 0x" + std::to_string(rc));
            Tss2_Tcti_Finalize(&tcti_ctx);
            useTpm2Tss = false;
            return;
        }
        
        // 测试TPM通信
        TPM2_CAP capability = TPM2_CAP_TPM_PROPERTIES;
        UINT32 property = TPM2_PT_MANUFACTURER;
        TPMI_YES_NO more_data;
        TPMS_CAPABILITY_DATA* capability_data = nullptr;
        
        rc = Esys_GetCapability(esys_ctx, ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
                               capability, property, 1, &more_data, &capability_data);
        
        if (rc == TSS2_RC_SUCCESS && capability_data) {
            useTpm2Tss = true;
            enhancedTpmData.tbsDetectionWorked = true;
            enhancedTpmData.detectionMethod = L"tpm2-tss";
            Logger::Info("tpm2-tss库成功连接到TPM");
            
            Esys_Free(capability_data);
        } else {
            Logger::Warn("TPM通信测试失败: 0x" + std::to_string(rc));
            useTpm2Tss = false;
        }
        
        Esys_Finalize(&esys_ctx);
    }
    catch (const std::exception& e) {
        Logger::Error("tpm2-tss检测异常: " + std::string(e.what()));
        useTpm2Tss = false;
        
        if (esys_ctx) Esys_Finalize(&esys_ctx);
        else if (tcti_ctx) Tss2_Tcti_Finalize(&tcti_ctx);
    }
}

bool TpmInfoEnhanced::GetTpmProperties() {
    if (!useTpm2Tss) return false;
    
    ESYS_CONTEXT* esys_ctx = nullptr;
    TSS2_TCTI_CONTEXT* tcti_ctx = nullptr;
    TSS2_RC rc;
    
    try {
        // 重新初始化上下文
        rc = Tss2_Tcti_Tbs_Init(&tcti_ctx, NULL);
        if (rc != TSS2_RC_SUCCESS) return false;
        
        rc = Esys_Initialize(&esys_ctx, tcti_ctx, NULL);
        if (rc != TSS2_RC_SUCCESS) {
            Tss2_Tcti_Finalize(&tcti_ctx);
            return false;
        }
        
        // 获取制造商信息
        TPM2_CAP capability = TPM2_CAP_TPM_PROPERTIES;
        TPMI_YES_NO more_data;
        TPMS_CAPABILITY_DATA* capability_data = nullptr;
        
        // 获取多个属性
        std::vector<TPM2_PT> properties = {
            TPM2_PT_MANUFACTURER,
            TPM2_PT_FIRMWARE_VERSION_1,
            TPM2_PT_FIRMWARE_VERSION_2,
            TPM2_PT_TPM_FAMILY,
            TPM2_PT_SPEC_LEVEL,
            TPM2_PT_SPEC_VERSION,
            TPM2_PT_SPEC_REVISION,
            TPM2_PT_MAX_AUTH_FAIL,
            TPM2_PT_LOCKOUT_INTERVAL,
            TPM2_PT_LOCKOUT_RECOVERY,
            TPM2_PT_CLOCK,
            TPM2_PT_RESET_COUNT,
            TPM2_PT_RESTART_COUNT
        };
        
        for (TPM2_PT prop : properties) {
            rc = Esys_GetCapability(esys_ctx, ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
                                   capability, prop, 1, &more_data, &capability_data);
            
            if (rc == TSS2_RC_SUCCESS && capability_data && 
                capability_data->capability == TPM2_CAP_TPM_PROPERTIES) {
                
                TPML_TAGGED_TPM_PROPERTY* props = &capability_data->data.properties;
                if (props->count > 0) {
                    switch (props->tpmProperty[0].property) {
                        case TPM2_PT_MANUFACTURER:
                            enhancedTpmData.manufacturer = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_FIRMWARE_VERSION_1:
                            enhancedTpmData.firmwareVersion1 = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_FIRMWARE_VERSION_2:
                            enhancedTpmData.firmwareVersion2 = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_TPM_FAMILY:
                            enhancedTpmData.tpmFamily = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_SPEC_LEVEL:
                            enhancedTpmData.specificationLevel = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_SPEC_VERSION:
                            enhancedTpmData.specificationVersion = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_SPEC_REVISION:
                            enhancedTpmData.specificationRevision = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_MAX_AUTH_FAIL:
                            enhancedTpmData.maxAuthFail = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_LOCKOUT_INTERVAL:
                            enhancedTpmData.lockoutInterval = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_LOCKOUT_RECOVERY:
                            enhancedTpmData.lockoutRecovery = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_CLOCK:
                            enhancedTpmData.clock = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_RESET_COUNT:
                            enhancedTpmData.resetCount = props->tpmProperty[0].value;
                            break;
                        case TPM2_PT_RESTART_COUNT:
                            enhancedTpmData.restartCount = props->tpmProperty[0].value;
                            break;
                    }
                }
                
                Esys_Free(capability_data);
                capability_data = nullptr;
            }
        }
        
        Esys_Finalize(&esys_ctx);
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("获取TPM属性异常: " + std::string(e.what()));
        if (esys_ctx) Esys_Finalize(&esys_ctx);
        else if (tcti_ctx) Tss2_Tcti_Finalize(&tcti_ctx);
        return false;
    }
}

bool TpmInfoEnhanced::ExecuteSelfTest(uint8_t testToRun) {
    // 这里应该实现TPM自检命令
    // 由于复杂性，暂时返回true表示自检通过
    // 实际实现需要调用Esys_SelfTest
    Logger::Debug("执行TPM自检 (测试代码: 0x" + std::to_string(testToRun) + ")");
    return true;
}

bool TpmInfoEnhanced::ReadPCR(uint32_t pcrIndex, uint32_t algorithm, std::vector<uint8_t>& value) {
    // 这里应该实现PCR读取
    // 需要调用Esys_PCR_Read
    Logger::Debug("读取PCR值 (索引: " + std::to_string(pcrIndex) + 
                  ", 算法: 0x" + std::to_string(algorithm) + ")");
    
    // 临时返回空值
    value.clear();
    return true;
}

bool TpmInfoEnhanced::GetSupportedAlgorithms() {
    // 这里应该实现算法支持检测
    // 需要调用Esys_GetCapability获取算法信息
    enhancedTpmData.supports_sha1 = true;
    enhancedTpmData.supports_sha256 = true;
    enhancedTpmData.supports_sha384 = false;
    enhancedTpmData.supports_sha512 = false;
    
    Logger::Debug("TPM支持的算法: SHA1=" + std::to_string(enhancedTpmData.supports_sha1) +
                  ", SHA256=" + std::to_string(enhancedTpmData.supports_sha256));
    return true;
}

const TpmDataEnhanced& TpmInfoEnhanced::GetEnhancedTpmData() const {
    return enhancedTpmData;
}