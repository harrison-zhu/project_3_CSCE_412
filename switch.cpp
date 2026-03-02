// includes
#include <iostream>
#include <stdexcept>
#include "switch.h"

/**
 * @brief Constructs a switch_router with pointers to two load balancers.
 * @param lb_processing Pointer to the processing load balancer.
 * @param lb_streaming Pointer to the streaming load balancer.
 */
switch_router::switch_router(load_balancer* lb_processing, load_balancer* lb_streaming)
    : lb_processing(lb_processing), lb_streaming(lb_streaming), routed_P(0), routed_S(0), rejected_count(0) {
    *out << "[INIT] Switch router created." << std::endl;
};

/**
 * @brief Routes a request to the appropriate load balancer based on job type.
 * @param new_request The incoming request to route.
 * @param curr_time Current simulation clock tick.
 */
void switch_router::route_request(request new_request, int curr_time) {
    if (new_request.get_job_type() == 'P') {
        lb_processing->add_request(new_request);
        routed_P++;
    } else if (new_request.get_job_type() == 'S') {
        lb_streaming->add_request(new_request);
        routed_S++;
    } else {
        rejected_count++;
        *out << "[TICK " << curr_time << "] [SWITCH] Unknown job type '" << new_request.get_job_type() << "', request rejected." << std::endl;
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