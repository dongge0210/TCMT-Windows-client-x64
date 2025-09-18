﻿#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <string>
#include <vector>
#include "WMIManager.h"

class NetworkAdapter {
public:
    struct AdapterInfo {
        std::wstring name;
        std::wstring mac;
        std::wstring ip;
        std::wstring description;
        std::wstring adapterType; // 新增：网卡类型（无线/有线）
        bool isEnabled;
        bool isConnected;
        uint64_t speed;
        std::wstring speedString;
    };

    explicit NetworkAdapter(WmiManager& manager);
    ~NetworkAdapter();

    const std::vector<AdapterInfo>& GetAdapters() const;
    void Refresh();

private:
    void Initialize();
    void Cleanup();
    void QueryAdapterInfo();
    void QueryWmiAdapterInfo();
    void UpdateAdapterAddresses();
    std::wstring FormatMacAddress(const unsigned char* address, size_t length) const;
    std::wstring FormatSpeed(uint64_t bitsPerSecond) const;  // 添加声明
    bool IsVirtualAdapter(const std::wstring& name) const;
    std::wstring DetermineAdapterType(const std::wstring& name, const std::wstring& description, DWORD ifType) const; // 新增：网卡类型识别
    void SafeRelease(IUnknown* pInterface);

    WmiManager& wmiManager;
    std::vector<AdapterInfo> adapters;
    bool initialized;
};
