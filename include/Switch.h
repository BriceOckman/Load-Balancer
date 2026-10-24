/**
 * @file Switch.h
 * @brief The Switch class (bonus): routes requests by job type to two LBs.
 * @author Brice Ockman
 */
#ifndef SWITCH_H
#define SWITCH_H

#include "Config.h"
#include "Firewall.h"
#include "Logger.h"
#include "Request.h"
#include "LoadBalancer.h"
#include <string>

/**
 * @class Switch
 * @brief Routes streaming and processing jobs to separate load balancers.
 * @details The Switch owns two LoadBalancer instances ("LB-Processing" and
 * "LB-Streaming"), each with its own queue, server pool, scaling decisions,
 * and statistics. It checks the firewall once at the entry point, then
 * forwards each request by jobType: 'P' to the processing LB and 'S' to the
 * streaming LB. step() advances both load balancers in the same cycle, so
 * they run concurrently in lockstep simulated time.
 */
class Switch {
public:
    /**
     * @brief Construct a switch with two load balancers.
     * @param cfg Simulation config.
     * @param fw Shared firewall, checked once per routed request.
     * @param logger Shared logger.
     * @param processingServers Initial servers in the processing LB.
     * @param streamingServers Initial servers in the streaming LB.
     */
    Switch(const Config& cfg, Firewall* fw, Logger& logger,
           int processingServers, int streamingServers);

    /// @brief Delete both owned load balancers.
    ~Switch();

    Switch(const Switch&) = delete;
    Switch& operator=(const Switch&) = delete;

    /**
     * @brief Route one request through the firewall to the LB for its job type.
     * @param r The request to route.
     * @return True if routed (queued), false if blocked by the firewall.
     */
    bool route(Request r);

    /// @brief Advance both load balancers by one clock cycle.
    void step();

    /// @brief Print start summaries for both load balancers.
    void printStartSummary();
    /// @brief Print routing totals, then each LB's end summary.
    void printEndSummary();

    /// @return How many 'P' requests were routed.
    long getRoutedP() const;
    /// @return How many 'S' requests were routed.
    long getRoutedS() const;
    /// @return The processing load balancer.
    LoadBalancer& processing();
    /// @return The streaming load balancer.
    LoadBalancer& streaming();

private:
    LoadBalancer* processingLB; ///< Owns the 'P' pool.
    LoadBalancer* streamingLB;  ///< Owns the 'S' pool.
    Firewall* firewall;         ///< Entry-point firewall (may be nullptr).
    Logger& logger;             ///< Shared logger.
    long routedP = 0;           ///< Count of routed processing requests.
    long routedS = 0;           ///< Count of routed streaming requests.
    long blocked = 0;           ///< Count blocked at the switch.
};

#endif // SWITCH_H
