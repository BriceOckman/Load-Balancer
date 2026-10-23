/**
 * @file Colors.h
 * @brief ANSI color constants used by the Logger for colored console output.
 * @author Brice Ockman
 */
#ifndef COLORS_H
#define COLORS_H

/// ANSI escape sequences. The log file always receives plain text without these.
namespace Colors {
constexpr const char* RESET   = "\033[0m";
constexpr const char* RED     = "\033[31m";
constexpr const char* GREEN   = "\033[32m";
constexpr const char* YELLOW  = "\033[33m";
constexpr const char* BLUE    = "\033[34m";
constexpr const char* MAGENTA = "\033[35m";
constexpr const char* CYAN    = "\033[36m";
constexpr const char* BOLD    = "\033[1m";
constexpr const char* BOLD_WHITE = "\033[1;37m";
} // namespace Colors

#endif // COLORS_H
