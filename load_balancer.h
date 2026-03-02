/**
 * @file load_balancer.h
 * @brief Declaration of the load_balancer class.
 *
 * @details The load_balancer manages a pool of dynamically allocated web servers
 * and a queue of pending requests. Each tick it assigns queued requests to idle
 * servers, checks for finished requests, and scales the server pool up or down
 * based on queue size relative to configurable thresholds.
 *
 * Two instances are created per simulation — one for processing ('P') requests
 * and one for streaming ('S') requests — each scaling independently.
 *
 * @author Your Name
 * @date 2025
 */

#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <iostream>
#include <vector>
#include <queue>
#include "request.h"
#include "web_server.h"

/// @brief Global output stream defined in main.cpp.
extern std::ostream* out;

/**
 * @brief Manages a pool of web servers and a queue of pending requests.
 *
 * @details Implements dynamic scaling logic:
 * - If queue size > scale_up_threshold * num_servers: add a server
 * - If queue size < scale_down_threshold * num_servers: remove a server
 * - Scaling actions are separated by scale_wait_cycles to avoid thrashing
 * - Server count never drops below 1
 *
 * Servers are dynamically allocated with `new` and deallocated with `delete`.
 * The destructor cleans up all remaining servers.
 */
class load_balancer {
    private:
        std::queue<request> request_queue; ///< Queue of pending requests waiting for a server
        std::vector<web_server*> servers;  ///< Dynamically allocated pool of web servers
        char job_type;                     ///< Job type this LB handles: 'P' or 'S'

        int scale_up_count;    ///< Total number of times a server was added
        int scale_down_count;  ///< Total number of times a server was removed
        int inactive_count;    ///< Total servers deallocated over the simulation
        int rejected_count;    ///< Requests rejected (reserved for future use)
        int total_requests;    ///< Total requests ever added to this load balancer

        int next_scale_check;      ///< Tick at which the next scaling check is allowed
        int scale_up_threshold;    ///< Queue multiplier to trigger scale up (default 80)
        int scale_down_threshold;  ///< Queue multiplier to trigger scale down (default 50)
        int scale_wait_cycles;     ///< Ticks to wait between scaling actions

    public:
        /**
         * @brief Default constructor. Creates a load balancer for job type 'P' with 10 servers.
         */
        load_balancer();

        /**
         * @brief Constructs a load balancer with specified job type, server count, and thresholds.
         * @param job_type Job type this load balancer handles ('P' or 'S').
         * @param num_servers Initial number of web servers to create.
         * @param scale_up_threshold Queue multiplier threshold to add a server.
         * @param scale_down_threshold Queue multiplier threshold to remove a server.
         * @param scale_wait_cycles Clock cycles to wait between scaling actions.
         */
        load_balancer(char job_type, int num_servers, int scale_up_threshold, int scale_down_threshold, int scale_wait_cycles);

        /**
         * @brief Adds a request to the pending queue.
         * @param new_request The request to enqueue.
         */
        void add_request(request new_request);

        /**
         * @brief Dynamically allocates a new web server and adds it to the pool.
         * @param curr_time Current clock tick (for logging).
         */
        void add_server(int curr_time);

        /**
         * @brief Removes and deallocates the first idle server found in the pool.
         * @param curr_time Current clock tick (for logging).
         * @throws std::out_of_range if all servers are busy and none can be removed.
         */
        void remove_server(int curr_time);

        /**
         * @brief Processes one clock tick: assigns requests, checks completions, scales if needed.
         * @details
         * Step 1: Assign queued requests to any idle servers.
         * Step 2: Call check_finished() on each server to update their state.
         * Step 3: Check queue size against thresholds and scale up or down if needed,
         *         respecting the scale_wait_cycles cooldown.
         * @param curr_time Current clock tick.
         */
        void process_tick(int curr_time);

        /// @brief Returns current number of pending requests in the queue.
        int get_queue_size() const     { return request_queue.size(); }
        /// @brief Returns current number of active servers in the pool.
        int get_server_count() const   { return servers.size(); }
        /// @brief Returns total number of scale-up events.
        int get_scale_ups() const      { return scale_up_count; }
        /// @brief Returns total number of scale-down events.
        int get_scale_downs() const    { return scale_down_count; }
        /// @brief Returns total number of servers deallocated.
        int get_inactive_count() const { return inactive_count; }
        /// @brief Returns total number of rejected requests.
        int get_rejected() const       { return rejected_count; }
        /// @brief Returns total requests ever added to this load balancer.
        int get_total_requests() const { return total_requests; }

        /**
         * @brief Destructor. Deallocates all remaining web servers in the pool.
         */
        ~load_balancer();
};

#endif