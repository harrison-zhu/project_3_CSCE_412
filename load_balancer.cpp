// includes
#include <iostream>
#include <stdexcept>
#include "load_balancer.h"

// defaults to 'P' for processing workload categorization
load_balancer::load_balancer(): job_type('P') {
    // default is 10 servers created initially
    for (size_t i = 0; i < 10; i++) {
        servers.push_back(new web_server(0));
    }

    std::cout << "[INIT] Load balancer created for job type "
              << 'F' << " with "
              << 10 << " servers." << std::endl;
};

load_balancer::load_balancer(char job_type, int num_servers) {
    this->job_type = job_type;
    for (size_t i = 0; i < num_servers; i++) {
        servers.push_back(new web_server(0));
    }

    std::cout << "[INIT] Load balancer created for job type "
              << job_type << " with "
              << num_servers << " servers." << std::endl;
};

void load_balancer::add_request(request new_request) {
    request_queue.push(new_request);
};

void load_balancer::add_server(int curr_time) {
    servers.push_back(new web_server(curr_time));
};

void load_balancer::remove_server(int curr_time) {
    for (size_t i = 0; i < servers.size(); i++) {
        if (servers.at(i)->check_finished(curr_time)) {
            delete servers.at(i);        // deallocate
            servers.erase(servers.begin() + i);
            return;
        }
    }
    throw std::out_of_range("All servers busy, cannot remove server to scale down.");
};

void load_balancer::process_tick(int curr_time) {

};

load_balancer::~load_balancer() {
    for (size_t i = 0; i < servers.size(); i++) {
        delete servers.at(i);
    }
    servers.clear();
}