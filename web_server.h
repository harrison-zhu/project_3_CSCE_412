/**
 * @file web_server.h
 * @brief Declaration of the web_server class.
 *
 * @details A web_server represents a single server instance managed by a
 * load balancer. Each server processes one request at a time. Servers are
 * dynamically allocated and deallocated by the load balancer in response
 * to queue size changes.
 *
 * A static counter tracks the total number of servers ever created across
 * all load balancers, giving each server a unique ID for logging.
 *
 * @author Your Name
 * @date 2025
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <iostream>
#include "request.h"

/// @brief Global output stream defined in main.cpp.
extern std::ostream* out;

/**
 * @brief Represents a single web server in the load balancer simulation.
 *
 * @details Each web server holds one request at a time and tracks when that
 * request will finish based on the current clock tick and the request's
 * processing time. The load balancer calls check_finished() each tick to
 * determine if the server is free to accept a new request.
 *
 * Servers are dynamically allocated (new) when added and deallocated (delete)
 * when removed by the load balancer.
 */
class web_server {
    private:
        request curr_request;      ///< The request currently being processed
        bool running;              ///< Whether the server is currently processing a request
        int start_time;            ///< Clock tick when the current request started
        int end_time;              ///< Clock tick when the current request will finish
        int server_number;         ///< Unique ID assigned at creation
        char job_type;             ///< Job type of the load balancer this server belongs to
        static int tot_servers;    ///< Total servers ever created (used for unique IDs)

    public:
        /**
         * @brief Default constructor. Creates an idle server for load balancer 'P'.
         */
        web_server();

        /**
         * @brief Constructs an idle server assigned to a specific load balancer.
         * @param curr_time Current clock tick (0 during init, >0 if added during simulation).
         * @param job_type Job type of the owning load balancer ('P' or 'S').
         */
        web_server(int curr_time, char job_type);

        /**
         * @brief Constructs a server and immediately begins processing a request.
         * @param curr_request The request to begin processing.
         * @param curr_time Current clock tick.
         * @param job_type Job type of the owning load balancer ('P' or 'S').
         */
        web_server(request curr_request, int curr_time, char job_type);

        /**
         * @brief Checks whether the server has finished processing its current request.
         * @details Sets running to false and logs completion if curr_time >= end_time.
         * @param curr_time Current clock tick.
         * @return true if the server is idle (not running), false if still processing.
         */
        bool check_finished(int curr_time);

        /**
         * @brief Assigns a new request to this server and begins processing.
         * @param curr_request The request to process.
         * @param curr_time Current clock tick.
         * @throws std::out_of_range if the server is already processing a request.
         */
        void add_request(request curr_request, int curr_time);

        /// @brief Returns whether the server is currently processing a request.
        bool is_running() const { return running; }

        /// @brief Returns the unique server ID number.
        int get_server_number() const { return server_number; }
};

#endif