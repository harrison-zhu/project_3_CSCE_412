#include <iostream>
#include <string>
#include <random>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "request.h"
#include "web_server.h"
#include "load_balancer.h"
#include "switch.h"

// global output stream - defined here, extern in headers
std::ostream* out = &std::cout;

struct Config {
    int num_servers;
    int run_time;
    int scale_up_threshold;
    int scale_down_threshold;
    int scale_wait_cycles;
    int request_time_min;
    int request_time_max;
    int new_request_rate_min;
    int new_request_rate_max;
    int streaming_percent;
    int log_interval;
    std::string output_file;
};

/**
 * @brief Loads configuration from a key=value text file.
 * @param filename Path to the config file.
 * @return Config struct populated with values from file.
 * @throws std::runtime_error if file cannot be opened.
 */
Config load_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filename);
    }

    Config cfg;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string key, value;

        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            if      (key == "num_servers")          cfg.num_servers          = std::stoi(value);
            else if (key == "run_time")              cfg.run_time             = std::stoi(value);
            else if (key == "scale_up_threshold")   cfg.scale_up_threshold   = std::stoi(value);
            else if (key == "scale_down_threshold") cfg.scale_down_threshold = std::stoi(value);
            else if (key == "scale_wait_cycles")    cfg.scale_wait_cycles    = std::stoi(value);
            else if (key == "request_time_min")     cfg.request_time_min     = std::stoi(value);
            else if (key == "request_time_max")     cfg.request_time_max     = std::stoi(value);
            else if (key == "new_request_rate_min") cfg.new_request_rate_min = std::stoi(value);
            else if (key == "new_request_rate_max") cfg.new_request_rate_max = std::stoi(value);
            else if (key == "streaming_percent")    cfg.streaming_percent    = std::stoi(value);
            else if (key == "log_interval") cfg.log_interval = std::stoi(value);
            else if (key == "output_file")          cfg.output_file          = value;
        }
    }

    return cfg;
}

/**
 * @brief Generates a random request with random IPs, processing time, and job type.
 * @param process_time_low Minimum processing time.
 * @param process_time_high Maximum processing time.
 * @param streaming_percent Percentage chance (0-100) that the request is streaming.
 * @return A randomly generated request.
 */
request generate_request(int process_time_low, int process_time_high, int streaming_percent) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 255);

    std::string IP_in = std::to_string(dist(rng)) + "." +
           std::to_string(dist(rng)) + "." +
           std::to_string(dist(rng)) + "." +
           std::to_string(dist(rng));

    std::string IP_out = std::to_string(dist(rng)) + "." +
           std::to_string(dist(rng)) + "." +
           std::to_string(dist(rng)) + "." +
           std::to_string(dist(rng));

    std::uniform_int_distribution<int> time_dist(process_time_low, process_time_high);
    int time = time_dist(rng);

    std::uniform_int_distribution<int> percent_dist(1, 100);
    char job_type = (percent_dist(rng) <= streaming_percent) ? 'S' : 'P';

    return request(IP_in, IP_out, time, job_type);
};

