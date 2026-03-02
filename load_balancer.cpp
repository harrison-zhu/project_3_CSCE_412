// includes
#include <iostream>
#include <stdexcept>
#include "load_balancer.h"

// defaults to 'P' for processing workload categorization
load_balancer::load_balancer(): job_type('P'), scale_up_count(0), scale_down_count(0), inactive_count(0), rejected_count(0), total_requests(0), next_scale_check(0), scale_up_threshold(80), scale_down_threshold(50), scale_wait_cycles(50) {
    for (size_t i = 0; i < 10; i++) {
        servers.push_back(new web_server(0, 'P'));
    }
    *out << "[INIT] Load balancer created for job type P with 10 servers." << std::endl;
};

load_balancer::load_balancer(char job_type, int num_servers, int scale_up_threshold, int scale_down_threshold, int scale_wait_cycles)
    : scale_up_count(0), scale_down_count(0), inactive_count(0), rejected_count(0), total_requests(0), next_scale_check(0),
      scale_up_threshold(scale_up_threshold), scale_down_threshold(scale_down_threshold), scale_wait_cycles(scale_wait_cycles) {
    this->job_type = job_type;
    for (size_t i = 0; i < num_servers; i++) {
        servers.push_back(new web_server(0, job_type));
    }
    *out << "[INIT] Load balancer created for job type "
         << job_type << " with "
         << num_servers << " servers." << std::endl;
};

void load_balancer::add_request(request new_request) {
    request_queue.push(new_request);
    total_requests++;
};

void load_balancer::add_server(int curr_time) {
    servers.push_back(new web_server(curr_time, job_type));
    scale_up_count++;
};

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

void load_balancer::process_tick(int curr_time) {
    // step 1: assign requests to idle servers
    for (size_t i = 0; i < servers.size(); i++) {
        if (!servers.at(i)->is_running() && !request_queue.empty()) {
            request next = request_queue.front();
            request_queue.pop();
            servers.at(i)->add_request(next, curr_time);
            *out << "[TICK " << curr_time << "] [" << job_type << "] Server "
                 << servers.at(i)->get_server_number()
                 << " assigned request. Queue size: " << request_queue.size() << std::endl;
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
        *out << "[TICK " << curr_time << "] [" << job_type << "] Queue too large ("
             << queue_size << " > " << scale_up_threshold * num_servers
             << "). Added server. Total servers: " << servers.size() << std::endl;
    }
    // scale down if queue is too small, but never go below 1 server
    else if (queue_size < scale_down_threshold * num_servers && num_servers > 1) {
        try {
            remove_server(curr_time);
            next_scale_check = curr_time + scale_wait_cycles;
            *out << "[TICK " << curr_time << "] [" << job_type << "] Queue too small ("
                 << queue_size << " < " << scale_down_threshold * num_servers
                 << "). Removed server. Total servers: " << servers.size() << std::endl;
        } catch (std::out_of_range& e) {
            // all servers busy, skip scale down this cycle
            *out << "[TICK " << curr_time << "] [" << job_type << "] Scale down skipped: " << e.what() << std::endl;
        }
    }
};

load_balancer::~load_balancer() {
    for (size_t i = 0; i < servers.size(); i++) {
        delete servers.at(i);
    }
    servers.clear();
}