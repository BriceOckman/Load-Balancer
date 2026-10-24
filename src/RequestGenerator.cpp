/**
 * @file RequestGenerator.cpp
 * @brief Implementation of the RequestGenerator class.
 * @author Brice Ockman
 */
#include "RequestGenerator.h"
#include "Firewall.h"
#include <random>

RequestGenerator::RequestGenerator(const Config& cfg_) : cfg(cfg_) {
    int seed = cfg.getInt("seed", 42);
    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    } else {
        rng.seed(static_cast<unsigned>(seed));
    }
}

std::string RequestGenerator::randomIP() {
    std::uniform_int_distribution<int> octet(0, 255);
    return std::to_string(octet(rng)) + "." + std::to_string(octet(rng)) + "." +
           std::to_string(octet(rng)) + "." + std::to_string(octet(rng));
}

std::string RequestGenerator::attackIP() {
    std::string range = cfg.getString("attack_range", "10.0.0.0-10.0.255.255");
    auto dash = range.find('-');
    try {
        if (dash == std::string::npos) return range;
        std::uint32_t s = Firewall::ipToUint(range.substr(0, dash));
        std::uint32_t e = Firewall::ipToUint(range.substr(dash + 1));
        if (e < s) std::swap(s, e);
        std::uniform_int_distribution<std::uint32_t> pick(s, e);
        return Firewall::uintToIp(pick(rng));
    } catch (const std::exception&) {
        return randomIP();
    }
}

Request RequestGenerator::generate(long cycle) {
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    int minT = cfg.getInt("min_task_time", 10);
    int maxT = cfg.getInt("max_task_time", 100);
    if (minT > maxT) std::swap(minT, maxT);
    std::uniform_int_distribution<int> taskTime(minT, maxT);

    Request r;
    r.id = nextId++;
    double attackP = cfg.getDouble("attack_probability", 0.02);
    r.ipIn = (prob(rng) < attackP) ? attackIP() : randomIP();
    r.ipOut = randomIP();
    r.time = taskTime(rng);
    double streamRatio = cfg.getDouble("streaming_ratio", 0.30);
    r.jobType = (prob(rng) < streamRatio) ? 'S' : 'P';
    r.arrivalCycle = cycle;
    r.startCycle = -1;
    return r;
}

int RequestGenerator::arrivalsThisCycle() {
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    double p = cfg.getDouble("arrival_probability", 0.30);
    if (prob(rng) >= p) return 0;
    int maxBurst = cfg.getInt("max_burst", 3);
    if (maxBurst < 1) maxBurst = 1;
    std::uniform_int_distribution<int> burst(1, maxBurst);
    return burst(rng);
}
