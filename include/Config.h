/**
 * @file Config.h
 * @brief The Config class: key/value settings loaded from a config file.
 * @author Brice Ockman
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>
#include <vector>

/**
 * @class Config
 * @brief Simulation settings read from a text configuration file.
 * @details The file holds one "key = value" pair per line; '#' starts a
 * comment. Repeated "blocked_range" keys are collected into a list instead
 * of overwriting each other. Every getter takes a built-in default, so the
 * program runs sensibly even when the file or a key is missing.
 */
class Config {
public:
    /**
     * @brief Load settings from a file, replacing any previously loaded values.
     * @param path Path to the config file.
     * @return False if the file could not be opened (defaults still apply).
     */
    bool load(const std::string& path);

    /// @param key Setting name. @param def Fallback. @return Parsed int or def.
    int getInt(const std::string& key, int def) const;
    /// @param key Setting name. @param def Fallback. @return Parsed double or def.
    double getDouble(const std::string& key, double def) const;
    /// @param key Setting name. @param def Fallback. @return Parsed bool ("true"/"1") or def.
    bool getBool(const std::string& key, bool def) const;
    /// @param key Setting name. @param def Fallback. @return Stored string or def.
    std::string getString(const std::string& key, const std::string& def) const;

    /// @return All repeated "blocked_range" values, in file order.
    std::vector<std::string> getBlockedRanges() const;

    /**
     * @brief Set or override a value (used for CLI flags and user prompts).
     * @param key Setting name.
     * @param value New value, stored as text.
     */
    void set(const std::string& key, const std::string& value);

private:
    std::map<std::string, std::string> values; ///< Single-valued settings.
    std::vector<std::string> blockedRanges;    ///< Repeated blocked_range settings.
};

#endif // CONFIG_H
