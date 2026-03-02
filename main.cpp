#include <iostream>
#include <string>
#include <random>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "request.h"
#include "web_server.h"
#include "load_balancer.h"

struct Config {
    int num_servers;
    int run_time;
    int scale_up_threshold;    // 80
    int scale_down_threshold;  // 50
    int scale_wait_cycles;     // n clock cycles to wait before checking again
    int request_time_min;      // min clock cycles a request takes
    int request_time_max;      // max clock cycles a request takes
    int new_request_rate_min;
    int new_request_rate_max;
    std::string output_file;  // empty string means terminal only
    char job_type;             // 'P' processing or 'S' streaming
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
        // skip empty lines and comments
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
            else if (key == "output_file") cfg.output_file = value;
            else if (key == "job_type")             cfg.job_type             = value[0];
        }
    }

    return cfg;
}

// generates a random request
request generate_request(int process_time_low, int process_time_high) {
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

    std::uniform_int_distribution<int> job_dist(0, 1);
    char job_type = (job_dist(rng) == 0) ? 'P' : 'S';

    return request(IP_in, IP_out, time, job_type);
};

int main() {
    // load config
    Config cfg = load_config("config.txt");

    // open log file
    std::ofstream log("simulation.log");
    if (!log.is_open()) {
        throw std::runtime_error("Could not open log file.");
    }

    // init rng for request rate
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> rate_dist(cfg.new_request_rate_min, cfg.new_request_rate_max);

    // create load balancers
    log << "[INIT] Creating load balancers..." << std::endl;
    std::cout << "[INIT] Creating load balancers..." << std::endl;

    load_balancer load_balancer_P('P', cfg.num_servers);
    load_balancer load_balancer_S('S', cfg.num_servers);

    // generate initial queue of servers * 100 requests
    int initial_requests = cfg.num_servers * 100;
    log << "[INIT] Generating " << initial_requests << " initial requests per load balancer..." << std::endl;
    std::cout << "[INIT] Generating " << initial_requests << " initial requests per load balancer..." << std::endl;

    for (int i = 0; i < initial_requests; i++) {
        request r = generate_request(cfg.request_time_min, cfg.request_time_max);
        if (r.get_job_type() == 'P')
            load_balancer_P.add_request(r);
        else
            load_balancer_S.add_request(r);
    }

    log << "[INIT] Initialization complete. Starting simulation..." << std::endl;
    std::cout << "[INIT] Initialization complete. Starting simulation for " << cfg.run_time << " ticks..." << std::endl;

    // schedule first new request
    int next_request_time = rate_dist(rng);

    int time = 0;

    while (time < 10000) {


        time++;
    }

    return 0;
}