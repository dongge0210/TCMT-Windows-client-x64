#include "TpmInfo.h"
#include "../Utils/Logger.h"
#include "../Utils/WMIManager.h"
#include <comutil.h>
#include <tbs.h>        // TPM Base Services
#include <wbemidl.h>

// Link TBS library and other necessary libraries
#pragma comment(lib, "tbs.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "wbemuuid.lib")

TpmInfo::TpmInfo(WmiManager& manager) : wmiManager(manager) {
    // Log in ASCII to avoid source encoding issues
    Logger::Info("Start TPM detection - prefer TBS, WMI as fallback");
    
    // Initialize detection method tracking
    tpmData.detectionMethod = L"Not detected";
    tpmData.wmiDetectionWorked = false;
    tpmData.tbsDetectionWorked = false;
    
    // Step 1: Try TBS detection first
    Logger::Info("Step 1: Try TBS detection...");
    try {
        DetectTpmViaTbs();
        if (tpmData.tbsAvailable) {
            tpmData.tbsDetectionWorked = true;
            hasTpm = true;
            Logger::Info("TBS detection succeeded - TPM present");
        } else {
            Logger::Info("TBS detection failed - will try WMI fallback");
        }
    } catch (const std::exception& e) {
        Logger::Error(std::string("TBS TPM detection exception: ") + e.what());
        std::string errorMsg = "TBS detection exception: ";
        errorMsg += e.what();
        tpmData.errorMessage = std::wstring(errorMsg.begin(), errorMsg.end());
    } catch (...) {
        Logger::Error("TBS TPM detection unknown exception");
        tpmData.errorMessage = L"TBS detection unknown exception";
    }
    
    // Step 2: Try WMI detection (regardless of TBS success, attempt to get more info)
    Logger::Info("Step 2: Try WMI detection for details...");
    
    if (!wmiManager.IsInitialized()) {
        Logger::Warn("WMI not initialized; cannot query TPM via WMI");
        if (!tpmData.tbsDetectionWorked) {
            tpmData.errorMessage = L"WMI service not initialized and TBS detection failed";
        }
    } else {
        pSvc = wmiManager.GetWmiService();
        if (!pSvc) {
            Logger::Warn("Cannot get WMI service; cannot query TPM via WMI");
            if (!tpmData.tbsDetectionWorked) {
                tpmData.errorMessage = L"Cannot get WMI service and TBS detection failed";
            }
        } else {
            try {
                DetectTpmViaWmi();
                if (tpmData.wmiDetectionWorked) {
                    Logger::Info("WMI detection succeeded - detailed TPM info");
                    if (!hasTpm) {
                        hasTpm = true; // WMI detected TPM even if TBS failed
                    }
                } else {
                    Logger::Info("WMI detection found no TPM info");
                }
            } catch (const std::exception& e) {
                Logger::Error(std::string("WMI TPM detection exception: ") + e.what());
                if (tpmData.errorMessage.empty()) {
                    std::string errorMsg = "WMI detection exception: ";
                    errorMsg += e.what();
                    tpmData.errorMessage = std::wstring(errorMsg.begin(), errorMsg.end());
                }
            } catch (...) {
                Logger::Error("WMI TPM detection unknown exception");
                if (tpmData.errorMessage.empty()) {
                    tpmData.errorMessage = L"WMI detection unknown exception";
                }
            }
        }
    }
    
    // Step 3: Determine final detection method and status
    DetermineDetectionMethod();
    
    // Log final detection results
    if (hasTpm) {
        std::string manufacturerStr(tpmData.manufacturerName.begin(), tpmData.manufacturerName.end());
        std::string versionStr(tpmData.version.begin(), tpmData.version.end());
        std::string statusStr(tpmData.status.begin(), tpmData.status.end());
        std::string methodStr(tpmData.detectionMethod.begin(), tpmData.detectionMethod.end());
        Logger::Info("TPM detection finished - found TPM: " + manufacturerStr + " v" + versionStr + 
                    " (" + statusStr + ") [method: " + methodStr + "]");
    } else {
        std::string errorStr(tpmData.errorMessage.begin(), tpmData.errorMessage.end());
        Logger::Info("TPM detection finished - not found: " + errorStr);
    }
}

TpmInfo::~TpmInfo() {
    Logger::Info("Finish TPM info detection");
}

