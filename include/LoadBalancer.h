/**
 * @file LoadBalancer.h
 * @brief The LoadBalancer class: a request queue plus a dynamic server pool.
 * @author Brice Ockman
 */
#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "Config.h"
#include "Firewall.h"
#include "Logger.h"
#include "Request.h"
#include "RequestQueue.h"
#include "Stats.h"
#include "WebServer.h"
#include <cstddef>
#include <string>
#include <vector>

/**
 * @class LoadBalancer
 * @brief Distributes queued requests across a pool of web servers over time.
 * @details Owns one RequestQueue by value and a dynamic set of WebServer
 * objects through raw pointers: servers are allocated with new on scale-up
 * and deleted on scale-down (or in the destructor), satisfying the dynamic
 * allocation requirement with no leaks. Each step() advances the simulation
 * by one clock cycle: tick servers, reap drained servers, dispatch queued
 * requests to idle servers, then evaluate dynamic scaling. The scaling rule
 * keeps the queue between scale_low_ratio and scale_high_ratio times the
 * active server count, with a cooldown of scale_cooldown cycles between
 * actions. Multiple instances can run at once (see Switch).
 */
class LoadBalancer {
public:
    /**
     * @brief Construct a load balancer with an initial server pool.
     * @param name Display name used in log lines and summaries.
     * @param cfg Simulation config (scaling thresholds, cooldown, min/max).
     * @param fw Firewall to screen incoming requests, or nullptr if the
     *           caller (e.g. Switch) already filtered them.
     * @param logger Shared logger for events and summaries.
     * @param initialServers Number of servers to allocate at startup.
     */
    LoadBalancer(const std::string& name, const Config& cfg, Firewall* fw,
                 Logger& logger, int initialServers);

    /// @brief Delete every remaining WebServer (no memory leaks).
    ~LoadBalancer();

    LoadBalancer(const LoadBalancer&) = delete;
    LoadBalancer& operator=(const LoadBalancer&) = delete;

    /**
     * @brief Offer a request to this load balancer.
     * @param r The request (its arrival cycle is set to the current clock).
     * @return True if queued; false if blocked by the firewall.
     */
    bool addRequest(Request r);

    /// @brief Advance the simulation by one clock cycle.
    void step();

    /// @return The current simulation cycle.
    long getClock() const;
    /// @return The number of requests currently waiting.
    std::size_t getQueueSize() const;
    /// @return Servers that can accept work (not draining).
    int getActiveServerCount() const;
    /// @return All servers, including draining ones.
    int getServerCount() const;
    /// @return Requests still being processed at this moment.
    int getInServiceCount() const;
    /// @return Statistics collected so far.
    const LBStats& getStats() const;
    /// @return This load balancer's display name.
    const std::string& getName() const;

    /// @brief Print and log the starting summary for this load balancer.
    void printStartSummary();
    /// @brief Print and log the ending summary for this load balancer.
    void printEndSummary();

private:
    void processServers();   ///< Tick every server; record completions.
    void reapDrained();      ///< Delete draining servers that are now idle.
    void dispatch();         ///< Assign queued requests to idle servers.
    void evaluateScaling();  ///< Add/remove one server if the band is violated.
    void addServer();        ///< Allocate one new server.
    void removeServer();     ///< Delete an idle server, or mark one draining.
    void recordSample();     ///< Update queue/server samples and band counters.

    std::string name;                    ///< Display name.
    RequestQueue queue;                  ///< Waiting requests (composed).
    std::vector<WebServer*> servers;     ///< Owned server pointers.
    long clock = 0;                      ///< Current simulation cycle.
    int cooldownLeft = 0;                ///< Cycles until scaling is re-evaluated.
    int nextServerId = 1;                ///< Id for the next allocated server.
    std::size_t dispatchStart = 0;       ///< Rotating dispatch start index.
    const Config& cfg;                   ///< Simulation settings.
    Firewall* firewall;                  ///< Shared firewall, or nullptr.
    Logger& logger;                      ///< Shared logger.
    LBStats stats;                       ///< Running statistics.
    int lowRatio;                        ///< scale_low_ratio from config.
    int highRatio;                       ///< scale_high_ratio from config.
    int cooldownCfg;                     ///< scale_cooldown from config.
    int minServersCfg;                   ///< min_servers from config.
    int maxServersCfg;                   ///< max_servers from config.
    int statusInterval;                  ///< Cycles between STATUS log lines.
    bool warnedEmpty = false;            ///< Avoid repeating the empty-queue warning.
};

#endif // LOADBALANCER_H
