// includes
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "switch.h"

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
 * @brief Converts a dotted decimal IP string to a 32-bit integer.
 * @param ip IP address string (e.g. "192.168.1.100").
 * @return 32-bit unsigned integer representation.
 */
uint32_t switch_router::ip_to_int(const std::string& ip) const {
    uint32_t result = 0;
    std::istringstream ss(ip);
    std::string octet;
    int shift = 24;
    while (std::getline(ss, octet, '.') && shift >= 0) {
        result |= (static_cast<uint32_t>(std::stoi(octet)) << shift);
        shift -= 8;
    }
    return result;
};

/**
 * @brief Checks if an IP address falls within any blocked range.
 * @param ip The IP address to check.
 * @return true if blocked, false if allowed.
 */
bool switch_router::is_blocked(const std::string& ip) const {
    uint32_t ip_int = ip_to_int(ip);
    for (const auto& range : blocked_ranges) {
        if (ip_int >= range.start && ip_int <= range.end) {
            return true;
        }
    }
    return false;
};

/**
 * @brief Constructs a switch_router with pointers to two load balancers.
 * @param lb_processing Pointer to the processing load balancer.
 * @param lb_streaming Pointer to the streaming load balancer.
 * @param firewall_enabled Whether the firewall should be active.
 */
switch_router::switch_router(load_balancer* lb_processing, load_balancer* lb_streaming, bool firewall_enabled)
    : lb_processing(lb_processing), lb_streaming(lb_streaming),
      routed_P(0), routed_S(0), rejected_count(0),
      firewall_enabled(firewall_enabled), blocked_count(0) {
    *out << GREEN << "[INIT] Switch router created. Firewall: "
         << (firewall_enabled ? "ENABLED" : "DISABLED") << RESET << std::endl;
};

/**
 * @brief Routes a request to the appropriate load balancer based on job type.
 *        If firewall is enabled, checks IP range before routing.
 * @param new_request The incoming request to route.
 * @param curr_time Current simulation clock tick.
 */
void switch_router::route_request(request new_request, int curr_time) {
    // check firewall if enabled
    if (firewall_enabled && is_blocked(new_request.get_IP_in())) {
        blocked_count++;
        *out << RED << "[TICK " << curr_time << "] [FIREWALL] Blocked request from "
             << new_request.get_IP_in() << " -> " << new_request.get_IP_out() << RESET << std::endl;
        return;
    }

    if (new_request.get_job_type() == 'P') {
        lb_processing->add_request(new_request);
        routed_P++;
    } else if (new_request.get_job_type() == 'S') {
        lb_streaming->add_request(new_request);
        routed_S++;
    } else {
        rejected_count++;
        *out << RED << "[TICK " << curr_time << "] [SWITCH] Unknown job type '"
             << new_request.get_job_type() << "', request rejected." << RESET << std::endl;
    }
};

/**
 * @brief Processes one clock tick for both load balancers.
 * @param curr_time Current simulation clock tick.
 */
void switch_router::process_tick(int curr_time) {
    lb_processing->process_tick(curr_time);
    lb_streaming->process_tick(curr_time);
};

/**
 * @brief Adds an IP range to the blocklist.
 * @param start Start IP address of the range (e.g. "192.168.1.0").
 * @param end End IP address of the range (e.g. "192.168.1.255").
 */
void switch_router::block_range(const std::string& start, const std::string& end) {
    uint32_t start_int = ip_to_int(start);
    uint32_t end_int   = ip_to_int(end);
    for (const auto& range : blocked_ranges) {
        if (range.start == start_int && range.end == end_int) {
            *out << YELLOW << "[FIREWALL] Range " << start << " - " << end << " already blocked." << RESET << std::endl;
            return;
        }
    }
    blocked_ranges.push_back({start_int, end_int, start, end});
    *out << RED << "[FIREWALL] Blocked IP range " << start << " - " << end << RESET << std::endl;
};

/**
 * @brief Removes an IP range from the blocklist.
 * @param start Start IP address of the range.
 * @param end End IP address of the range.
 */
void switch_router::unblock_range(const std::string& start, const std::string& end) {
    uint32_t start_int = ip_to_int(start);
    uint32_t end_int   = ip_to_int(end);
    for (size_t i = 0; i < blocked_ranges.size(); i++) {
        if (blocked_ranges.at(i).start == start_int && blocked_ranges.at(i).end == end_int) {
            blocked_ranges.erase(blocked_ranges.begin() + i);
            *out << GREEN << "[FIREWALL] Unblocked IP range " << start << " - " << end << RESET << std::endl;
            return;
        }
    }
    *out << YELLOW << "[FIREWALL] Range " << start << " - " << end << " not found in blocklist." << RESET << std::endl;
};