void TpmInfo::DetectTpmViaWmi() {
    if (!pSvc) {
        Logger::Error("WMI service unavailable");
        return;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_Tpm"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hres)) {
        Logger::Warn("WMI query Win32_Tpm failed (maybe unsupported or not enabled)");
        return;
    }

    ULONG uReturn = 0;
    IWbemClassObject* pclsObj = nullptr;
    bool foundTpmViaWmi = false;
    
    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        foundTpmViaWmi = true;
        
        VARIANT vtManufacturerName, vtManufacturerId, vtSpecVersion;
        VARIANT vtIsEnabled, vtIsActivated, vtIsOwned, vtPhysicalPresenceRequired;
        
        VariantInit(&vtManufacturerName);
        VariantInit(&vtManufacturerId);
        VariantInit(&vtSpecVersion);
        VariantInit(&vtIsEnabled);
        VariantInit(&vtIsActivated);
        VariantInit(&vtIsOwned);
        VariantInit(&vtPhysicalPresenceRequired);

        // Get manufacturer information
        if (SUCCEEDED(pclsObj->Get(L"ManufacturerName", 0, &vtManufacturerName, 0, 0)) && 
            vtManufacturerName.vt == VT_BSTR) {
            tpmData.manufacturerName = vtManufacturerName.bstrVal;
        }

        if (SUCCEEDED(pclsObj->Get(L"ManufacturerId", 0, &vtManufacturerId, 0, 0)) && 
            vtManufacturerId.vt == VT_I4) {
            wchar_t manufacturerIdStr[32];
            swprintf_s(manufacturerIdStr, L"0x%08X", vtManufacturerId.intVal);
            tpmData.manufacturerId = manufacturerIdStr;
        }

        // Get specification version
        if (SUCCEEDED(pclsObj->Get(L"SpecVersion", 0, &vtSpecVersion, 0, 0)) && 
            vtSpecVersion.vt == VT_BSTR) {
            tpmData.version = vtSpecVersion.bstrVal;
        }

        // Get status information
        if (SUCCEEDED(pclsObj->Get(L"IsEnabled_InitialValue", 0, &vtIsEnabled, 0, 0)) && 
            vtIsEnabled.vt == VT_BOOL) {
            tpmData.isEnabled = (vtIsEnabled.boolVal == VARIANT_TRUE);
        }

        if (SUCCEEDED(pclsObj->Get(L"IsActivated_InitialValue", 0, &vtIsActivated, 0, 0)) && 
            vtIsActivated.vt == VT_BOOL) {
            tpmData.isActivated = (vtIsActivated.boolVal == VARIANT_TRUE);
        }

        if (SUCCEEDED(pclsObj->Get(L"IsOwned_InitialValue", 0, &vtIsOwned, 0, 0)) && 
            vtIsOwned.vt == VT_BOOL) {
            tpmData.isOwned = (vtIsOwned.boolVal == VARIANT_TRUE);
        }

        if (SUCCEEDED(pclsObj->Get(L"PhysicalPresenceRequired", 0, &vtPhysicalPresenceRequired, 0, 0)) && 
            vtPhysicalPresenceRequired.vt == VT_BOOL) {
            tpmData.physicalPresenceRequired = (vtPhysicalPresenceRequired.boolVal == VARIANT_TRUE);
        }

        // Set overall status
        tpmData.isReady = tpmData.isEnabled && tpmData.isActivated;
        
        if (tpmData.isReady) {
            if (tpmData.status.empty() || tpmData.status == L"Detected via TBS") {
                tpmData.status = L"Ready";
            }
        } else if (tpmData.isEnabled && !tpmData.isActivated) {
            if (tpmData.status.empty() || tpmData.status == L"Detected via TBS") {
                tpmData.status = L"Enabled but not activated";
            }
        } else if (!tpmData.isEnabled) {
            if (tpmData.status.empty() || tpmData.status == L"Detected via TBS") {
                tpmData.status = L"Not enabled";
            }
        } else {
            if (tpmData.status.empty() || tpmData.status == L"Detected via TBS") {
                tpmData.status = L"Unknown status";
            }
        }

        // Clean up variables
        VariantClear(&vtManufacturerName);
        VariantClear(&vtManufacturerId);
        VariantClear(&vtSpecVersion);
        VariantClear(&vtIsEnabled);
        VariantClear(&vtIsActivated);
        VariantClear(&vtIsOwned);
        VariantClear(&vtPhysicalPresenceRequired);
        
        pclsObj->Release();
        
        // Log TPM information
        std::string manufacturerStr(tpmData.manufacturerName.begin(), tpmData.manufacturerName.end());
        std::string versionStr(tpmData.version.begin(), tpmData.version.end());
        std::string statusStr(tpmData.status.begin(), tpmData.status.end());
        
        Logger::Info("WMI detected TPM: " + manufacturerStr + 
                    ", version: " + versionStr + 
                    ", status: " + statusStr + 
                    ", enabled: " + (tpmData.isEnabled ? "yes" : "no") +
                    ", activated: " + (tpmData.isActivated ? "yes" : "no"));
        
        break; // Only process first TPM
    }

    pEnumerator->Release();

    if (foundTpmViaWmi) {
        tpmData.wmiDetectionWorked = true;
        hasTpm = true;
        Logger::Info("WMI detected TPM device");
    } else {
        Logger::Info("WMI did not detect TPM device");
    }
}

