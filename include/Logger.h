/**
 * @file Logger.h
 * @brief The Logger class: colored console events and a plain-text log file.
 * @author Brice Ockman
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

/**
 * @enum EventType
 * @brief Categories of simulation events, each with its own color.
 * INFO is blue, SCALE_UP green, SCALE_DOWN yellow, BLOCKED red,
 * STATUS cyan, WARNING magenta, and SWITCH / summaries bold.
 */
enum class EventType { INFO, SCALE_UP, SCALE_DOWN, BLOCKED, STATUS, WARNING, SWITCH };

/**
 * @class Logger
 * @brief Writes simulation events to the console (colored) and a log file (plain).
 * @details In verbose mode every event is printed. In quiet mode events are
 * only counted and nothing is printed, while summaries are always shown in
 * both modes and both destinations.
 */
class Logger {
public:
    /**
     * @brief Construct a logger.
     * @param path Log file to open (truncated). If it cannot be opened, the
     *             logger writes to the console only.
     * @param verbose True to print events, false for summaries only.
     * @param useColor True to emit ANSI colors on the console.
     */
    Logger(const std::string& path, bool verbose, bool useColor);

    /**
     * @brief Record one simulation event.
     * @param t Event category (controls the color).
     * @param cycle Simulation cycle at which the event happened.
     * @param lbName Name of the load balancer reporting the event.
     * @param msg Human-readable event description.
     */
    void event(EventType t, long cycle, const std::string& lbName, const std::string& msg);

    /**
     * @brief Write a start/end summary block (always shown, both modes).
     * @param text The full summary text.
     */
    void summary(const std::string& text);

    /// @return True if this logger prints individual events.
    bool isVerbose() const;
    /// @return True if the log file was opened successfully.
    bool fileOk() const;

private:
    /// @brief Map an event type to its display name and console color.
    static const char* colorFor(EventType t);
    static const char* nameFor(EventType t);

    bool verbose;        ///< Print events when true.
    bool useColor;       ///< Emit ANSI colors when true.
    std::ofstream file;  ///< Plain-text log file.
};

#endif // LOGGER_H
