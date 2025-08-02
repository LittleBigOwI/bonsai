#pragma once

#include "../core/scanner.hpp"

#include <ftxui/component/component.hpp>

#include <memory>
#include <vector>
#include <string>

namespace ui {
    ftxui::Component RenderSidebar(const std::string& path, int width, ScanSnapshot* snapshot);
}
