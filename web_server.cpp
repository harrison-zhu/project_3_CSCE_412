// includes
#include <iostream>
#include <stdexcept>
#include "web_server.h"

// colors for logging
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"
#define WHITE   "\033[37m"

// define static variables
int web_server::tot_servers = 0;

web_server::web_server(): running(false), job_type('P') {
    this->tot_servers++;
    this->server_number = this->tot_servers;
    *out << GREEN << "[INIT] Created web server " << server_number << " for load balancer P" << RESET << std::endl;
};

web_server::web_server(int curr_time, char job_type) {
    this->running = false;
    this->job_type = job_type;
    this->tot_servers++;
    this->server_number = this->tot_servers;

    if (curr_time == 0) {
        *out << GREEN << "[INIT] Created web server " << server_number << " for load balancer " << job_type << RESET << std::endl;
    } else {
        *out << CYAN << "[TICK " << curr_time << "] Created web server " << server_number << " for load balancer " << job_type << RESET << std::endl;
    }
};

web_server::web_server(request curr_request, int curr_time, char job_type) {
    this->curr_request = curr_request;
    this->start_time = curr_time;
    this->end_time = curr_time + curr_request.get_time();
    this->running = true;
    this->job_type = job_type;
    this->tot_servers++;
    this->server_number = this->tot_servers;
    *out << CYAN << "[TICK " << curr_time << "] Created web server " << server_number << " for load balancer " << job_type << RESET << std::endl;
};

bool web_server::check_finished(int curr_time) {
    if (curr_time >= end_time && running) {
        running = false;
        *out << MAGENTA << "[TICK " << curr_time << "] Web server " << server_number
             << " [" << job_type << "] finished request." << RESET << std::endl;
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