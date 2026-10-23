/**
 * @file WebServer.h
 * @brief The WebServer class: processes one request at a time.
 * @author Brice Ockman
 */
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Request.h"

/**
 * @class WebServer
 * @brief A simulated web server in the load balancer's pool.
 * @details A server is idle or busy. When assigned a request it counts down
 * the request's remaining cycles once per tick(); when the count reaches
 * zero the request is complete and the server asks for another. A server
 * marked draining finishes its current request, takes no new work, and is
 * then deallocated by the load balancer.
 */
class WebServer {
public:
    /**
     * @brief Construct an idle server.
     * @param id Unique server id within its load balancer.
     * @param createdCycle Simulation cycle at which the server was created.
     */
    WebServer(int id, long createdCycle);

    /**
     * @brief Assign a request to this (idle, non-draining) server.
     * @param r The request to process.
     * @param cycle The current simulation cycle (recorded as the start cycle).
     */
    void assign(const Request& r, long cycle);

    /**
     * @brief Advance the server by one clock cycle.
     * @return True exactly on the cycle a request finishes ("asking for another").
     */
    bool tick();

    /// @return True while a request is being processed.
    bool isBusy() const;
    /// @return True if marked for removal once its current request finishes.
    bool isDraining() const;
    /// @param b Set or clear the draining flag.
    void setDraining(bool b);
    /// @return This server's id.
    int getId() const;
    /// @return The request currently assigned (valid while busy).
    const Request& getCurrent() const;
    /// @return How many cycles remain on the current request (0 if idle).
    int getRemaining() const;
    /// @return The number of requests this server has completed.
    long getCompleted() const;
    /**
     * @brief Fraction of this server's lifetime spent busy.
     * @param now The current simulation cycle.
     * @return busyCycles / max(1, now - createdCycle), in [0, 1].
     */
    double getUtilization(long now) const;

private:
    int id;              ///< Unique server id.
    Request current;     ///< The request being processed (if busy).
    int remaining = 0;   ///< Cycles left on the current request.
    bool busy = false;   ///< True while processing.
    bool draining = false; ///< True when marked for removal.
    long createdCycle;   ///< Cycle this server was allocated.
    long busyCycles = 0; ///< Total cycles spent processing.
    long completed = 0;  ///< Requests finished by this server.
};

#endif // WEBSERVER_H
