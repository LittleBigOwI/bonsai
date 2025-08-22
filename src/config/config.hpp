#pragma once

#include "gen/defaults.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <optional>
#include <array>
#include <vector>
#include <string>
#include <stdexcept>

namespace fs = std::filesystem;

struct Config {
    int SCANNER_THREADS;
    int SIDEBAR_WIDTH;
    std::string SIDEBAR_SELECTED_FOLDER_ICON;
    std::string SIDEBAR_SELECTED_FILE_ICON;
    std::string SIDEBAR_FOLDER_ICON;
    std::string SIDEBAR_FILE_ICON;
    std::string SIDEBAR_BACK_ICON;
    int CHART_MAX_SIZE_THRESHOLD_PERCENTAGE;
    int CHART_MAX_GENERATIONS;
    double CHART_DIM_FACTOR;
    std::vector<std::array<int, 3>> CHART_COLORS;

    // Get singleton instance
    static const Config& get() {
        static Config instance = []() -> Config {
            if (!ensureConfigFile()) throw std::runtime_error("Cannot create config file");
            std::string path = getUserConfigPath();
            auto cfgOpt = parseConfigFile(path);
            if (!cfgOpt) throw std::runtime_error("Invalid config file");
            return *cfgOpt;
        }();
        return instance;
    }

private:
    // Helpers
    inline static std::string getUserConfigPath() {
        const char* home = getenv("HOME");
        if (!home) return "";
        return std::string(home) + "/.config/bonsai/bonsai.conf";
    }

    inline static bool writeDefaultConfig(const std::string& path) {
        std::ofstream out(path, std::ios::binary);
        if (!out.is_open()) return false;
        out.write(reinterpret_cast<const char*>(bonsai_default), bonsai_default_len);
        return true;
    }

    inline static bool ensureConfigFile() {
        std::string config_path = getUserConfigPath();
        fs::path config_dir = fs::path(config_path).parent_path();
        if (!fs::exists(config_dir)) fs::create_directories(config_dir);
        if (!fs::exists(config_path)) return writeDefaultConfig(config_path);
        return true;
    }

    inline static std::optional<Config> parseConfigFile(const std::string& config_path) {
        std::ifstream in(config_path);
        if (!in.is_open()) return std::nullopt;

        Config cfg;
        std::string line;
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t"));
            s.erase(s.find_last_not_of(" \t") + 1);
        };

        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            trim(key); trim(value);

            try {
                if (key == "SCANNER_THREADS") cfg.SCANNER_THREADS = std::stoi(value);
                else if (key == "SIDEBAR_WIDTH") cfg.SIDEBAR_WIDTH = std::stoi(value);
                else if (key == "SIDEBAR_SELECTED_FOLDER_ICON") cfg.SIDEBAR_SELECTED_FOLDER_ICON = value.substr(1, value.size() - 2);
                else if (key == "SIDEBAR_SELECTED_FILE_ICON") cfg.SIDEBAR_SELECTED_FILE_ICON = value.substr(1, value.size() - 2);
                else if (key == "SIDEBAR_FOLDER_ICON") cfg.SIDEBAR_FOLDER_ICON = value.substr(1, value.size() - 2);
                else if (key == "SIDEBAR_FILE_ICON") cfg.SIDEBAR_FILE_ICON = value.substr(1, value.size() - 2);
                else if (key == "SIDEBAR_BACK_ICON") cfg.SIDEBAR_BACK_ICON = value.substr(1, value.size() - 2);
                else if (key == "CHART_MAX_SIZE_THRESHOLD_PERCENTAGE") cfg.CHART_MAX_SIZE_THRESHOLD_PERCENTAGE = std::stoi(value);
                else if (key == "CHART_MAX_GENERATIONS") cfg.CHART_MAX_GENERATIONS = std::stoi(value);
                else if (key == "CHART_DIM_FACTOR") cfg.CHART_DIM_FACTOR = std::stod(value);
                else if (key == "CHART_COLORS") {
                    std::string colors_block;
                    if (value.find('[') != std::string::npos) {
                        colors_block += value.substr(value.find('[') + 1);
                        std::string line2;
                        while (std::getline(in, line2)) {
                            if (line2.find(']') != std::string::npos) {
                                colors_block += line2.substr(0, line2.find(']'));
                                break;
                            }
                            colors_block += line2;
                        }
                    }
                    std::stringstream ss(colors_block);
                    std::string tuple;
                    while (std::getline(ss, tuple, '}')) {
                        size_t start = tuple.find('{');
                        if (start == std::string::npos) continue;
                        tuple = tuple.substr(start + 1);
                        std::stringstream ts(tuple);
                        std::array<int, 3> color{};
                        for (int i = 0; i < 3 && ts.good(); i++) {
                            std::string num;
                            std::getline(ts, num, ',');
                            color[i] = std::stoi(num);
                        }
                        cfg.CHART_COLORS.push_back(color);
                    }
                }
            } catch (...) {
                return std::nullopt;
            }
        }

        return cfg;
    }
};