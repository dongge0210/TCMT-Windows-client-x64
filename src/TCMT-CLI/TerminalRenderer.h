#pragma once
#include <string>
#include <vector>

class TerminalRenderer {
public:
    void ClearScreen();
    void RenderTitle(const std::string& title);
    void RenderSection(const std::string& sectionName);
    void RenderKeyValue(const std::string& key, const std::string& value);
    void RenderProgressBar(float percent);
    void RenderStatus(const std::string& status, bool ok);
    void RenderLines(const std::vector<std::string>& lines);
};
