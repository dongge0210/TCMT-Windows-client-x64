#include "TpmInfo.h"
#include "../Utils/Logger.h"
#include "../Utils/WmiManager.h"
#include "../Utils/WinUtils.h"
#include <tbs.h>
#include <wbemidl.h>
#include <comutil.h>
#include <string>

#pragma comment(lib, "tbs.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "wbemuuid.lib")

TpmInfo::TpmInfo(WmiManager& manager) : wmiManager(manager) {
    // Initialize defaults
    hasTpm = false;
    tpmData.detectionMethod = L"NONE";
    tpmData.wmiDetectionWorked = false;
    tpmData.tbsDetectionWorked = false;
    tpmData.status = L"Unknown";
    tpmData.errorMessage.clear();

    Logger::Info("TPM detection start (TBS primary, WMI fallback)");

    // Try TBS first
    try {
        DetectTpmViaTbs();
    } catch (...) {
        // keep defaults
        Logger::Warn("TBS detection threw an exception");
    }

    // Try WMI for details or as a fallback
    try {
        DetectTpmViaWmi();
    } catch (...) {
        Logger::Warn("WMI detection threw an exception");
    }

    DetermineDetectionMethod();

    if (hasTpm) {
        std::string manufacturerStr = WinUtils::WstringToString(tpmData.manufacturerName);
        std::string versionStr = WinUtils::WstringToString(tpmData.version);
        std::string statusStr = WinUtils::WstringToString(tpmData.status);
        std::string methodStr = WinUtils::WstringToString(tpmData.detectionMethod);
        Logger::Info("TPM detected: " + manufacturerStr + " v" + versionStr +
                     " (" + statusStr + ") [method: " + methodStr + "]");
    } else {
        std::string errorStr = WinUtils::WstringToString(tpmData.errorMessage);
        Logger::Info("No TPM detected: " + errorStr);
    }
}

TpmInfo::~TpmInfo() {
    Logger::Info("TPM detection end");
}

void TpmInfo::DetectTpmViaWmi() {
    if (!wmiManager.IsInitialized()) {
        Logger::Warn("WMI manager not initialized");
        return;
    }

    IWbemServices* pSvc = wmiManager.GetWmiService();
    if (!pSvc) {
        Logger::Warn("WMI service not available");
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

    if (FAILED(hres) || !pEnumerator) {
        Logger::Debug("Win32_Tpm query failed or returned null");
        return;
    }

    ULONG uReturn = 0;
    IWbemClassObject* pclsObj = nullptr;
    bool found = false;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        found = true;

        VARIANT vtManufacturerName{}, vtManufacturerId{}, vtSpecVersion{};
        VARIANT vtIsEnabled{}, vtIsActivated{}, vtIsOwned{}, vtPhysicalPresenceRequired{};

        VariantInit(&vtManufacturerName);
        VariantInit(&vtManufacturerId);
        VariantInit(&vtSpecVersion);
        VariantInit(&vtIsEnabled);
        VariantInit(&vtIsActivated);
        VariantInit(&vtIsOwned);
        VariantInit(&vtPhysicalPresenceRequired);

        if (SUCCEEDED(pclsObj->Get(L"ManufacturerName", 0, &vtManufacturerName, 0, 0)) && vtManufacturerName.vt == VT_BSTR) {
            tpmData.manufacturerName = vtManufacturerName.bstrVal;
        }
        if (SUCCEEDED(pclsObj->Get(L"ManufacturerId", 0, &vtManufacturerId, 0, 0)) && vtManufacturerId.vt == VT_I4) {
            wchar_t manufacturerIdStr[32];
            swprintf_s(manufacturerIdStr, L"0x%08X", vtManufacturerId.intVal);
            tpmData.manufacturerId = manufacturerIdStr;
        }
        if (SUCCEEDED(pclsObj->Get(L"SpecVersion", 0, &vtSpecVersion, 0, 0)) && vtSpecVersion.vt == VT_BSTR) {
            tpmData.version = vtSpecVersion.bstrVal;
        }
        if (SUCCEEDED(pclsObj->Get(L"IsEnabled_InitialValue", 0, &vtIsEnabled, 0, 0)) && vtIsEnabled.vt == VT_BOOL) {
            tpmData.isEnabled = (vtIsEnabled.boolVal == VARIANT_TRUE);
        }
        if (SUCCEEDED(pclsObj->Get(L"IsActivated_InitialValue", 0, &vtIsActivated, 0, 0)) && vtIsActivated.vt == VT_BOOL) {
            tpmData.isActivated = (vtIsActivated.boolVal == VARIANT_TRUE);
        }
        if (SUCCEEDED(pclsObj->Get(L"IsOwned_InitialValue", 0, &vtIsOwned, 0, 0)) && vtIsOwned.vt == VT_BOOL) {
            tpmData.isOwned = (vtIsOwned.boolVal == VARIANT_TRUE);
        }
        if (SUCCEEDED(pclsObj->Get(L"PhysicalPresenceRequired", 0, &vtPhysicalPresenceRequired, 0, 0)) && vtPhysicalPresenceRequired.vt == VT_BOOL) {
            tpmData.physicalPresenceRequired = (vtPhysicalPresenceRequired.boolVal == VARIANT_TRUE);
        }

        tpmData.isReady = tpmData.isEnabled && tpmData.isActivated;
        if (tpmData.isReady) tpmData.status = L"Ready";
        else if (tpmData.isEnabled && !tpmData.isActivated) tpmData.status = L"EnabledNotActivated";
        else if (!tpmData.isEnabled) tpmData.status = L"Disabled";
        else tpmData.status = L"Unknown";

        VariantClear(&vtManufacturerName);
        VariantClear(&vtManufacturerId);
        VariantClear(&vtSpecVersion);
        VariantClear(&vtIsEnabled);
        VariantClear(&vtIsActivated);
        VariantClear(&vtIsOwned);
        VariantClear(&vtPhysicalPresenceRequired);

        pclsObj->Release();
        break; // handle first
    }

    pEnumerator->Release();

    if (found) {
        tpmData.wmiDetectionWorked = true;
        hasTpm = true;
    }
}

