// file: src/core/tpm/TpmInfo.cpp
#include "TpmInfo.h"
#include "../Utils/Logger.h"
#include "../Utils/WmiManager.h"
#include "../Utils/WinUtils.h"

#define WIN32_LEAN_AND_MEAN
#include <tbs.h>
#include <wbemidl.h>
#include <comutil.h>
#include <string>
#include <cwchar>

#pragma comment(lib, "tbs.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "wbemuuid.lib")

namespace {
    struct TbsContextGuard {
        TBS_HCONTEXT ctx{ 0 };
        ~TbsContextGuard() {
            if (ctx) {
                Tbsip_Context_Close(ctx);
                ctx = 0;
            }
        }
    };

    std::wstring MapTbsError(TBS_RESULT r) {
        switch (r) {
        case TBS_E_TPM_NOT_FOUND:        return L"TPM not found";
        case TBS_E_SERVICE_NOT_RUNNING:  return L"TBS service not running";
        case TBS_E_INSUFFICIENT_BUFFER:  return L"Insufficient buffer";
#ifdef TBS_E_BAD_PARAMETER
        case TBS_E_BAD_PARAMETER:        return L"Bad parameter";
#endif
#ifdef TBS_E_INTERNAL_ERROR
        case TBS_E_INTERNAL_ERROR:       return L"Internal error";
#endif
#ifdef TBS_E_IOERROR
        case TBS_E_IOERROR:              return L"I/O error";
#endif
#ifdef TBS_E_ACCESS_DENIED
        case TBS_E_ACCESS_DENIED:        return L"Access denied";
#endif
        default: {
            wchar_t buf[48];
            swprintf_s(buf, L"TBS error 0x%08X", r);
            return buf;
        }
        }
    }

    void SetStatusIfEmpty(std::wstring& status, const std::wstring& value) {
        if (status.empty() || status == L"Unknown") status = value;
    }
}

TpmInfo::TpmInfo(WmiManager& manager) : wmiManager(manager) {
    hasTpm = false;
    tpmData.detectionMethod = L"NONE";
    tpmData.wmiDetectionWorked = false;
    tpmData.tbsDetectionWorked = false;
    tpmData.status = L"Unknown";
    tpmData.errorMessage.clear();

    Logger::Info("TPM detection start (TBS primary, WMI fallback)");

    try { DetectTpmViaTbs(); }
    catch (...) { Logger::Warn("TBS detection threw an exception"); }
    try { DetectTpmViaWmi(); }
    catch (...) { Logger::Warn("WMI detection threw an exception"); }

    DetermineDetectionMethod();

    if (hasTpm) {
        Logger::Info("TPM detected: " +
            WinUtils::WstringToString(tpmData.manufacturerName) + " v" +
            WinUtils::WstringToString(tpmData.version) + " (" +
            WinUtils::WstringToString(tpmData.status) + ") [method: " +
            WinUtils::WstringToString(tpmData.detectionMethod) + "]");
    }
    else {
        Logger::Info("No TPM detected: " + WinUtils::WstringToString(tpmData.errorMessage));
    }
}

