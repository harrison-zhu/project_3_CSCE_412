// includes
#include <iostream>
#include <stdexcept>
#include "web_server.h"

// define static variables
int web_server::tot_servers = 0;

web_server::web_server(): running(false) {
    this->tot_servers++;
    this->server_number = this->tot_servers;

    std::cout << "Created web server " << server_number << std::endl;
};

web_server::web_server(int curr_time) {
    this->running = false;
    this->tot_servers++;
    this->server_number = this->tot_servers;

    if (curr_time == 0) {
        std::cout << "[INIT] ";
    } else {
        std::cout << "[Time " << curr_time << "] ";
    }
    std::cout << "Created web server " << server_number << std::endl;
};

web_server::web_server(request curr_request, int curr_time) {
    this->curr_request = curr_request;
    this->start_time = curr_time;
    this->end_time = curr_time + curr_request.get_time();
    this->running = true;
    this->tot_servers++;
    this->server_number = this->tot_servers;
};

bool web_server::check_finished(int curr_time) {
    if (curr_time == end_time) {
        running = false;

        std::cout << "[Time " << curr_time << "] ";
        std::cout << "Web server " << server_number << " has finished running a request" << std::endl;
    }
    
    return (!running);
}

void web_server::add_request(request curr_request, int curr_time) {
    // redundant, but throw error if still running other request
    if (running == true) {
        throw std::out_of_range("Trying to add request when there is one running");
    }

    this->curr_request = curr_request;
    this->start_time = curr_time;
    this->end_time = curr_time + curr_request.get_time();
    this->running = true;
}