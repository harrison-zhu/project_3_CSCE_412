#include "request.h"

// likely not used
request::request(): IP_in(), IP_out(), time(), job_type('P') {};

request::request(std::string IP_in, std::string IP_out, int time, char job_type) {
    this->IP_in = IP_in;
    this->IP_out = IP_out;
    this->time = time;
    this->job_type = job_type;
}

std::string request::get_IP_in() {
    return IP_in;
}

std::string request::get_IP_out() {
    return IP_out;
}

int request::get_time() {
    return time;
}

char request::get_job_type() {
    return job_type;
}