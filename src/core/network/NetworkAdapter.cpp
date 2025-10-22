#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <windows.h>
#include <iphlpapi.h>

#include "NetworkAdapter.h"
#include "Logger.h"
#include <comutil.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

NetworkAdapter::NetworkAdapter(WmiManager& manager)
    : wmiManager(manager), initialized(false) {
    Initialize();
}

NetworkAdapter::~NetworkAdapter() {
    Cleanup();
}

void NetworkAdapter::Initialize() {
    if (wmiManager.IsInitialized()) {
        QueryAdapterInfo();
        initialized = true;
    }
    else {
        Logger::Error("WMI not initialized, cannot get network information");
    }
}

void NetworkAdapter::Cleanup() {
    adapters.clear();
    initialized = false;
}

void NetworkAdapter::Refresh() {
    Cleanup();
    Initialize();
}

void NetworkAdapter::QueryAdapterInfo() {
    QueryWmiAdapterInfo();
    UpdateAdapterAddresses();
}

bool NetworkAdapter::IsVirtualAdapter(const std::wstring& name) const {
    const std::vector<std::wstring> virtualKeywords = {
        L"VirtualBox",
        L"Hyper-V",
        L"Virtual",
        L"VPN",
        L"Bluetooth",
        L"VMware",
        L"Loopback",
        L"Microsoft Wi-Fi Direct"
    };

    for (const auto& keyword : virtualKeywords) {
        if (name.find(keyword) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

void NetworkAdapter::QueryWmiAdapterInfo() {
    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hres = wmiManager.GetWmiService()->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE PhysicalAdapter = True"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hres)) {
        Logger::Error("Network adapter WMI query failed: HRESULT=0x" + std::to_string(hres));
        return;
    }

    ULONG uReturn = 0;
    IWbemClassObject* pclsObj = nullptr;
    while (pEnumerator && pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        AdapterInfo info;
        info.isEnabled = false;

        // Get adapter name
        VARIANT vtName, vtDesc, vtStatus;
        VariantInit(&vtName);
        VariantInit(&vtDesc);
        VariantInit(&vtStatus);

        if (SUCCEEDED(pclsObj->Get(L"Name", 0, &vtName, 0, 0)) && vtName.vt == VT_BSTR) {
            info.name = vtName.bstrVal;
        }

        if (SUCCEEDED(pclsObj->Get(L"Description", 0, &vtDesc, 0, 0)) && vtDesc.vt == VT_BSTR) {
            info.description = vtDesc.bstrVal;
        }

        // Check adapter status
        if (SUCCEEDED(pclsObj->Get(L"NetEnabled", 0, &vtStatus, 0, 0))) {
            info.isEnabled = (vtStatus.boolVal == VARIANT_TRUE);
        }

        VariantClear(&vtName);
        VariantClear(&vtDesc);
        VariantClear(&vtStatus);

        // Filter virtual adapters
        if (IsVirtualAdapter(info.name) || IsVirtualAdapter(info.description)) {
            SafeRelease(pclsObj);
            continue;
        }

        // Get MAC address
        VARIANT vtMac;
        VariantInit(&vtMac);
        if (SUCCEEDED(pclsObj->Get(L"MACAddress", 0, &vtMac, 0, 0)) && vtMac.vt == VT_BSTR) {
            info.mac = vtMac.bstrVal;
        }
        VariantClear(&vtMac);

        // Initialize adapter type as Unknown
        info.adapterType = L"Unknown";

        if (!info.name.empty() && !info.mac.empty()) {
            adapters.push_back(info);
        }

        SafeRelease(pclsObj);
    }

    SafeRelease(pEnumerator);
}

std::wstring NetworkAdapter::FormatMacAddress(const unsigned char* address, size_t length) const {
    std::wstringstream ss;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0) ss << L":";
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill(L'0')
            << static_cast<int>(address[i]);
    }
    return ss.str();
}

