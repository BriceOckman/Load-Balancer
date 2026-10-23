/**
 * @file Firewall.h
 * @brief The Firewall class: blocks requests from configured IP ranges.
 * @author Brice Ockman
 */
#ifndef FIREWALL_H
#define FIREWALL_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * @struct IPRange
 * @brief An inclusive range of IPv4 addresses stored as 32-bit integers.
 */
struct IPRange {
    std::uint32_t start = 0; ///< First blocked address (inclusive).
    std::uint32_t end = 0;   ///< Last blocked address (inclusive).
};

/**
 * @class Firewall
 * @brief An IP range blocker used for firewall / DoS prevention.
 * @details Rules are added as "A.B.C.D-E.F.G.H" ranges or CIDR blocks such
 * as "192.168.0.0/16". IPs are compared as 32-bit unsigned integers, so a
 * check is a few integer comparisons per rule.
 */
class Firewall {
public:
    /**
     * @brief Convert a dotted-quad IPv4 string to a 32-bit integer.
     * @param ip Address such as "192.168.4.7".
     * @return (a<<24)|(b<<16)|(c<<8)|d for "a.b.c.d".
     * @throws std::invalid_argument if the input is not four octets in 0..255.
     */
    static std::uint32_t ipToUint(const std::string& ip);

    /**
     * @brief Convert a 32-bit integer back to a dotted-quad string.
     * @param v The packed address.
     * @return The dotted-quad form.
     */
    static std::string uintToIp(std::uint32_t v);

    /**
     * @brief Parse and add a rule given as "start-end" or CIDR "a.b.c.d/prefix".
     * @param spec The rule text.
     * @return False (leaving the rules unchanged) if the spec is invalid.
     */
    bool addRange(const std::string& spec);

    /**
     * @brief Add a rule from packed integer bounds (swapped if out of order).
     * @param start First address in the range.
     * @param end Last address in the range.
     */
    void addRange(std::uint32_t start, std::uint32_t end);

    /**
     * @brief Check whether an IP falls in any blocked range (inclusive).
     * @param ip Dotted-quad source IP.
     * @return True if blocked. As a side effect, increments the blocked count.
     */
    bool isBlocked(const std::string& ip);

    /// @return How many isBlocked() checks have returned true.
    long getBlockedCount() const;

    /// @return The loaded rules, for summaries.
    const std::vector<IPRange>& getRanges() const;

private:
    std::vector<IPRange> ranges; ///< Blocked ranges.
    long blockedCount = 0;       ///< Running count of blocked requests.
};

#endif // FIREWALL_H
