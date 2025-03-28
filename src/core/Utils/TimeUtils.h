#pragma once
#include <string>
#include <chrono>

namespace TimeUtils {
    // ��ȷ����ʱ������ͣ�100���뾫�ȣ�
    using SystemTimePoint = std::chrono::time_point<
        std::chrono::system_clock,
        std::chrono::duration<int64_t, std::ratio<1, 10000000>>
    >;

    std::string FormatTimePoint(const SystemTimePoint& tp);
    std::string GetBootTimeUtc();
}