void NetworkAdapter::UpdateAdapterAddresses() {
    ULONG bufferSize = 15000;
    std::vector<BYTE> buffer(bufferSize);
    PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&buffer[0]);

    DWORD result = GetAdaptersAddresses(AF_INET,
        GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS,  // Include gateway info
        nullptr,
        pAddresses,
        &bufferSize);

    if (result == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(bufferSize);
        pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&buffer[0]);
        result = GetAdaptersAddresses(AF_INET,
            GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS,
            nullptr,
            pAddresses,
            &bufferSize);
    }

    if (result != NO_ERROR) {
        Logger::Error("Failed to get network adapter addresses: " + std::to_string(result));
        return;
    }

    for (PIP_ADAPTER_ADDRESSES adapter = pAddresses; adapter; adapter = adapter->Next) {
        if (adapter->IfType != IF_TYPE_ETHERNET_CSMACD &&
            adapter->IfType != IF_TYPE_IEEE80211) {
            continue;
        }

        std::wstring macAddress = FormatMacAddress(
            adapter->PhysicalAddress,
            adapter->PhysicalAddressLength);

        for (auto& adapterInfo : adapters) {
            if (adapterInfo.mac == macAddress) {
                // Update connection status
                adapterInfo.isConnected = (adapter->OperStatus == IfOperStatusUp);

                // Update network speed - fix for disconnected adapters showing abnormal speeds
                if (adapterInfo.isConnected) {
                    // Only record real speed when connected
                    adapterInfo.speed = adapter->TransmitLinkSpeed;
                    adapterInfo.speedString = FormatSpeed(adapter->TransmitLinkSpeed);
                }
                else {
                    // Set to 0 when not connected
                    adapterInfo.speed = 0;
                    adapterInfo.speedString = L"Not connected";
                }

                // Determine adapter type
                adapterInfo.adapterType = DetermineAdapterType(
                    adapterInfo.name,
                    adapterInfo.description,
                    adapter->IfType
                );

                // Update IP address (only when connected)
                if (adapterInfo.isConnected) {
                    PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress;
                    while (address) {
                        if (address->Address.lpSockaddr->sa_family == AF_INET) {
                            char ipStr[INET_ADDRSTRLEN];
                            sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(address->Address.lpSockaddr);
                            inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN);
                            adapterInfo.ip = std::wstring(ipStr, ipStr + strlen(ipStr));
                            break;
                        }
                        address = address->Next;
                    }
                }
                else {
                    adapterInfo.ip = L"Not connected";
                }
                break;
            }
        }
    }
}

// Helper method to format network speed
std::wstring NetworkAdapter::FormatSpeed(uint64_t bitsPerSecond) const {
    const double GB = 1000000000.0;
    const double MB = 1000000.0;
    const double KB = 1000.0;

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1);

    if (bitsPerSecond >= GB) {
        ss << (bitsPerSecond / GB) << L" Gbps";
    }
    else if (bitsPerSecond >= MB) {
        ss << (bitsPerSecond / MB) << L" Mbps";
    }
    else if (bitsPerSecond >= KB) {
        ss << (bitsPerSecond / KB) << L" Kbps";
    }
    else {
        ss << bitsPerSecond << L" bps";
    }

    return ss.str();
}

// New method to determine adapter type
std::wstring NetworkAdapter::DetermineAdapterType(const std::wstring& name, const std::wstring& description, DWORD ifType) const {
    // First, judge by Windows interface type
    if (ifType == IF_TYPE_IEEE80211) {
        return L"Wireless";
    }
    else if (ifType == IF_TYPE_ETHERNET_CSMACD) {
        return L"Wired";
    }

    // If interface type is not clear, judge by name and description
    std::wstring combinedText = name + L" " + description;

    // Convert to lowercase for comparison
    std::wstring lowerText = combinedText;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::towlower);

    // Wireless adapter keywords
    const std::vector<std::wstring> wirelessKeywords = {
        L"wi-fi", L"wifi", L"wireless", L"802.11", L"wlan",
        L"ac", L"ax", L"n", L"g",
        L"realtek 8822ce", L"intel wireless", L"qualcomm atheros",
        L"broadcom", L"ralink", L"mediatek"
    };

    // Wired adapter keywords
    const std::vector<std::wstring> ethernetKeywords = {
        L"ethernet", L"gigabit", L"fast ethernet", L"lan",
        L"realtek pcie gbe", L"intel ethernet", L"killer ethernet"
    };

    // Check for wireless keywords
    for (const auto& keyword : wirelessKeywords) {
        if (lowerText.find(keyword) != std::wstring::npos) {
            return L"Wireless";
        }
    }

    // Check for wired keywords
    for (const auto& keyword : ethernetKeywords) {
        if (lowerText.find(keyword) != std::wstring::npos) {
            return L"Wired";
        }
    }

    // By default, infer from interface type
    if (ifType == IF_TYPE_ETHERNET_CSMACD || ifType == IF_TYPE_FASTETHER ||
        ifType == IF_TYPE_GIGABITETHERNET) {
        return L"Wired";
    }

    return L"Unknown Type";
}

void NetworkAdapter::SafeRelease(IUnknown* pInterface) {
    if (pInterface) {
        pInterface->Release();
    }
}

const std::vector<NetworkAdapter::AdapterInfo>& NetworkAdapter::GetAdapters() const {
    return adapters;
}
