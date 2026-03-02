#ifndef WEB_SERVER_H
#define WEB_SERVER_H

// includes
#include <iostream>
#include "request.h"

// global output stream defined in main.cpp
extern std::ostream* out;

class web_server {
    private:
        request curr_request;
        bool running;
        int start_time;
        int end_time;
        int server_number;
        char job_type;
        static int tot_servers;
    public:
        web_server();
        web_server(int curr_time, char job_type);
        web_server(request curr_request, int curr_time, char job_type);
        bool check_finished(int curr_time);
        void add_request(request curr_request, int curr_time);
        bool is_running() const { return running; }
        int get_server_number() const { return server_number; }
};

#endif