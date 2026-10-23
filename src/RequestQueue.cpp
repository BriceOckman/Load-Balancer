/**
 * @file RequestQueue.cpp
 * @brief Implementation of the FIFO RequestQueue.
 * @author Brice Ockman
 */
#include "RequestQueue.h"
#include <stdexcept>

void RequestQueue::push(const Request& r) {
    q.push(r);
    if (q.size() > peakSize) peakSize = q.size();
}

Request RequestQueue::pop() {
    if (q.empty()) throw std::out_of_range("RequestQueue::pop on empty queue");
    Request r = q.front();
    q.pop();
    return r;
}

const Request& RequestQueue::front() const {
    if (q.empty()) throw std::out_of_range("RequestQueue::front on empty queue");
    return q.front();
}

bool RequestQueue::empty() const { return q.empty(); }
std::size_t RequestQueue::size() const { return q.size(); }
std::size_t RequestQueue::getPeakSize() const { return peakSize; }
