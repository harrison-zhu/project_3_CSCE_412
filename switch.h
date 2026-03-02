#ifndef SWITCH_H
#define SWITCH_H

// includes
#include <iostream>
#include "request.h"
#include "load_balancer.h"

// global output stream defined in main.cpp
extern std::ostream* out;

/**
 * @brief High-level switch that routes incoming requests to the appropriate
 *        load balancer based on job type ('P' for processing, 'S' for streaming).
 */
class switch_router {
    private:
        load_balancer* lb_processing;   // load balancer for processing requests
        load_balancer* lb_streaming;    // load balancer for streaming requests
        int routed_P;                   // total requests routed to processing
        int routed_S;                   // total requests routed to streaming
        int rejected_count;             // requests rejected by firewall

    public:
        switch_router(load_balancer* lb_processing, load_balancer* lb_streaming);
        void route_request(request new_request, int curr_time);
        void process_tick(int curr_time);

        // getters for summary
        int get_routed_P() const      { return routed_P; }
        int get_routed_S() const      { return routed_S; }
        int get_rejected() const      { return rejected_count; }
};

#endif