// includes
#include <stdexcept>
#include "load_balancer.h"

// defaults to 'P' for processing workload categorization
load_balancer::load_balancer(): job_type('P') {
    // default is 10 servers created initially
    for (size_t i = 0; i < 10; i++) {
        servers.push_back(web_server());
    }
};

load_balancer::load_balancer(char job_type, int num_servers) {
    this->job_type = job_type;
    for (size_t i = 0; i < num_servers; i++) {
        servers.push_back(web_server());
    }
};

void load_balancer::add_request(request new_request) {
    request_queue.push(new_request);
};

void load_balancer::add_server() {
    servers.push_back(web_server());
};

void load_balancer::remove_server(int curr_time) {
    for (size_t i = 0; i < servers.size(); i++) {
        if (servers.at(i).check_finished(curr_time)) {
            servers.erase(servers.begin() + i);
            return;
        }
    }

    throw std::out_of_range("All servers busy, cannot remove server to scale down.");
};
