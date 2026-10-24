/**
 * @file LoadBalancer.cpp
 * @brief Implementation of the LoadBalancer class.
 * @author Brice Ockman
 */
#include "LoadBalancer.h"
#include <algorithm>
#include <sstream>

LoadBalancer::LoadBalancer(const std::string& name_, const Config& cfg_, Firewall* fw,
                           Logger& logger_, int initialServers)
    : name(name_), cfg(cfg_), firewall(fw), logger(logger_),
      lowRatio(cfg_.getInt("scale_low_ratio", 50)),
      highRatio(cfg_.getInt("scale_high_ratio", 80)),
      cooldownCfg(cfg_.getInt("scale_cooldown", 50)),
      minServersCfg(cfg_.getInt("min_servers", 1)),
      maxServersCfg(cfg_.getInt("max_servers", 1000)),
      statusInterval(cfg_.getInt("status_interval", 1000)) {
    if (initialServers < 1) initialServers = 1;
    for (int i = 0; i < initialServers; ++i)
        servers.push_back(new WebServer(nextServerId++, 0));
    stats.startServers = initialServers;
    stats.minServers = initialServers;
    stats.maxServers = initialServers;
    stats.minQueue = static_cast<std::size_t>(-1);
}

LoadBalancer::~LoadBalancer() {
    for (WebServer* s : servers) delete s;
    servers.clear();
}

bool LoadBalancer::addRequest(Request r) {
    r.arrivalCycle = clock;
    if (firewall && firewall->isBlocked(r.ipIn)) {
        ++stats.blocked;
        logger.event(EventType::BLOCKED, clock, name,
                     toString(r) + " from " + r.ipIn + " blocked by firewall");
        return false;
    }
    ++stats.generated;
    queue.push(r);
    return true;
}

void LoadBalancer::processServers() {
    for (WebServer* s : servers) {
        if (!s->isBusy()) continue;
        Request cur = s->getCurrent(); // capture before tick() may finish it
        if (s->tick()) {
            ++stats.processed;
            if (cur.jobType == 'S') ++stats.processedS; else ++stats.processedP;
            stats.totalWait += static_cast<long double>(cur.startCycle - cur.arrivalCycle);
            stats.totalTurnaround += static_cast<long double>(clock + 1 - cur.arrivalCycle);
        }
    }
}

void LoadBalancer::reapDrained() {
    for (auto it = servers.begin(); it != servers.end();) {
        if ((*it)->isDraining() && !(*it)->isBusy()) {
            delete *it;
            it = servers.erase(it);
        } else {
            ++it;
        }
    }
}

void LoadBalancer::dispatch() {
    if (servers.empty()) return;
    std::size_t n = servers.size();
    for (std::size_t k = 0; k < n && !queue.empty(); ++k) {
        WebServer* s = servers[(dispatchStart + k) % n];
        if (!s->isBusy() && !s->isDraining()) s->assign(queue.pop(), clock);
    }
    dispatchStart = (dispatchStart + 1) % n; // rotate: do not favor low ids
    if (queue.empty() && getActiveServerCount() > 0 && !warnedEmpty) {
        // Normal condition, noted once per empty streak in verbose mode.
        warnedEmpty = true;
    } else if (!queue.empty()) {
        warnedEmpty = false;
    }
}

void LoadBalancer::addServer() {
    servers.push_back(new WebServer(nextServerId++, clock));
    ++stats.scaleUps;
    std::ostringstream oss;
    oss << "servers " << (getActiveServerCount() - 1) << " -> " << getActiveServerCount()
        << " (queue " << queue.size() << " > " << highRatio << " x servers)";
    logger.event(EventType::SCALE_UP, clock, name, oss.str());
}

void LoadBalancer::removeServer() {
    // Prefer removing an idle server outright.
    for (auto it = servers.begin(); it != servers.end(); ++it) {
        if (!(*it)->isBusy() && !(*it)->isDraining()) {
            int id = (*it)->getId();
            delete *it;
            servers.erase(it);
            ++stats.scaleDowns;
            std::ostringstream oss;
            oss << "servers -> " << getActiveServerCount() << " (removed idle server #"
                << id << ", queue " << queue.size() << " < " << lowRatio << " x servers)";
            logger.event(EventType::SCALE_DOWN, clock, name, oss.str());
            return;
        }
    }
    // All busy: drain the server closest to finishing; it is deleted when idle.
    WebServer* best = nullptr;
    for (WebServer* s : servers)
        if (!s->isDraining() && (!best || s->getRemaining() < best->getRemaining())) best = s;
    if (best) {
        best->setDraining(true);
        ++stats.scaleDowns;
        std::ostringstream oss;
        oss << "server #" << best->getId() << " draining (queue " << queue.size()
            << " < " << lowRatio << " x servers)";
        logger.event(EventType::SCALE_DOWN, clock, name, oss.str());
    }
}

void LoadBalancer::evaluateScaling() {
    if (cooldownLeft > 0) { --cooldownLeft; return; }
    int s = getActiveServerCount();
    std::size_t q = queue.size();
    if (s > 0 && q > static_cast<std::size_t>(highRatio) * static_cast<std::size_t>(s) &&
        s < maxServersCfg) {
        addServer();
        cooldownLeft = cooldownCfg;
    } else if (s > 0 && q < static_cast<std::size_t>(lowRatio) * static_cast<std::size_t>(s) &&
               s > minServersCfg) {
        removeServer();
        cooldownLeft = cooldownCfg;
    }
}

