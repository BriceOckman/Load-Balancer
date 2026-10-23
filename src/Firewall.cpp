/**
 * @file Firewall.cpp
 * @brief Implementation of the Firewall IP range blocker.
 * @author Brice Ockman
 */
#include "Firewall.h"
#include <sstream>
#include <stdexcept>

std::uint32_t Firewall::ipToUint(const std::string& ip) {
    std::uint32_t parts[4] = {0, 0, 0, 0};
    std::stringstream ss(ip);
    std::string token;
    int idx = 0;
    while (std::getline(ss, token, '.')) {
        if (idx >= 4 || token.empty() || token.size() > 3)
            throw std::invalid_argument("invalid IP: " + ip);
        for (char c : token)
            if (c < '0' || c > '9') throw std::invalid_argument("invalid IP: " + ip);
        unsigned long v = std::stoul(token);
        if (v > 255) throw std::invalid_argument("invalid IP: " + ip);
        parts[idx++] = static_cast<std::uint32_t>(v);
    }
    if (idx != 4) throw std::invalid_argument("invalid IP: " + ip);
    return (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
}

std::string Firewall::uintToIp(std::uint32_t v) {
    return std::to_string((v >> 24) & 0xFF) + "." + std::to_string((v >> 16) & 0xFF) +
           "." + std::to_string((v >> 8) & 0xFF) + "." + std::to_string(v & 0xFF);
}

bool Firewall::addRange(const std::string& spec) {
    try {
        auto dash = spec.find('-');
        auto slash = spec.find('/');
        if (dash != std::string::npos) {
            std::uint32_t s = ipToUint(spec.substr(0, dash));
            std::uint32_t e = ipToUint(spec.substr(dash + 1));
            addRange(s, e);
            return true;
        }
        if (slash != std::string::npos) {
            std::uint32_t base = ipToUint(spec.substr(0, slash));
            int prefix = std::stoi(spec.substr(slash + 1));
            if (prefix < 0 || prefix > 32) return false;
            std::uint32_t mask = (prefix == 0) ? 0u : (0xFFFFFFFFu << (32 - prefix));
            std::uint32_t s = base & mask;
            std::uint32_t e = s | ~mask;
            addRange(s, e);
            return true;
        }
        // A single IP is treated as a range of one.
        std::uint32_t v = ipToUint(spec);
        addRange(v, v);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void Firewall::addRange(std::uint32_t start, std::uint32_t end) {
    if (start > end) std::swap(start, end);
    ranges.push_back({start, end});
}

bool Firewall::isBlocked(const std::string& ip) {
    std::uint32_t v;
    try {
        v = ipToUint(ip);
    } catch (const std::invalid_argument&) {
        return false; // malformed IPs are not matched by any rule
    }
    for (const auto& r : ranges) {
        if (v >= r.start && v <= r.end) {
            ++blockedCount;
            return true;
        }
    }
    return false;
}

long Firewall::getBlockedCount() const { return blockedCount; }
const std::vector<IPRange>& Firewall::getRanges() const { return ranges; }
