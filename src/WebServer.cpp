/**
 * @file WebServer.cpp
 * @brief Implementation of the WebServer class.
 * @author Brice Ockman
 */
#include "WebServer.h"
#include <algorithm>

WebServer::WebServer(int id_, long createdCycle_) : id(id_), createdCycle(createdCycle_) {}

void WebServer::assign(const Request& r, long cycle) {
    current = r;
    current.startCycle = cycle;
    remaining = r.time;
    busy = true;
}

bool WebServer::tick() {
    if (!busy) return false;
    ++busyCycles;
    if (--remaining <= 0) {
        busy = false;
        ++completed;
        return true; // finished; asking for another request
    }
    return false;
}

bool WebServer::isBusy() const { return busy; }
bool WebServer::isDraining() const { return draining; }
void WebServer::setDraining(bool b) { draining = b; }
int WebServer::getId() const { return id; }
const Request& WebServer::getCurrent() const { return current; }
int WebServer::getRemaining() const { return remaining; }
long WebServer::getCompleted() const { return completed; }

double WebServer::getUtilization(long now) const {
    long alive = std::max(1L, now - createdCycle);
    return static_cast<double>(busyCycles) / static_cast<double>(alive);
}
