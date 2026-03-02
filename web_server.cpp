// includes
#include <iostream>
#include <stdexcept>
#include "web_server.h"

// define static variables
int web_server::tot_servers = 0;

web_server::web_server(): running(false), job_type('P') {
    this->tot_servers++;
    this->server_number = this->tot_servers;
    *out << "[INIT] Created web server " << server_number << " for load balancer P" << std::endl;
};

web_server::web_server(int curr_time, char job_type) {
    this->running = false;
    this->job_type = job_type;
    this->tot_servers++;
    this->server_number = this->tot_servers;

    if (curr_time == 0) {
        *out << "[INIT] ";
    } else {
        *out << "[Time " << curr_time << "] ";
    }
    *out << "[" << job_type << "] ";
    *out << "Created web server " << server_number << std::endl;
};

web_server::web_server(request curr_request, int curr_time, char job_type) {
    this->curr_request = curr_request;
    this->start_time = curr_time;
    this->end_time = curr_time + curr_request.get_time();
    this->running = true;
    this->job_type = job_type;
    this->tot_servers++;
    this->server_number = this->tot_servers;
    *out << "[Time " << curr_time << "] Created web server " << server_number << " for load balancer " << job_type << std::endl;
};

bool web_server::check_finished(int curr_time) {
    if (curr_time >= end_time && running) {
        running = false;
        *out << "[Time " << curr_time << "] Web server " << server_number
             << " [" << job_type << "] has finished running a request" << std::endl;
    }
    return (!running);
}

void web_server::add_request(request curr_request, int curr_time) {
    if (running == true) {
        throw std::out_of_range("Trying to add request when there is one running");
    }
    this->curr_request = curr_request;
    this->start_time = curr_time;
    this->end_time = curr_time + curr_request.get_time();
    this->running = true;
}