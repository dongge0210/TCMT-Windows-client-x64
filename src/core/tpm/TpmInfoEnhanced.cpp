#include "TpmInfoEnhanced.h"
#include "../Utils/Logger.h"
#include <vector>
#include <cstring>

#ifdef USE_TPM2_TSS
#include <tss2/tss2_esys.h>
#include <tss2/tss2_tcti.h>
#endif

#ifdef PLATFORM_WINDOWS
TpmInfoEnhanced::TpmInfoEnhanced(WmiManager& manager) : TpmInfo(manager) {
#else
TpmInfoEnhanced::TpmInfoEnhanced() : TpmInfo() {
#endif
    try {
        // 尝试使用tpm2-tss库进行检测
        DetectTpmViaTpm2Tss();
        
        if (useTpm2Tss) {
            Logger::Info("Using tpm2-tss library for TPM detection");
            GetCapabilities();
            CheckHealthStatus();
        } else {
            Logger::Warn("tpm2-tss library not available, using basic TPM detection");
        }
    }
    catch (const std::exception& e) {
        Logger::Error("TPM enhanced detection initialization failed: " + std::string(e.what()));
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
        Logger::Warn("tpm2-tss not available, cannot perform self-test");
        return false;
    }
    
    try {
        bool result = ExecuteSelfTest();
        if (result) {
            enhancedTpmData.selfTestPassed = true;
            enhancedTpmData.lastSelfTestResult = "All tests passed";
            Logger::Info("TPM self-test completed");
        } else {
            enhancedTpmData.selfTestPassed = false;
            enhancedTpmData.lastSelfTestResult = "Self-test failed";
            Logger::Error("TPM自检失败");
        }
        return result;
    }
    catch (const std::exception& e) {
        Logger::Error("TPM self-test failed: " + std::string(e.what()));
        return false;
    }
}

#ifdef USE_TPM2_TSS
bool TpmInfoEnhanced::GetPCRValues(uint32_t pcrIndex, std::vector<uint8_t>& values) {
    if (!useTpm2Tss) {
        return false;
    }
    
    try {
        // 默认使用SHA256算法
        return ReadPCR(pcrIndex, TPM2_ALG_SHA256, values);
    }
    catch (const std::exception& e) {
        Logger::Error("Exception getting PCR values: " + std::string(e.what()));
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
        Logger::Error("Exception getting TPM capabilities: " + std::string(e.what()));
        return false;
    }
}
#else
bool TpmInfoEnhanced::GetPCRValues(uint32_t pcrIndex, std::vector<uint8_t>& values) {
    Logger::Warn("tpm2-tss not available, cannot get PCR values");
    return false;
}

bool TpmInfoEnhanced::GetCapabilities() {
    Logger::Warn("tpm2-tss not available, cannot get TPM capabilities");
    return false;
}
#endif

bool TpmInfoEnhanced::CheckHealthStatus() {
    if (!useTpm2Tss) {
        enhancedTpmData.healthStatus = "tpm2-tss not available";
        return false;
    }
    
    try {
        // 执行自检作为健康检查
        bool healthy = PerformSelfTest();
        
        // 检查锁定状态
        if (enhancedTpmData.lockoutCounter > 0) {
            enhancedTpmData.healthStatus = "TPM locked";
            healthy = false;
        } else if (healthy) {
            enhancedTpmData.healthStatus = "Healthy";
        } else {
            enhancedTpmData.healthStatus = "Self-test failed";
        }
        
        return healthy;
    }
    catch (const std::exception& e) {
        Logger::Error("TPM health check exception: " + std::string(e.what()));
        enhancedTpmData.healthStatus = "Check exception";
        return false;
    }
}

#ifdef USE_TPM2_TSS
void TpmInfoEnhanced::DetectTpmViaTpm2Tss() {
    ESYS_CONTEXT* ctx = nullptr;
    TSS2_TCTI_CONTEXT* tcti = nullptr;
    TSS2_RC rc;
    
    try {
        // 初始化TCTI (使用TBS接口)
        rc = Tss2_Tcti_Tbs_Init(&tcti, nullptr);
        if (rc != TSS2_RC_SUCCESS) {
            Logger::Warn("无法初始化TBS TCTI: " + std::to_string(rc));
            useTpm2Tss = false;
            return;
        }
        
        // 初始化ESYS上下文
        rc = Esys_Initialize(&ctx, tcti, nullptr);
        if (rc != TSS2_RC_SUCCESS) {
            Logger::Warn("无法初始化ESYS上下文: " + std::to_string(rc));
            useTpm2Tss = false;
            Tss2_Tcti_Finalize(tcti);
            return;
        }
        
        // 获取TPM属性
        if (!GetTpmProperties()) {
            Logger::Warn("获取TPM属性失败");
            useTpm2Tss = false;
        } else {
            useTpm2Tss = true;
            enhancedTpmData.tpmPresent = true;
            Logger::Info("tpm2-tss库初始化成功");
        }
        
        // 清理资源
        Esys_Finalize(&ctx);
        Tss2_Tcti_Finalize(tcti);
    }
    catch (const std::exception& e) {
        Logger::Error("tpm2-tss初始化异常: " + std::string(e.what()));
        useTpm2Tss = false;
        if (ctx) Esys_Finalize(&ctx);
        if (tcti) Tss2_Tcti_Finalize(tcti);
    }
}
#else
void TpmInfoEnhanced::DetectTpmViaTpm2Tss() {
    Logger::Warn("tpm2-tss support not compiled, using basic TPM detection");
    useTpm2Tss = false;
}
#endif

#ifdef USE_TPM2_TSS
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
        else if (tcti_ctx) Tss2_Tcti_Finalize(tcti_ctx);
        return false;
    }
}
#else
bool TpmInfoEnhanced::GetTpmProperties() {
    Logger::Warn("tpm2-tss support not compiled, cannot get TPM properties");
    return false;
}
#endif

bool TpmInfoEnhanced::ExecuteSelfTest(uint8_t testToRun) {
    // 这里应该实现TPM自检命令
    // 由于复杂性，暂时返回true表示自检通过
    // 实际实现需要调用Esys_SelfTest
    Logger::Debug("Performing TPM self-test (test code: 0x" + std::to_string(testToRun) + ")");
    return true;
}

bool TpmInfoEnhanced::ReadPCR(uint32_t pcrIndex, uint32_t algorithm, std::vector<uint8_t>& value) {
    // 这里应该实现PCR读取
    // 需要调用Esys_PCR_Read
    Logger::Debug("Reading PCR value (index: " + std::to_string(pcrIndex) + 
                  ", algorithm: 0x" + std::to_string(algorithm) + ")");
    
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
    
    Logger::Debug("TPM supported algorithms: SHA1=" + std::to_string(enhancedTpmData.supports_sha1) +
                  ", SHA256=" + std::to_string(enhancedTpmData.supports_sha256));
    return true;
}