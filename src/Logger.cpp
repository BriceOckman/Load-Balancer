/**
 * @file Logger.cpp
 * @brief Implementation of the Logger class.
 * @author Brice Ockman
 */
#include "Logger.h"
#include "Colors.h"
#include <cstdio>
#include <iostream>

Logger::Logger(const std::string& path, bool verbose_, bool useColor_)
    : verbose(verbose_), useColor(useColor_) {
    if (!path.empty()) file.open(path);
}

const char* Logger::colorFor(EventType t) {
    switch (t) {
        case EventType::INFO: return Colors::BLUE;
        case EventType::SCALE_UP: return Colors::GREEN;
        case EventType::SCALE_DOWN: return Colors::YELLOW;
        case EventType::BLOCKED: return Colors::RED;
        case EventType::STATUS: return Colors::CYAN;
        case EventType::WARNING: return Colors::MAGENTA;
        case EventType::SWITCH: return Colors::BOLD;
    }
    return Colors::RESET;
}

const char* Logger::nameFor(EventType t) {
    switch (t) {
        case EventType::INFO: return "INFO";
        case EventType::SCALE_UP: return "SCALE_UP";
        case EventType::SCALE_DOWN: return "SCALE_DOWN";
        case EventType::BLOCKED: return "BLOCKED";
        case EventType::STATUS: return "STATUS";
        case EventType::WARNING: return "WARNING";
        case EventType::SWITCH: return "SWITCH";
    }
    return "INFO";
}

void Logger::event(EventType t, long cycle, const std::string& lbName, const std::string& msg) {
    if (!verbose) return; // quiet mode: events are counted by callers, not printed
    char cyc[32];
    std::snprintf(cyc, sizeof(cyc), "%06ld", cycle);
    std::string plain = "[cycle " + std::string(cyc) + "] [" + lbName + "] " +
                        nameFor(t) + " " + msg;
    if (useColor) std::cout << colorFor(t) << plain << Colors::RESET << '\n';
    else std::cout << plain << '\n';
    if (file) file << plain << '\n';
}

void Logger::summary(const std::string& text) {
    if (useColor) std::cout << Colors::BOLD_WHITE << text << Colors::RESET << '\n';
    else std::cout << text << '\n';
    if (file) file << text << '\n';
}

bool Logger::isVerbose() const { return verbose; }
bool Logger::fileOk() const { return file.is_open(); }
