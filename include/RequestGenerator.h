/**
 * @file RequestGenerator.h
 * @brief The RequestGenerator class: creates random requests reproducibly.
 * @author Brice Ockman
 */
#ifndef REQUESTGENERATOR_H
#define REQUESTGENERATOR_H

#include "Config.h"
#include "Request.h"
#include <random>
#include <string>

/**
 * @class RequestGenerator
 * @brief Generates random requests and random arrival bursts.
 * @details Uses std::mt19937 seeded from the config ("seed"; 0 means a
 * random seed each run), so a fixed seed reproduces a run exactly. With
 * probability "attack_probability" a request's source IP is drawn from the
 * configured "attack_range" instead of uniformly, which is how the firewall
 * feature is demonstrated.
 */
class RequestGenerator {
public:
    /**
     * @brief Construct a generator from the simulation config.
     * @param cfg Loaded configuration (seed, time range, ratios, attack range).
     */
    explicit RequestGenerator(const Config& cfg);

    /**
     * @brief Generate one random request stamped with the given cycle.
     * @param cycle The current simulation cycle (its arrival cycle).
     * @return A new request with a fresh, increasing id.
     */
    Request generate(long cycle);

    /**
     * @brief How many new requests arrive in one cycle.
     * @return With probability arrival_probability, a uniform count in
     *         [1, max_burst]; otherwise 0.
     */
    int arrivalsThisCycle();

    /// @return A uniformly random dotted-quad IP.
    std::string randomIP();

    /// @return A random IP inside the configured attack_range.
    std::string attackIP();

private:
    std::mt19937 rng;             ///< Mersenne Twister engine (seeded from config).
    const Config& cfg;            ///< Simulation settings.
    unsigned long nextId = 1;     ///< Id to assign to the next request.
};

#endif // REQUESTGENERATOR_H
