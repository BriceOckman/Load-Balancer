/**
 * @file Switch.cpp
 * @brief Implementation of the Switch class (bonus).
 * @author Brice Ockman
 */
#include "Switch.h"
#include <sstream>

Switch::Switch(const Config& cfg, Firewall* fw, Logger& logger_,
               int processingServers, int streamingServers)
    : firewall(fw), logger(logger_) {
    // The Switch filters once, so the LBs are created with firewall = nullptr.
    processingLB = new LoadBalancer("LB-Processing", cfg, nullptr, logger, processingServers);
    streamingLB = new LoadBalancer("LB-Streaming", cfg, nullptr, logger, streamingServers);
}

Switch::~Switch() {
    delete processingLB;
    delete streamingLB;
}

bool Switch::route(Request r) {
    if (firewall && firewall->isBlocked(r.ipIn)) {
        ++blocked;
        logger.event(EventType::BLOCKED, processingLB->getClock(), "Switch",
                     toString(r) + " from " + r.ipIn + " blocked by firewall");
        return false;
    }
    if (r.jobType == 'S') {
        ++routedS;
        return streamingLB->addRequest(r);
    }
    ++routedP;
    return processingLB->addRequest(r);
}

void Switch::step() {
    processingLB->step();
    streamingLB->step();
}

void Switch::printStartSummary() {
    processingLB->printStartSummary();
    streamingLB->printStartSummary();
}

void Switch::printEndSummary() {
    std::ostringstream oss;
    oss << "==================== SWITCH SUMMARY ====================\n"
        << "Routed P / S .............. " << routedP << " / " << routedS << "\n"
        << "Blocked at switch ......... " << blocked;
    logger.event(EventType::SWITCH, processingLB->getClock(), "Switch", oss.str());
    logger.summary(oss.str());
    processingLB->printEndSummary();
    streamingLB->printEndSummary();
}

long Switch::getRoutedP() const { return routedP; }
long Switch::getRoutedS() const { return routedS; }
LoadBalancer& Switch::processing() { return *processingLB; }
LoadBalancer& Switch::streaming() { return *streamingLB; }
