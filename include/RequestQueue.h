/**
 * @file RequestQueue.h
 * @brief A FIFO queue of Requests wrapping std::queue, with peak tracking.
 * @author Brice Ockman
 */
#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#include "Request.h"
#include <cstddef>
#include <queue>

/**
 * @class RequestQueue
 * @brief First-in, first-out queue of waiting requests.
 * @details Wraps std::queue<Request> (stored by value) and remembers the
 * largest size ever observed so the summary can report the peak load.
 */
class RequestQueue {
public:
    /**
     * @brief Add a request to the back of the queue.
     * @param r The request to enqueue.
     */
    void push(const Request& r);

    /**
     * @brief Remove and return the request at the front of the queue.
     * @return The oldest queued request.
     * @throws std::out_of_range if the queue is empty.
     */
    Request pop();

    /**
     * @brief Peek at the request at the front without removing it.
     * @return A const reference to the front request.
     * @throws std::out_of_range if the queue is empty.
     */
    const Request& front() const;

    /// @return True if no requests are waiting.
    bool empty() const;

    /// @return The number of waiting requests.
    std::size_t size() const;

    /// @return The largest queue size ever observed.
    std::size_t getPeakSize() const;

private:
    std::queue<Request> q;       ///< Underlying FIFO storage.
    std::size_t peakSize = 0;    ///< High-water mark of size().
};

#endif // REQUESTQUEUE_H