void TpmInfo::DetectTpmViaTbs() {
    // Basic TBS probe
    TBS_HCONTEXT hContext = 0;
    TBS_CONTEXT_PARAMS params{};
    params.version = TBS_CONTEXT_VERSION_ONE;

    TBS_RESULT result = Tbsi_Context_Create(&params, &hContext);
    if (result == TBS_SUCCESS) {
        tpmData.tbsAvailable = true;

        TPM_DEVICE_INFO deviceInfo{};
        result = Tbsi_GetDeviceInfo(sizeof(deviceInfo), &deviceInfo);
        if (result == TBS_SUCCESS) {
            tpmData.tbsVersion = deviceInfo.tpmVersion;
            if (deviceInfo.tpmVersion == TPM_VERSION_12) {
                if (tpmData.version.empty()) tpmData.version = L"1.2";
            } else if (deviceInfo.tpmVersion == TPM_VERSION_20) {
                if (tpmData.version.empty()) tpmData.version = L"2.0";
            }
        }

        hasTpm = true; // TBS available implies TPM present
        tpmData.tbsDetectionWorked = true;
        if (tpmData.status.empty() || tpmData.status == L"Unknown") tpmData.status = L"DetectedViaTBS";

        Tbsip_Context_Close(hContext);
    } else {
        tpmData.tbsAvailable = false;
        // map common errors to messages
        switch (result) {
            case TBS_E_TPM_NOT_FOUND: tpmData.errorMessage = L"TPM not found"; break;
            case TBS_E_SERVICE_NOT_RUNNING: tpmData.errorMessage = L"TBS service not running"; break;
            case TBS_E_INSUFFICIENT_BUFFER: tpmData.errorMessage = L"Insufficient buffer"; break;
            case TBS_E_INVALID_PARAMETER: tpmData.errorMessage = L"Invalid parameter"; break;
            case TBS_E_ACCESS_DENIED: tpmData.errorMessage = L"Access denied"; break;
            default: tpmData.errorMessage = L"TBS error"; break;
        }
    }
}

const TpmInfo::TpmData& TpmInfo::GetTpmData() const {
    return tpmData;
}

void TpmInfo::DetermineDetectionMethod() {
    if (tpmData.tbsDetectionWorked && tpmData.wmiDetectionWorked) {
        tpmData.detectionMethod = L"TBS+WMI";
    } else if (tpmData.tbsDetectionWorked) {
        tpmData.detectionMethod = L"TBS";
    } else if (tpmData.wmiDetectionWorked) {
        tpmData.detectionMethod = L"WMI";
    } else {
        tpmData.detectionMethod = L"NONE";
    }
}