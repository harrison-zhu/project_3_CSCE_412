#ifndef REQUEST_H
#define REQUEST_H

// includes added in later once i know what i am doing
#include <string>

class request {
    private:
        std::string IP_in;
        std::string IP_out;
        int time;
        char job_type; // either P or S
    public:
        request();
        request(std::string IP_in, std::string IP_out, int time, char job_type);
        std::string get_IP_in();
        std::string get_IP_out();
        int get_time();
        char get_job_type();
};

#endif