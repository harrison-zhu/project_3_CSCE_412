/**
 * @file load_balancer.cpp
 * @brief Implementation of the load_balancer class.
 *
 * @details Handles dynamic server pool management, request queuing,
 * tick-based processing, and automatic scaling based on queue size thresholds.
 * Servers are heap-allocated and freed as the simulation scales up and down.
 *
 * @author Your Name
 * @date 2025
 */

#include <iostream>
#include <stdexcept>
#include "load_balancer.h"

// colors for logging
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"
#define WHITE   "\033[37m"

/**
 * @brief Default constructor. Creates a load balancer for job type 'P' with 10 servers
 *        and default scaling thresholds (80 up, 50 down, 50 wait cycles).
 */
load_balancer::load_balancer()
    : job_type('P'), scale_up_count(0), scale_down_count(0), inactive_count(0),
      rejected_count(0), total_requests(0), next_scale_check(50),
      scale_up_threshold(80), scale_down_threshold(50), scale_wait_cycles(50) {
    for (size_t i = 0; i < 10; i++) {
        servers.push_back(new web_server(0, 'P'));
    }
    *out << GREEN << "[INIT] Load balancer created for job type P with 10 servers." << RESET << std::endl;
};

/**
 * @brief Constructs a load balancer with specified job type, server count, and thresholds.
 * @param job_type Job type this load balancer handles ('P' or 'S').
 * @param num_servers Initial number of web servers to create.
 * @param scale_up_threshold Queue multiplier threshold to add a server.
 * @param scale_down_threshold Queue multiplier threshold to remove a server.
 * @param scale_wait_cycles Clock cycles to wait between scaling actions.
 */
load_balancer::load_balancer(char job_type, int num_servers, int scale_up_threshold, int scale_down_threshold, int scale_wait_cycles)
    : scale_up_count(0), scale_down_count(0), inactive_count(0), rejected_count(0),
      total_requests(0), next_scale_check(scale_wait_cycles),
      scale_up_threshold(scale_up_threshold), scale_down_threshold(scale_down_threshold),
      scale_wait_cycles(scale_wait_cycles) {
    this->job_type = job_type;
    for (size_t i = 0; i < num_servers; i++) {
        servers.push_back(new web_server(0, job_type));
    }
    *out << GREEN << "[INIT] Load balancer created for job type "
         << job_type << " with " << num_servers << " servers." << RESET << std::endl;
};

/**
 * @brief Adds a request to the pending queue.
 * @param new_request The request to enqueue.
 */
void load_balancer::add_request(request new_request) {
    request_queue.push(new_request);
    total_requests++;
};

/**
 * @brief Dynamically allocates a new web server and adds it to the pool.
 * @param curr_time Current clock tick (for logging).
 */
void load_balancer::add_server(int curr_time) {
    servers.push_back(new web_server(curr_time, job_type));
    scale_up_count++;
};

/**
 * @brief Removes and deallocates the first idle server found in the pool.
 * @param curr_time Current clock tick (for logging).
 * @throws std::out_of_range if all servers are busy and none can be removed.
 */
void load_balancer::remove_server(int curr_time) {
    for (size_t i = 0; i < servers.size(); i++) {
        if (servers.at(i)->check_finished(curr_time)) {
            delete servers.at(i);
            servers.erase(servers.begin() + i);
            scale_down_count++;
            inactive_count++;
            return;
        }
    }
    throw std::out_of_range("All servers busy, cannot remove server to scale down.");
};

/**
 * @brief Processes one clock tick for this load balancer.
 * @details
 * - Step 1: Assign queued requests to any idle servers.
 * - Step 2: Call check_finished() on all servers to update running state.
 * - Step 3: If the scaling cooldown has passed, check queue size thresholds
 *           and add or remove a server accordingly. Never reduces below 1 server.
 * @param curr_time Current clock tick.
 */
void load_balancer::process_tick(int curr_time) {
    // step 1: assign requests to idle servers
    for (size_t i = 0; i < servers.size(); i++) {
        if (!servers.at(i)->is_running() && !request_queue.empty()) {
            request next = request_queue.front();
            request_queue.pop();
            servers.at(i)->add_request(next, curr_time);
        }
    }

    // step 2: tick each server (check if finished)
    for (size_t i = 0; i < servers.size(); i++) {
        servers.at(i)->check_finished(curr_time);
    }

    // step 3: check scaling, but only after wait period has passed
    if (curr_time < next_scale_check) return;

    int queue_size = request_queue.size();
    int num_servers = servers.size();

    // scale up if queue is too large
    if (queue_size > scale_up_threshold * num_servers) {
        add_server(curr_time);
        next_scale_check = curr_time + scale_wait_cycles;
        *out << CYAN << "[TICK " << curr_time << "] [" << job_type << "] Queue too large ("
             << queue_size << " > " << scale_up_threshold * num_servers
             << "). Added server. Total servers: " << servers.size() << RESET << std::endl;
    }
    // scale down if queue is too small, but never go below 1 server
    else if (queue_size < scale_down_threshold * num_servers && num_servers > 1) {
        try {
            remove_server(curr_time);
            next_scale_check = curr_time + scale_wait_cycles;
            *out << RED << "[TICK " << curr_time << "] [" << job_type << "] Queue too small ("
                 << queue_size << " < " << scale_down_threshold * num_servers
                 << "). Removed server. Total servers: " << servers.size() << RESET << std::endl;
        } catch (std::out_of_range& e) {
            *out << RED << "[TICK " << curr_time << "] [" << job_type << "] Scale down skipped: " << e.what() << RESET << std::endl;
        }
    }
};

/**
 * @brief Destructor. Deallocates all remaining web servers in the pool.
 */
load_balancer::~load_balancer() {
    for (size_t i = 0; i < servers.size(); i++) {
        delete servers.at(i);
    }
    servers.clear();
}