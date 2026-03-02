#ifndef WEB_SERVER_H
#define WEB_SERVER_H

// includes
#include "request.h"

class web_server {
    private:
        request curr_request;
        bool running;
        bool active;
        int start_time;
        int end_time;
        int server_number;
        static int tot_servers;
    public:
        web_server();
        web_server(int curr_time);
        web_server(request curr_request, int curr_time); // may be needed if make new one and then add in request
        bool check_finished(int curr_time);
        void add_request(request curr_request, int curr_time);
};

#endif