#include "TerminalRenderer.h"
#include "DisplayFormatter.h"
#include <iostream>

void TerminalRenderer::ClearScreen() {
    // Clear screen (ANSI escape sequences)
    std::cout << "\033[2J\033[H";
}

void TerminalRenderer::RenderTitle(const std::string& title) {
    std::cout << DisplayFormatter::FormatTitle(title);
}

void TerminalRenderer::RenderSection(const std::string& sectionName) {
    std::cout << DisplayFormatter::FormatSection(sectionName);
}

void TerminalRenderer::RenderKeyValue(const std::string& key, const std::string& value) {
    std::cout << DisplayFormatter::FormatKeyValue(key, value);
}

void TerminalRenderer::RenderProgressBar(float percent) {
    std::cout << DisplayFormatter::FormatProgressBar(percent);
}

void TerminalRenderer::RenderStatus(const std::string& status, bool ok) {
    std::cout << DisplayFormatter::FormatStatus(status, ok);
}

void TerminalRenderer::RenderLines(const std::vector<std::string>& lines) {
    for (const auto& line : lines) {
        std::cout << line << std::endl;
    }
}
