// includes
#include "web_server.h"
#include <stdexcept>

// define static variables
int web_server::tot_servers = 0;

web_server::web_server(): running(false) {
    this->tot_servers++;
    this->server_number = this->tot_servers;
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
    if (curr_time >= end_time) {
        running = false;
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