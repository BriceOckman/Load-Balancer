/**
 * @file Stats.h
 * @brief The LBStats struct: running statistics collected by a LoadBalancer.
 * @author Brice Ockman
 */
#ifndef STATS_H
#define STATS_H

#include <cstddef>

/**
 * @struct LBStats
 * @brief Counters and samples gathered over a simulation run.
 * @details Derived values (averages, throughput, percent of cycles in the
 * target queue band) are computed from these fields in the end summary.
 */
struct LBStats {
    long generated = 0;        ///< Requests accepted into the queue.
    long blocked = 0;          ///< Requests dropped by the firewall.
    long processed = 0;        ///< Requests completed by any server.
    long processedP = 0;       ///< Completed processing ('P') requests.
    long processedS = 0;       ///< Completed streaming ('S') requests.
    long scaleUps = 0;         ///< Number of scale-up actions.
    long scaleDowns = 0;       ///< Number of scale-down actions.
    int startServers = 0;      ///< Servers at the start of the run.
    int minServers = 0;        ///< Smallest active server count observed.
    int maxServers = 0;        ///< Largest active server count observed.
    std::size_t startQueue = 0;///< Queue size at the start of the run.
    std::size_t minQueue = 0;  ///< Smallest queue size observed.
    std::size_t maxQueue = 0;  ///< Largest queue size observed.
    long double queueSum = 0;  ///< Sum of queue samples, for the average.
    long samples = 0;          ///< Number of per-cycle samples taken.
    long double totalWait = 0;       ///< Sum of (startCycle - arrivalCycle).
    long double totalTurnaround = 0; ///< Sum of (finishCycle - arrivalCycle).
    long cyclesInBand = 0;     ///< Cycles with low*servers <= queue <= high*servers.
    long cyclesBelow = 0;      ///< Cycles with queue below the band.
    long cyclesAbove = 0;      ///< Cycles with queue above the band.
};

#endif // STATS_H