void TpmInfo::DetectTpmViaTbs() {
    // Try to detect TBS (TPM Base Services)
    TBS_HCONTEXT hContext = 0;
    
    // Use TBS_CONTEXT_PARAMS compatible with older Windows SDK
    TBS_CONTEXT_PARAMS contextParams = { 0 };
    contextParams.version = TBS_CONTEXT_VERSION_ONE; // Basic version, better compatibility

    TBS_RESULT result = Tbsi_Context_Create(&contextParams, &hContext);
    
    if (result == TBS_SUCCESS) {
        tpmData.tbsAvailable = true;
        
        // Get TBS version information
        TPM_DEVICE_INFO deviceInfo = {0};
        
        result = Tbsi_GetDeviceInfo(sizeof(deviceInfo), &deviceInfo);
        if (result == TBS_SUCCESS) {
            tpmData.tbsVersion = deviceInfo.tpmVersion;
            
            // Set version string based on TPM version
            if (deviceInfo.tpmVersion == TPM_VERSION_12) {
                if (tpmData.version.empty()) {
                    tpmData.version = L"1.2";
                }
                Logger::Info("Detected TPM 1.2");
            } else if (deviceInfo.tpmVersion == TPM_VERSION_20) {
                if (tpmData.version.empty()) {
                    tpmData.version = L"2.0";
                }
                Logger::Info("Detected TPM 2.0");
            } else {
                Logger::Info("Detected unknown TPM version: " + std::to_string(deviceInfo.tpmVersion));
            }
            
            Logger::Info("TBS available, TPM version: " + std::to_string(deviceInfo.tpmVersion));
        } else {
            Logger::Warn("Tbsi_GetDeviceInfo failed: 0x" + std::to_string(result));
        }
        
        // If WMI didn't detect TPM but TBS is available, TPM exists
        if (!hasTpm) {
            hasTpm = true;
            tpmData.isEnabled = true; // TBS available means TPM is enabled
            tpmData.isActivated = true; // If TBS is available, TPM is usually activated
            tpmData.status = L"Detected via TBS";
            Logger::Info("TPM detected via TBS");
        }
        
        // Close TBS context
        Tbsip_Context_Close(hContext);
    } else {
        tpmData.tbsAvailable = false;
        
        // Set detailed error information
        switch (result) {
#ifdef TBS_E_TPM_NOT_FOUND
            case TBS_E_TPM_NOT_FOUND:
                tpmData.errorMessage = L"TPM device not found";
                Logger::Info("TBS result: TPM not found");
                break;
#endif
#ifdef TBS_E_SERVICE_NOT_RUNNING
            case TBS_E_SERVICE_NOT_RUNNING:
                tpmData.errorMessage = L"TPM Base Services not running";
                Logger::Warn("TBS result: TPM Base Services not running");
                break;
#endif
#ifdef TBS_E_INSUFFICIENT_BUFFER
            case TBS_E_INSUFFICIENT_BUFFER:
                tpmData.errorMessage = L"Insufficient buffer";
                Logger::Warn("TBS result: insufficient buffer");
                break;
#endif
#ifdef TBS_E_INVALID_PARAMETER
            case TBS_E_INVALID_PARAMETER:
                tpmData.errorMessage = L"Invalid parameter";
                Logger::Warn("TBS result: invalid parameter");
                break;
#endif
#ifdef TBS_E_ACCESS_DENIED
            case TBS_E_ACCESS_DENIED:
                tpmData.errorMessage = L"Access denied";
                Logger::Warn("TBS result: access denied");
                break;
#endif
            default:
                tpmData.errorMessage = L"TBS initialization failed: 0x" + std::to_wstring(result);
                Logger::Warn("TBS detection failed, error: 0x" + std::to_string(result));
                break;
        }
        
        std::string errorStr(tpmData.errorMessage.begin(), tpmData.errorMessage.end());
        Logger::Warn("TBS detection failed: " + errorStr);
    }
}

const TpmInfo::TpmData& TpmInfo::GetTpmData() const {
    return tpmData;
}

void TpmInfo::DetermineDetectionMethod() {
    // Determine method used based on detection results
    if (tpmData.tbsDetectionWorked && tpmData.wmiDetectionWorked) {
        tpmData.detectionMethod = L"TBS+WMI";
        Logger::Info("Detection methods: TBS(primary) + WMI(details)");
    } else if (tpmData.tbsDetectionWorked && !tpmData.wmiDetectionWorked) {
        tpmData.detectionMethod = L"TBS";
        Logger::Info("Detection method: TBS only");
    } else if (!tpmData.tbsDetectionWorked && tpmData.wmiDetectionWorked) {
        tpmData.detectionMethod = L"WMI";
        Logger::Info("Detection method: WMI fallback");
    } else {
        tpmData.detectionMethod = L"Not detected";
        Logger::Info("Detection method: none - TPM not detected");
        
        // If no TPM detected, set appropriate error message
        if (tpmData.errorMessage.empty()) {
            tpmData.errorMessage = L"Both TBS and WMI detection failed";
        }
    }
}