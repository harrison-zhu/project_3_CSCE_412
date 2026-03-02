/**
 * @file request.cpp
 * @brief Implementation of the request class.
 *
 * @details Provides constructors and getter methods for the request class.
 * Requests are simple data-holding objects and have no side effects.
 *
 * @author Your Name
 * @date 2025
 */

#include "request.h"

/**
 * @brief Default constructor. Initializes an empty request with job type 'P'.
 */
request::request(): IP_in(), IP_out(), time(), job_type('P') {};

/**
 * @brief Constructs a request with all fields specified.
 * @param IP_in Source IP address.
 * @param IP_out Destination IP address.
 * @param time Number of clock ticks to process this request.
 * @param job_type Job type: 'P' for processing, 'S' for streaming.
 */
request::request(std::string IP_in, std::string IP_out, int time, char job_type) {
    this->IP_in = IP_in;
    this->IP_out = IP_out;
    this->time = time;
    this->job_type = job_type;
}

/**
 * @brief Returns the source IP address.
 * @return Source IP as a string.
 */
std::string request::get_IP_in() {
    return IP_in;
}

/**
 * @brief Returns the destination IP address.
 * @return Destination IP as a string.
 */
std::string request::get_IP_out() {
    return IP_out;
}

/**
 * @brief Returns the number of clock ticks required to process this request.
 * @return Processing time in clock ticks.
 */
int request::get_time() {
    return time;
}

/**
 * @brief Returns the job type of this request.
 * @return 'P' for processing, 'S' for streaming.
 */
char request::get_job_type() {
    return job_type;
}