void LoadBalancer::recordSample() {
    std::size_t q = queue.size();
    int s = getActiveServerCount();
    if (stats.samples == 0) stats.minQueue = q;
    stats.minQueue = std::min(stats.minQueue, q);
    stats.maxQueue = std::max(stats.maxQueue, q);
    stats.queueSum += static_cast<long double>(q);
    ++stats.samples;
    stats.minServers = std::min(stats.minServers, s);
    stats.maxServers = std::max(stats.maxServers, s);
    if (s > 0) {
        std::size_t lo = static_cast<std::size_t>(lowRatio) * static_cast<std::size_t>(s);
        std::size_t hi = static_cast<std::size_t>(highRatio) * static_cast<std::size_t>(s);
        if (q < lo) ++stats.cyclesBelow;
        else if (q > hi) ++stats.cyclesAbove;
        else ++stats.cyclesInBand;
    }
}

void LoadBalancer::step() {
    processServers();
    reapDrained();
    dispatch();
    evaluateScaling();
    recordSample();
    if (logger.isVerbose() && statusInterval > 0 && clock > 0 && clock % statusInterval == 0) {
        int busy = 0;
        for (WebServer* s : servers) if (s->isBusy()) ++busy;
        std::ostringstream oss;
        oss << "queue " << queue.size() << " | servers " << getActiveServerCount() << " ("
            << busy << " busy) | done " << stats.processed;
        logger.event(EventType::STATUS, clock, name, oss.str());
    }
    ++clock;
}

long LoadBalancer::getClock() const { return clock; }
std::size_t LoadBalancer::getQueueSize() const { return queue.size(); }

int LoadBalancer::getActiveServerCount() const {
    int n = 0;
    for (WebServer* s : servers) if (!s->isDraining()) ++n;
    return n;
}

int LoadBalancer::getServerCount() const { return static_cast<int>(servers.size()); }

int LoadBalancer::getInServiceCount() const {
    int n = 0;
    for (WebServer* s : servers) if (s->isBusy()) ++n;
    return n;
}

const LBStats& LoadBalancer::getStats() const { return stats; }
const std::string& LoadBalancer::getName() const { return name; }

void LoadBalancer::printStartSummary() {
    stats.startQueue = queue.size();
    std::ostringstream oss;
    oss << "==================== START SUMMARY: " << name << " ====================\n"
        << "Initial servers ........... " << stats.startServers << "\n"
        << "Initial queue size ........ " << stats.startQueue << "\n"
        << "Scaling band .............. " << lowRatio << " x - " << highRatio
        << " x servers, cooldown " << cooldownCfg << " cycles\n"
        << "Server limits ............. min " << minServersCfg << ", max " << maxServersCfg;
    logger.summary(oss.str());
}

void LoadBalancer::printEndSummary() {
    long cycles = (clock > 0) ? clock : 1;
    double avgQueue = stats.samples ? static_cast<double>(stats.queueSum / stats.samples) : 0.0;
    double avgWait = stats.processed ? static_cast<double>(stats.totalWait / stats.processed) : 0.0;
    double avgTurn = stats.processed ? static_cast<double>(stats.totalTurnaround / stats.processed) : 0.0;
    double throughput = static_cast<double>(stats.processed) / static_cast<double>(cycles);
    long totalCycles = stats.cyclesInBand + stats.cyclesBelow + stats.cyclesAbove;
    double pctIn = totalCycles ? 100.0 * stats.cyclesInBand / totalCycles : 0.0;
    double pctBelow = totalCycles ? 100.0 * stats.cyclesBelow / totalCycles : 0.0;
    double pctAbove = totalCycles ? 100.0 * stats.cyclesAbove / totalCycles : 0.0;
    const char* result = "BALANCED";
    if (pctIn < 70.0) result = (pctAbove >= pctBelow) ? "OVERLOADED" : "UNDERUTILIZED";

    std::ostringstream oss;
    oss.setf(std::ios::fixed); oss.precision(1);
    oss << "==================== END SUMMARY: " << name << " ====================\n"
        << "Cycles run ................ " << clock << "\n"
        << "Requests generated ........ " << stats.generated
        << "    blocked ......... " << stats.blocked << "\n"
        << "Requests processed ........ " << stats.processed << "    (P " << stats.processedP
        << " / S " << stats.processedS << ")\n"
        << "Remaining in queue ........ " << queue.size()
        << "     in service ...... " << getInServiceCount() << "\n"
        << "Queue size ................ start " << stats.startQueue << "  end " << queue.size()
        << "  min " << (stats.samples ? stats.minQueue : stats.startQueue)
        << "  max " << std::max(stats.maxQueue, queue.getPeakSize()) << "\n"
        << "Queue avg ................. " << avgQueue << "     in band " << pctIn
        << "% / below " << pctBelow << "% / above " << pctAbove << "%\n"
        << "Servers ................... start " << stats.startServers << "  end "
        << getActiveServerCount() << "  min " << stats.minServers << "  max " << stats.maxServers << "\n"
        << "Scale-ups / scale-downs ... " << stats.scaleUps << " / " << stats.scaleDowns << "\n"
        << "Avg wait / turnaround ..... " << avgWait << " / " << avgTurn << " cycles\n"
        << "Throughput ................ " << throughput << " requests/cycle\n"
        << "Result .................... " << result << "\n"
        << "===============================================================";
    logger.summary(oss.str());

    if (logger.isVerbose()) {
        std::ostringstream table;
        table << "Per-server results (" << name << "): id | completed | utilization %";
        for (WebServer* s : servers) {
            double util = 100.0 * s->getUtilization(clock);
            table << "\n  server " << s->getId() << " | " << s->getCompleted() << " | "
                  << util << "%";
        }
        logger.summary(table.str());
    }
}