TpmInfo::~TpmInfo() {
    Logger::Debug("TPM detection end");
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

        if (SUCCEEDED(pclsObj->Get(L"ManufacturerName", 0, &vtManufacturerName, 0, 0)) && vtManufacturerName.vt == VT_BSTR)
            tpmData.manufacturerName = vtManufacturerName.bstrVal;

        if (SUCCEEDED(pclsObj->Get(L"ManufacturerId", 0, &vtManufacturerId, 0, 0)) && vtManufacturerId.vt == VT_I4) {
            wchar_t manufacturerIdStr[32];
            swprintf_s(manufacturerIdStr, L"0x%08X", vtManufacturerId.intVal);
            tpmData.manufacturerId = manufacturerIdStr;
        }

        if (SUCCEEDED(pclsObj->Get(L"SpecVersion", 0, &vtSpecVersion, 0, 0)) && vtSpecVersion.vt == VT_BSTR)
            tpmData.version = vtSpecVersion.bstrVal;

        if (SUCCEEDED(pclsObj->Get(L"IsEnabled_InitialValue", 0, &vtIsEnabled, 0, 0)) && vtIsEnabled.vt == VT_BOOL)
            tpmData.isEnabled = (vtIsEnabled.boolVal == VARIANT_TRUE);

        if (SUCCEEDED(pclsObj->Get(L"IsActivated_InitialValue", 0, &vtIsActivated, 0, 0)) && vtIsActivated.vt == VT_BOOL)
            tpmData.isActivated = (vtIsActivated.boolVal == VARIANT_TRUE);

        if (SUCCEEDED(pclsObj->Get(L"IsOwned_InitialValue", 0, &vtIsOwned, 0, 0)) && vtIsOwned.vt == VT_BOOL)
            tpmData.isOwned = (vtIsOwned.boolVal == VARIANT_TRUE);

        if (SUCCEEDED(pclsObj->Get(L"PhysicalPresenceRequired", 0, &vtPhysicalPresenceRequired, 0, 0)) && vtPhysicalPresenceRequired.vt == VT_BOOL)
            tpmData.physicalPresenceRequired = (vtPhysicalPresenceRequired.boolVal == VARIANT_TRUE);

        tpmData.isReady = tpmData.isEnabled && tpmData.isActivated;
        if (tpmData.isReady)          tpmData.status = L"Ready";
        else if (tpmData.isEnabled && !tpmData.isActivated) tpmData.status = L"EnabledNotActivated";
        else if (!tpmData.isEnabled)  tpmData.status = L"Disabled";
        else                          tpmData.status = L"Unknown";

        VariantClear(&vtManufacturerName);
        VariantClear(&vtManufacturerId);
        VariantClear(&vtSpecVersion);
        VariantClear(&vtIsEnabled);
        VariantClear(&vtIsActivated);
        VariantClear(&vtIsOwned);
        VariantClear(&vtPhysicalPresenceRequired);

        pclsObj->Release();
        break;
    }

    pEnumerator->Release();

    if (found) {
        tpmData.wmiDetectionWorked = true;
        hasTpm = true;
    }
}

void TpmInfo::DetectTpmViaTbs() {
    TbsContextGuard guard;
    TBS_CONTEXT_PARAMS params{};
    params.version = TBS_CONTEXT_VERSION_ONE;

    TBS_RESULT result = Tbsi_Context_Create(&params, &guard.ctx);
    if (result == TBS_SUCCESS) {
        tpmData.tbsAvailable = true;

        TPM_DEVICE_INFO deviceInfo{};
        result = Tbsi_GetDeviceInfo(sizeof(deviceInfo), &deviceInfo);
        if (result == TBS_SUCCESS) {
            tpmData.tbsVersion = deviceInfo.tpmVersion;
            if (deviceInfo.tpmVersion == TPM_VERSION_12) {
                SetStatusIfEmpty(tpmData.status, L"DetectedViaTBS");
                if (tpmData.version.empty()) tpmData.version = L"1.2";
            }
            else if (deviceInfo.tpmVersion == TPM_VERSION_20) {
                SetStatusIfEmpty(tpmData.status, L"DetectedViaTBS");
                if (tpmData.version.empty()) tpmData.version = L"2.0";
            }
        }

        hasTpm = true;
        tpmData.tbsDetectionWorked = true;
        SetStatusIfEmpty(tpmData.status, L"DetectedViaTBS");
    }
    else {
        tpmData.tbsAvailable = false;
        tpmData.errorMessage = MapTbsError(result);
        // do not set hasTpm=true here
    }
}

const TpmInfo::TpmData& TpmInfo::GetTpmData() const {
    return tpmData;
}

void TpmInfo::DetermineDetectionMethod() {
    if (tpmData.tbsDetectionWorked && tpmData.wmiDetectionWorked) {
        tpmData.detectionMethod = L"TBS+WMI";
    }
    else if (tpmData.tbsDetectionWorked) {
        tpmData.detectionMethod = L"TBS";
    }
    else if (tpmData.wmiDetectionWorked) {
        tpmData.detectionMethod = L"WMI";
    }
    else {
        tpmData.detectionMethod = L"NONE";
    }
}