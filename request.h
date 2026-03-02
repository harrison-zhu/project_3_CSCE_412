/**
 * @file request.h
 * @brief Declaration of the request class.
 *
 * @details A request represents a single network request entering the load
 * balancer system. It holds the source and destination IP addresses, the
 * number of clock ticks required to process it, and its job type which
 * determines which load balancer it is routed to.
 *
 * @author Your Name
 * @date 2025
 */

#ifndef REQUEST_H
#define REQUEST_H

#include <string>

/**
 * @brief Represents a single network request in the load balancer simulation.
 *
 * @details Holds all metadata for a request as it moves through the system:
 * from generation in main, through the switch router firewall check,
 * into a load balancer queue, and finally onto a web server for processing.
 */
class request {
    private:
        std::string IP_in;   ///< Source IP address of the request
        std::string IP_out;  ///< Destination IP address of the request
        int time;            ///< Number of clock ticks required to process this request
        char job_type;       ///< Job type: 'P' for processing, 'S' for streaming

    public:
        /**
         * @brief Default constructor. Initializes an empty request with job type 'P'.
         */
        request();

        /**
         * @brief Constructs a request with all fields specified.
         * @param IP_in Source IP address.
         * @param IP_out Destination IP address.
         * @param time Number of clock ticks to process this request.
         * @param job_type Job type: 'P' for processing, 'S' for streaming.
         */
        request(std::string IP_in, std::string IP_out, int time, char job_type);

        /// @brief Returns the source IP address.
        std::string get_IP_in();

        /// @brief Returns the destination IP address.
        std::string get_IP_out();

        /// @brief Returns the number of clock ticks required to process this request.
        int get_time();

        /// @brief Returns the job type ('P' or 'S').
        char get_job_type();
};

#endif