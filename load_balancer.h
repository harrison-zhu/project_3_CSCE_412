#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

// includes
#include <iostream>
#include <vector>
#include <queue>
#include "request.h"
#include "web_server.h"

// global output stream defined in main.cpp
extern std::ostream* out;

class load_balancer {
    private:
        std::queue<request> request_queue;
        std::vector<web_server*> servers;
        char job_type;

        // tracking counters for summary
        int scale_up_count;
        int scale_down_count;
        int inactive_count;      // total servers deallocated
        int rejected_count;      // requests rejected by firewall
        int total_requests;      // total requests ever added to queue

        int next_scale_check;    // tick to check scaling again after waiting

        // scaling thresholds from config
        int scale_up_threshold;
        int scale_down_threshold;
        int scale_wait_cycles;

    public:
        load_balancer();
        load_balancer(char job_type, int num_servers, int scale_up_threshold, int scale_down_threshold, int scale_wait_cycles);
        void add_request(request new_request);
        void add_server(int curr_time);
        void remove_server(int curr_time);
        void process_tick(int curr_time);

        // getters for summary
        int get_queue_size() const    { return request_queue.size(); }
        int get_server_count() const  { return servers.size(); }
        int get_scale_ups() const     { return scale_up_count; }
        int get_scale_downs() const   { return scale_down_count; }
        int get_inactive_count() const{ return inactive_count; }
        int get_rejected() const      { return rejected_count; }
        int get_total_requests() const{ return total_requests; }

        ~load_balancer();
};

#endif