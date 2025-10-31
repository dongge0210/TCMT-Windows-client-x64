#include "MotherboardInfo.h"
#include "Logger.h"
#include <comdef.h>
#include <sstream>

std::string MotherboardInfoCollector::BstrToString(const BSTR& bstr) {
    if (bstr == nullptr) return "";
    
    int len = SysStringLen(bstr);
    if (len == 0) return "";
    
    // 转换为UTF-8字符串
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, bstr, len, nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, bstr, len, &result[0], size_needed, nullptr, nullptr);
    return result;
}

bool MotherboardInfoCollector::QueryWmiProperty(IWbemServices* pSvc, const std::wstring& className, 
                                               const std::wstring& propertyName, std::string& result) {
    if (!pSvc) return false;
    
    HRESULT hres;
    IEnumWbemClassObject* pEnumerator = nullptr;
    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;
    VARIANT vtProp;
    
    try {
        // 查询指定类的所有实例
        std::wstring query = L"SELECT * FROM " + className;
        hres = pSvc->ExecQuery(
            bstr_t(L"WQL"),
            bstr_t(query.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &pEnumerator
        );
        
        if (FAILED(hres)) {
            Logger::Error("WMI查询失败: " + std::to_string(hres));
            return false;
        }
        
        // 获取第一个实例
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (uReturn == 0) {
            Logger::Warn("未找到 " + std::string(className.begin(), className.end()) + " 实例");
            return false;
        }
        
        // 获取属性值
        VariantInit(&vtProp);
        hres = pclsObj->Get(propertyName.c_str(), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres) && vtProp.vt == VT_BSTR && vtProp.bstrVal != nullptr) {
            result = BstrToString(vtProp.bstrVal);
            VariantClear(&vtProp);
            pclsObj->Release();
            pEnumerator->Release();
            return true;
        }
        
        VariantClear(&vtProp);
        pclsObj->Release();
        pEnumerator->Release();
        return false;
    }
    catch (...) {
        Logger::Error("查询WMI属性时发生异常");
        if (pclsObj) pclsObj->Release();
        if (pEnumerator) pEnumerator->Release();
        VariantClear(&vtProp);
        return false;
    }
}

MotherboardInfo MotherboardInfoCollector::CollectMotherboardInfo(IWbemServices* pSvc) {
    MotherboardInfo info;
    
    if (!pSvc) {
        Logger::Error("WMI服务未初始化，无法采集主板信息");
        return info;
    }
    
    try {
        // 采集主板信息
        if (QueryWmiProperty(pSvc, L"Win32_BaseBoard", L"Manufacturer", info.manufacturer)) {
            Logger::Debug("主板制造商: " + info.manufacturer);
        }
        
        if (QueryWmiProperty(pSvc, L"Win32_BaseBoard", L"Product", info.product)) {
            Logger::Debug("主板型号: " + info.product);
        }
        
        if (QueryWmiProperty(pSvc, L"Win32_BaseBoard", L"Version", info.version)) {
            Logger::Debug("主板版本: " + info.version);
        }
        
        if (QueryWmiProperty(pSvc, L"Win32_BaseBoard", L"SerialNumber", info.serialNumber)) {
            Logger::Debug("主板序列号: " + info.serialNumber);
        }
        
        // 采集BIOS信息
        if (QueryWmiProperty(pSvc, L"Win32_BIOS", L"Manufacturer", info.biosVendor)) {
            Logger::Debug("BIOS厂商: " + info.biosVendor);
        }
        
        if (QueryWmiProperty(pSvc, L"Win32_BIOS", L"Version", info.biosVersion)) {
            Logger::Debug("BIOS版本: " + info.biosVersion);
        }
        
        if (QueryWmiProperty(pSvc, L"Win32_BIOS", L"ReleaseDate", info.biosReleaseDate)) {
            Logger::Debug("BIOS发布日期: " + info.biosReleaseDate);
        }
        
        // 检查是否至少获取到了一些有效信息
        if (!info.manufacturer.empty() || !info.product.empty() || 
            !info.biosVendor.empty() || !info.biosVersion.empty()) {
            info.isValid = true;
            Logger::Info("主板/BIOS信息采集完成");
        } else {
            Logger::Warn("未能获取到有效的主板/BIOS信息");
        }
    }
    catch (...) {
        Logger::Error("采集主板信息时发生异常");
    }
    
    return info;
}