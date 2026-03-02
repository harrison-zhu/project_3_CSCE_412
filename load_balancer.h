#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

// includes
#include <vector>
#include <queue>
#include "request.h"
#include "web_server.h"

class load_balancer {
    private:
        std::queue<request> request_queue; // likely to track the queue size and number of queues
        std::vector<web_server*> servers;
        char job_type;
    public:
        load_balancer();
        load_balancer(char job_type, int num_servers);
        void add_request(request new_request);
        void add_server(int curr_time);
        void remove_server(int curr_time);
        void process_tick(int curr_time);
        ~load_balancer();
};

#endif