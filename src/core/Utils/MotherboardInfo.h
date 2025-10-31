#pragma once
#include <string>
#include <wbemidl.h>

struct MotherboardInfo {
    std::string manufacturer;
    std::string product;
    std::string version;
    std::string serialNumber;
    std::string biosVendor;
    std::string biosVersion;
    std::string biosReleaseDate;
    bool isValid;
    
    MotherboardInfo() : isValid(false) {}
};

class MotherboardInfoCollector {
public:
    static MotherboardInfo CollectMotherboardInfo(IWbemServices* pSvc);
    
private:
    static std::string BstrToString(const BSTR& bstr);
    static bool QueryWmiProperty(IWbemServices* pSvc, const std::wstring& className, 
                                const std::wstring& propertyName, std::string& result);
};