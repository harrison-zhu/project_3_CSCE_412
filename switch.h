/**
 * @file switch.h
 * @brief Declaration of the switch_router class and ip_range struct.
 *
 * @details The switch_router acts as a high-level traffic manager that sits
 * above the load balancers. All incoming requests pass through it first.
 * It checks requests against the built-in IP range firewall (if enabled),
 * then routes them to the appropriate load balancer based on job type:
 * 'P' for processing and 'S' for streaming.
 *
 * The firewall performs numeric IP range comparison, converting dotted decimal
 * addresses to 32-bit integers for efficient range checks.
 *
 * @author Your Name
 * @date 2025
 */

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
 * @brief Represents a blocked IP address range.
 *
 * @details Stores the range as both 32-bit integers for fast numeric comparison
 * and as human-readable strings for logging purposes.
 */
struct ip_range {
    uint32_t start;        ///< Start of blocked range as 32-bit integer
    uint32_t end;          ///< End of blocked range as 32-bit integer
    std::string start_str; ///< Human-readable start IP (e.g. "192.168.1.0")
    std::string end_str;   ///< Human-readable end IP (e.g. "192.168.1.255")
};

/**
 * @brief High-level switch that routes requests to load balancers with built-in firewall.
 *
 * @details Sits above both load balancers and acts as the single entry point
 * for all incoming requests. Responsibilities:
 * - Check each request against the IP range firewall (if enabled)
 * - Route allowed requests to the correct load balancer by job type
 * - Tick both load balancers each clock cycle via process_tick()
 * - Track routing and blocking statistics for the simulation summary
 */
class switch_router {
    private:
        load_balancer* lb_processing;         ///< Pointer to the processing load balancer
        load_balancer* lb_streaming;          ///< Pointer to the streaming load balancer
        int routed_P;                         ///< Total requests routed to processing LB
        int routed_S;                         ///< Total requests routed to streaming LB
        int rejected_count;                   ///< Requests rejected due to unknown job type
        bool firewall_enabled;                ///< Whether the IP range firewall is active
        std::vector<ip_range> blocked_ranges; ///< List of currently blocked IP ranges
        int blocked_count;                    ///< Total requests blocked by the firewall

        /**
         * @brief Converts a dotted decimal IP string to a 32-bit integer.
         * @param ip IP address string (e.g. "192.168.1.100").
         * @return 32-bit unsigned integer representation of the IP.
         */
        uint32_t ip_to_int(const std::string& ip) const;

        /**
         * @brief Checks whether an IP address falls within any blocked range.
         * @param ip The IP address to check.
         * @return true if the IP is blocked, false if allowed.
         */
        bool is_blocked(const std::string& ip) const;

    public:
        /**
         * @brief Constructs a switch_router.
         * @param lb_processing Pointer to the processing load balancer.
         * @param lb_streaming Pointer to the streaming load balancer.
         * @param firewall_enabled Whether the IP range firewall should be active.
         */
        switch_router(load_balancer* lb_processing, load_balancer* lb_streaming, bool firewall_enabled);

        /**
         * @brief Routes a request to the appropriate load balancer.
         *        Checks firewall before routing if enabled.
         * @param new_request The incoming request to route.
         * @param curr_time Current simulation clock tick (for logging).
         */
        void route_request(request new_request, int curr_time);

        /**
         * @brief Advances both load balancers by one clock tick.
         * @param curr_time Current simulation clock tick.
         */
        void process_tick(int curr_time);

        /**
         * @brief Adds an IP range to the firewall blocklist.
         * @param start Start IP address of the range (e.g. "192.168.1.0").
         * @param end End IP address of the range (e.g. "192.168.1.255").
         */
        void block_range(const std::string& start, const std::string& end);

        /**
         * @brief Removes an IP range from the firewall blocklist.
         * @param start Start IP address of the range to unblock.
         * @param end End IP address of the range to unblock.
         */
        void unblock_range(const std::string& start, const std::string& end);

        /// @brief Returns total requests routed to the processing load balancer.
        int get_routed_P() const          { return routed_P; }
        /// @brief Returns total requests routed to the streaming load balancer.
        int get_routed_S() const          { return routed_S; }
        /// @brief Returns total requests rejected due to unknown job type.
        int get_rejected() const          { return rejected_count; }
        /// @brief Returns total requests blocked by the firewall.
        int get_blocked() const           { return blocked_count; }
        /// @brief Returns whether the firewall is currently enabled.
        bool get_firewall_enabled() const { return firewall_enabled; }
};

#endif