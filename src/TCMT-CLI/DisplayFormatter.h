#pragma once
#include <string>
#include <vector>

class DisplayFormatter {
public:
    static std::string FormatTitle(const std::string& title);
    static std::string FormatSection(const std::string& sectionName);
    static std::string FormatKeyValue(const std::string& key, const std::string& value, int keyWidth = 16);
    static std::string FormatProgressBar(float percent, int width = 30);
    static std::string FormatStatus(const std::string& status, bool ok);
    static std::string ColorText(const std::string& text, int colorCode);
};
