#include "DisplayFormatter.h"
#include <sstream>
#include <iomanip>

std::string DisplayFormatter::FormatTitle(const std::string& title) {
    std::ostringstream oss;
    oss << "\033[1;36m==== " << title << " ====\033[0m\n";
    return oss.str();
}

std::string DisplayFormatter::FormatSection(const std::string& sectionName) {
    std::ostringstream oss;
    oss << "\033[1;34m-- " << sectionName << " --\033[0m\n";
    return oss.str();
}

std::string DisplayFormatter::FormatKeyValue(const std::string& key, const std::string& value, int keyWidth) {
    std::ostringstream oss;
    oss << std::left << std::setw(keyWidth) << key << ": " << value << "\n";
    return oss.str();
}

std::string DisplayFormatter::FormatProgressBar(float percent, int width) {
    int filled = static_cast<int>(percent * width);
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) oss << "#";
        else oss << "-";
    }
    oss << "] " << std::fixed << std::setprecision(1) << (percent * 100) << "%\n";
    return oss.str();
}

std::string DisplayFormatter::FormatStatus(const std::string& status, bool ok) {
    int color = ok ? 32 : 31;
    return ColorText(status, color) + "\n";
}

std::string DisplayFormatter::ColorText(const std::string& text, int colorCode) {
    std::ostringstream oss;
    oss << "\033[" << colorCode << "m" << text << "\033[0m";
    return oss.str();
}
