/**
 * @file Config.cpp
 * @brief Implementation of the Config file parser.
 * @author Brice Ockman
 */
#include "Config.h"
#include <algorithm>
#include <cctype>
#include <fstream>

namespace {
std::string trim(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
} // namespace

bool Config::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    values.clear();
    blockedRanges.clear();
    std::string line;
    while (std::getline(in, line)) {
        // Strip trailing comments, then skip blanks and full-line comments.
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        line = trim(line);
        if (line.empty()) continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue; // malformed line: ignore
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        if (key == "blocked_range") blockedRanges.push_back(value);
        else values[key] = value;
    }
    return true;
}

int Config::getInt(const std::string& key, int def) const {
    auto it = values.find(key);
    if (it == values.end()) return def;
    try { return std::stoi(it->second); } catch (const std::exception&) { return def; }
}

double Config::getDouble(const std::string& key, double def) const {
    auto it = values.find(key);
    if (it == values.end()) return def;
    try { return std::stod(it->second); } catch (const std::exception&) { return def; }
}

bool Config::getBool(const std::string& key, bool def) const {
    auto it = values.find(key);
    if (it == values.end()) return def;
    std::string v = lower(it->second);
    if (v == "true" || v == "1" || v == "yes") return true;
    if (v == "false" || v == "0" || v == "no") return false;
    return def;
}

std::string Config::getString(const std::string& key, const std::string& def) const {
    auto it = values.find(key);
    return (it == values.end()) ? def : it->second;
}

std::vector<std::string> Config::getBlockedRanges() const { return blockedRanges; }

void Config::set(const std::string& key, const std::string& value) { values[key] = value; }
