/**
 * @file web_server.cpp
 * @brief Implementation of the web_server class.
 *
 * @details Manages server lifecycle including creation, request assignment,
 * tick-based progress tracking, and completion detection. Each server
 * processes exactly one request at a time and logs all state changes.
 *
 * @author Your Name
 * @date 2025
 */

#include <iostream>
#include <stdexcept>
#include "web_server.h"

// colors for logging
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"
#define WHITE   "\033[37m"

/// @brief Static counter tracking total servers ever created for unique ID assignment.
int web_server::tot_servers = 0;

/**
 * @brief Default constructor. Creates an idle server for load balancer 'P'.
 */
web_server::web_server(): running(false), job_type('P') {
    this->tot_servers++;
    this->server_number = this->tot_servers;
    *out << GREEN << "[INIT] Created web server " << server_number << " for load balancer P" << RESET << std::endl;
};

/**
 * @brief Constructs an idle server assigned to a specific load balancer.
 * @param curr_time Current clock tick. If 0, logs as INIT; otherwise logs as TICK.
 * @param job_type Job type of the owning load balancer ('P' or 'S').
 */
web_server::web_server(int curr_time, char job_type) {
    this->running = false;
    this->job_type = job_type;
    this->tot_servers++;
    this->server_number = this->tot_servers;

    if (curr_time == 0) {
        *out << GREEN << "[INIT] Created web server " << server_number << " for load balancer " << job_type << RESET << std::endl;
    } else {
        *out << CYAN << "[TICK " << curr_time << "] Created web server " << server_number << " for load balancer " << job_type << RESET << std::endl;
    }
};

/**
 * @brief Constructs a server and immediately begins processing a request.
 * @param curr_request The request to begin processing.
 * @param curr_time Current clock tick.
 * @param job_type Job type of the owning load balancer ('P' or 'S').
 */
web_server::web_server(request curr_request, int curr_time, char job_type) {
    this->curr_request = curr_request;
    this->start_time = curr_time;
    this->end_time = curr_time + curr_request.get_time();
    this->running = true;
    this->job_type = job_type;
    this->tot_servers++;
    this->server_number = this->tot_servers;
    *out << CYAN << "[TICK " << curr_time << "] Created web server " << server_number << " for load balancer " << job_type << RESET << std::endl;
};

/**
 * @brief Checks whether the server has finished processing its current request.
 * @details If curr_time >= end_time and the server is running, marks it as idle
 * and logs the completion. Called every tick by the load balancer.
 * @param curr_time Current clock tick.
 * @return true if the server is idle, false if still processing.
 */
bool web_server::check_finished(int curr_time) {
    if (curr_time >= end_time && running) {
        running = false;
        *out << MAGENTA << "[TICK " << curr_time << "] Web server " << server_number
             << " [" << job_type << "] finished request." << RESET << std::endl;
    }
    return (!running);
}

/**
 * @brief Assigns a new request to this server and begins processing.
 * @param curr_request The request to process.
 * @param curr_time Current clock tick.
 * @throws std::out_of_range if the server is already processing a request.
 */
void web_server::add_request(request curr_request, int curr_time) {
    if (running == true) {
        throw std::out_of_range("Trying to add request when there is one running");
    }
    this->curr_request = curr_request;
    this->start_time = curr_time;
    this->end_time = curr_time + curr_request.get_time();
    this->running = true;
}