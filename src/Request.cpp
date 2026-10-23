/**
 * @file Request.cpp
 * @brief Implementation of the Request formatting helper.
 * @author Brice Ockman
 */
#include "Request.h"

std::string toString(const Request& r) {
    return "#" + std::to_string(r.id) + " [" + std::string(1, r.jobType) + "] " +
           r.ipIn + " -> " + r.ipOut + " (" + std::to_string(r.time) + " cyc)";
}
