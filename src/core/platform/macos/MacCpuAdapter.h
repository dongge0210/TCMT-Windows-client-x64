#pragma once
#include "../../common/BaseInfo.h"
#include "../../common/PlatformDefs.h"
#include "../../cpu/CpuInfo.h"
#include <memory>
#include <string>

#ifdef PLATFORM_MACOS

// 前向声明
class MacCpuInfo;

class MacCpuAdapter : public ICpuAdapter {
public:
    MacCpuAdapter();
    virtual ~MacCpuAdapter();

    // ICpuAdapter 接口实现
    virtual double GetUsage() override;
    virtual std::string GetName() override;
    virtual int GetTotalCores() const override;
    virtual int GetSmallCores() const override;
    virtual int GetLargeCores() const override;
    virtual double GetLargeCoreSpeed() const override;
    virtual double GetSmallCoreSpeed() const override;
    virtual DWORD GetCurrentSpeed() const override;
    virtual bool IsHyperThreadingEnabled() const override;
    virtual bool IsVirtualizationEnabled() const override;
    virtual double GetLastSampleIntervalMs() const override;
    virtual double GetBaseFrequencyMHz() const override;
    virtual double GetCurrentFrequencyMHz() const override;
    virtual bool Initialize() override;
    virtual void Cleanup() override;
    virtual bool Update() override;

private:
    std::unique_ptr<MacCpuInfo> m_macCpuInfo;
};

#endif // PLATFORM_MACOS