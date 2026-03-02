#ifndef SWITCH_H
#define SWITCH_H

// includes
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include "request.h"
#include "load_balancer.h"

// global output stream defined in main.cpp
extern std::ostream* out;

/**
 * @brief Represents a blocked IP range with a start and end address.
 */
struct ip_range {
    uint32_t start;        ///< Start of blocked range as 32-bit integer
    uint32_t end;          ///< End of blocked range as 32-bit integer
    std::string start_str; ///< Human-readable start IP
    std::string end_str;   ///< Human-readable end IP
};

/**
 * @brief High-level switch that routes incoming requests to the appropriate
 *        load balancer based on job type ('P' for processing, 'S' for streaming).
 *        Contains built-in firewall for IP range blocking.
 */
class switch_router {
    private:
        load_balancer* lb_processing;         // load balancer for processing requests
        load_balancer* lb_streaming;          // load balancer for streaming requests
        int routed_P;                         // total requests routed to processing
        int routed_S;                         // total requests routed to streaming
        int rejected_count;                   // requests rejected due to unknown job type
        bool firewall_enabled;                // whether firewall is active
        std::vector<ip_range> blocked_ranges; // list of blocked IP ranges
        int blocked_count;                    // total requests blocked by firewall

        uint32_t ip_to_int(const std::string& ip) const;
        bool is_blocked(const std::string& ip) const;

    public:
        switch_router(load_balancer* lb_processing, load_balancer* lb_streaming, bool firewall_enabled);
        void route_request(request new_request, int curr_time);
        void process_tick(int curr_time);
        void block_range(const std::string& start, const std::string& end);
        void unblock_range(const std::string& start, const std::string& end);

        // getters for summary
        int get_routed_P() const          { return routed_P; }
        int get_routed_S() const          { return routed_S; }
        int get_rejected() const          { return rejected_count; }
        int get_blocked() const           { return blocked_count; }
        bool get_firewall_enabled() const { return firewall_enabled; }
};

#endif