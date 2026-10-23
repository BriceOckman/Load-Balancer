/**
 * @file Request.h
 * @brief The Request struct: one unit of work for the load balancer simulation.
 * @author Brice Ockman
 */
#ifndef REQUEST_H
#define REQUEST_H

#include <string>

/**
 * @struct Request
 * @brief A single web request flowing through the simulation.
 * @details Holds the source and destination IPs, the processing time in
 * clock cycles, the job type ('P' processing or 'S' streaming), and the
 * cycle stamps used to compute wait and turnaround times.
 */
struct Request {
    unsigned long id = 0;          ///< Unique, increasing id assigned by the generator.
    std::string ipIn;              ///< Source (requester) IP, dotted quad.
    std::string ipOut;             ///< Destination IP, dotted quad.
    int time = 1;                  ///< Cycles required to process this request (>= 1).
    char jobType = 'P';            ///< 'P' = processing, 'S' = streaming.
    long arrivalCycle = 0;         ///< Cycle the request entered the queue.
    long startCycle = -1;          ///< Cycle a server began it; -1 until assigned.
};

/**
 * @brief Format a request for log lines, e.g. "#1042 [P] 192.168.4.7 -> 10.3.2.1 (37 cyc)".
 * @param r The request to format.
 * @return A short human-readable description.
 */
std::string toString(const Request& r);

#endif // REQUEST_H