int main(int argc, char* argv[]) {
    // load config
    Config cfg = load_config("config.txt");

    // set up output stream - command line arg overrides config file
    std::ofstream file_out;
    if (argc > 1) {
        // output file passed as command line argument
        file_out.open(argv[1]);
        if (!file_out.is_open()) {
            throw std::runtime_error("Could not open output file: " + std::string(argv[1]));
        }
        out = &file_out;
    } else if (!cfg.output_file.empty()) {
        // output file set in config
        file_out.open(cfg.output_file);
        if (!file_out.is_open()) {
            throw std::runtime_error("Could not open output file: " + cfg.output_file);
        }
        out = &file_out;
    }
    // otherwise out stays as &std::cout

    // init rng for request rate
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> rate_dist(cfg.new_request_rate_min, cfg.new_request_rate_max);

    // create load balancers
    *out << "[INIT] Creating load balancers..." << std::endl;

    load_balancer load_balancer_P('P', cfg.num_servers / 2, cfg.scale_up_threshold, cfg.scale_down_threshold, cfg.scale_wait_cycles);
    load_balancer load_balancer_S('S', cfg.num_servers - cfg.num_servers / 2, cfg.scale_up_threshold, cfg.scale_down_threshold, cfg.scale_wait_cycles);

    // create switch router
    switch_router router(&load_balancer_P, &load_balancer_S);

    // generate initial queue of servers * 100 requests
    int initial_requests = cfg.num_servers * 100;
    int initial_P = 0, initial_S = 0;

    *out << "[INIT] Generating " << initial_requests << " initial requests..." << std::endl;

    for (int i = 0; i < initial_requests; i++) {
        request r = generate_request(cfg.request_time_min, cfg.request_time_max, cfg.streaming_percent);
        if (r.get_job_type() == 'P') initial_P++;
        else initial_S++;
    }

    *out << "[INIT] Starting queue size - P: " << initial_P << " | S: " << initial_S << std::endl;
    *out << "[INIT] Task time range: " << cfg.request_time_min << " - " << cfg.request_time_max << " ticks" << std::endl;
    *out << "[INIT] Initialization complete. Starting simulation for " << cfg.run_time << " ticks..." << std::endl;

    // schedule first new request
    int next_request_time = rate_dist(rng);
    int total_requests_generated = initial_requests;

    int time = 0;
    while (time < cfg.run_time) {

        // add new request at random intervals via switch router
        if (time >= next_request_time) {
            request r = generate_request(cfg.request_time_min, cfg.request_time_max, cfg.streaming_percent);
            router.route_request(r, time);
            total_requests_generated++;
            next_request_time = time + rate_dist(rng);
        }

        // process tick for both load balancers via switch router
        router.process_tick(time);

        // log queue size every log_interval ticks
        if (time % cfg.log_interval == 0) {
            *out << "[TICK " << time << "] Queue size - P: " << load_balancer_P.get_queue_size()
                 << " | S: " << load_balancer_S.get_queue_size()
                 << " | Servers - P: " << load_balancer_P.get_server_count()
                 << " | S: " << load_balancer_S.get_server_count() << std::endl;
        }

        time++;
    }

    // end summary
    int ending_queue = load_balancer_P.get_queue_size() + load_balancer_S.get_queue_size();

    *out << "\n===== SIMULATION SUMMARY =====" << std::endl;
    *out << "Run time:                  " << cfg.run_time << " ticks" << std::endl;
    *out << "Task time range:           " << cfg.request_time_min << " - " << cfg.request_time_max << " ticks" << std::endl;
    *out << "Starting queue size:       " << initial_requests << std::endl;
    *out << "Ending queue size:         " << ending_queue << std::endl;
    *out << "Total requests generated:  " << total_requests_generated << std::endl;
    *out << "Routed to P:               " << router.get_routed_P() << std::endl;
    *out << "Routed to S:               " << router.get_routed_S() << std::endl;
    *out << "Switch rejected:           " << router.get_rejected() << std::endl;
    *out << "Active servers (P):        " << load_balancer_P.get_server_count() << std::endl;
    *out << "Active servers (S):        " << load_balancer_S.get_server_count() << std::endl;
    *out << "Scale-up events (P):       " << load_balancer_P.get_scale_ups() << std::endl;
    *out << "Scale-up events (S):       " << load_balancer_S.get_scale_ups() << std::endl;
    *out << "Scale-down events (P):     " << load_balancer_P.get_scale_downs() << std::endl;
    *out << "Scale-down events (S):     " << load_balancer_S.get_scale_downs() << std::endl;
    *out << "Servers deallocated (P):   " << load_balancer_P.get_inactive_count() << std::endl;
    *out << "Servers deallocated (S):   " << load_balancer_S.get_inactive_count() << std::endl;
    *out << "Rejected requests (P):     " << load_balancer_P.get_rejected() << std::endl;
    *out << "Rejected requests (S):     " << load_balancer_S.get_rejected() << std::endl;

    if (file_out.is_open()) {
        file_out.close();
    }

    return 0;
}