/**
 * @file main.cpp
 * @brief Entry point for the simulated load balancer application.
 *
 * @details This program simulates a dynamic load balancer that manages two pools
 * of web servers — one for processing ('P') requests and one for streaming ('S')
 * requests. Requests are routed through a high-level switch that includes a
 * built-in IP range firewall. Server counts scale up or down dynamically based
 * on queue size thresholds read from a configuration file.
 *
 * Configuration is loaded from config.txt, which controls the number of servers,
 * run time, scaling thresholds, request rates, firewall settings, and output destination.
 *
 * Output can be directed to the terminal (default) or to a file via command line
 * argument or config setting.
 *
 * @author Your Name
 * @date 2025
 */

#include <iostream>
#include <string>
#include <random>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "request.h"
#include "web_server.h"
#include "load_balancer.h"
#include "switch.h"

// colors for logging
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"
#define WHITE   "\033[37m"

/// @brief Global output stream. Defaults to std::cout. Can be redirected to a file.
std::ostream* out = &std::cout;

/**
 * @brief Configuration struct holding all simulation parameters loaded from config.txt.
 */
struct Config {
    int num_servers;           ///< Initial number of web servers per load balancer
    int run_time;              ///< Total number of clock ticks to simulate
    int scale_up_threshold;    ///< Queue size multiplier to trigger adding a server (default 80)
    int scale_down_threshold;  ///< Queue size multiplier to trigger removing a server (default 50)
    int scale_wait_cycles;     ///< Clock cycles to wait between scaling actions
    int request_time_min;      ///< Minimum clock cycles a request takes to process
    int request_time_max;      ///< Maximum clock cycles a request takes to process
    int new_request_rate_min;  ///< Minimum clock cycles between new incoming requests
    int new_request_rate_max;  ///< Maximum clock cycles between new incoming requests
    int streaming_percent;     ///< Percentage of requests that are streaming type (0-100)
    int log_interval;          ///< How often (in ticks) to log queue status
    bool firewall_enabled;     ///< Whether the IP range firewall is active
    std::vector<std::pair<std::string, std::string>> blocked_ranges; ///< List of blocked IP ranges (start, end)
    std::string output_file;   ///< Output file path (empty means terminal)
};

/**
 * @brief Loads simulation configuration from a key=value text file.
 *
 * @details Parses each line of the config file as a key=value pair. Lines
 * beginning with '#' or empty lines are skipped. Blocked IP ranges are
 * specified as: blocked_range=start_ip,end_ip
 *
 * @param filename Path to the configuration file.
 * @return Config struct populated with values from the file.
 * @throws std::runtime_error if the file cannot be opened.
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
            else if (key == "run_time")             cfg.run_time             = std::stoi(value);
            else if (key == "scale_up_threshold")   cfg.scale_up_threshold   = std::stoi(value);
            else if (key == "scale_down_threshold") cfg.scale_down_threshold = std::stoi(value);
            else if (key == "scale_wait_cycles")    cfg.scale_wait_cycles    = std::stoi(value);
            else if (key == "request_time_min")     cfg.request_time_min     = std::stoi(value);
            else if (key == "request_time_max")     cfg.request_time_max     = std::stoi(value);
            else if (key == "new_request_rate_min") cfg.new_request_rate_min = std::stoi(value);
            else if (key == "new_request_rate_max") cfg.new_request_rate_max = std::stoi(value);
            else if (key == "streaming_percent")    cfg.streaming_percent    = std::stoi(value);
            else if (key == "log_interval")         cfg.log_interval         = std::stoi(value);
            else if (key == "firewall_enabled")     cfg.firewall_enabled     = std::stoi(value);
            else if (key == "output_file")          cfg.output_file          = value;
            else if (key == "blocked_range") {
                // format: start_ip,end_ip
                std::istringstream range_ss(value);
                std::string start, end;
                if (std::getline(range_ss, start, ',') && std::getline(range_ss, end)) {
                    cfg.blocked_ranges.push_back({start, end});
                }
            }
        }
    }

    return cfg;
}

/**
 * @brief Generates a random network request with random source/destination IPs,
 *        a random processing time, and a randomly assigned job type.
 *
 * @param process_time_low  Minimum number of ticks the request takes to process.
 * @param process_time_high Maximum number of ticks the request takes to process.
 * @param streaming_percent Percentage chance (0-100) the request is type 'S' (streaming).
 *                          Otherwise it is type 'P' (processing).
 * @return A newly constructed request object.
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

/**
 * @brief Main entry point. Initializes and runs the load balancer simulation.
 *
 * @details Workflow:
 * -# Load configuration from config.txt
 * -# Set up output stream (terminal or file)
 * -# Create two load balancers (processing and streaming)
 * -# Create a switch router with optional firewall
 * -# Populate initial request queues (num_servers * 100 requests)
 * -# Run simulation loop for run_time ticks:
 *    - Add new requests at random intervals
 *    - Process one tick on each load balancer
 *    - Log queue status every log_interval ticks
 * -# Print simulation summary
 *
 * @param argc Number of command line arguments.
 * @param argv Command line arguments. argv[1] optionally specifies an output file.
 * @return 0 on success.
 */
int main(int argc, char* argv[]) {
    // load config
    Config cfg = load_config("config.txt");

    // set up output stream - command line arg overrides config file
    std::ofstream file_out;
    if (argc > 1) {
        file_out.open(argv[1]);
        if (!file_out.is_open()) {
            throw std::runtime_error("Could not open output file: " + std::string(argv[1]));
        }
        out = &file_out;
    } else if (!cfg.output_file.empty()) {
        file_out.open(cfg.output_file);
        if (!file_out.is_open()) {
            throw std::runtime_error("Could not open output file: " + cfg.output_file);
        }
        out = &file_out;
    }

    // init rng for request rate
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> rate_dist(cfg.new_request_rate_min, cfg.new_request_rate_max);

    // create load balancers
    *out << GREEN << "[INIT] Creating load balancers..." << RESET << std::endl;

    load_balancer load_balancer_P('P', cfg.num_servers, cfg.scale_up_threshold, cfg.scale_down_threshold, cfg.scale_wait_cycles);
    load_balancer load_balancer_S('S', cfg.num_servers, cfg.scale_up_threshold, cfg.scale_down_threshold, cfg.scale_wait_cycles);

    // create switch router with firewall setting
    switch_router router(&load_balancer_P, &load_balancer_S, cfg.firewall_enabled);

    // load blocked IP ranges from config
    for (const auto& range : cfg.blocked_ranges) {
        router.block_range(range.first, range.second);
    }

    // generate initial queue of servers * 100 requests
    int initial_requests = cfg.num_servers * 100;
    int initial_P = 0, initial_S = 0;

    *out << GREEN << "[INIT] Generating " << initial_requests << " initial requests..." << RESET << std::endl;

    for (int i = 0; i < initial_requests; i++) {
        request r = generate_request(cfg.request_time_min, cfg.request_time_max, cfg.streaming_percent);
        router.route_request(r, 0);
        if (r.get_job_type() == 'P') initial_P++;
        else initial_S++;
    }

    *out << GREEN << "[INIT] Starting queue size - P: " << initial_P << " | S: " << initial_S << RESET << std::endl;
    *out << GREEN << "[INIT] Task time range: " << cfg.request_time_min << " - " << cfg.request_time_max << " ticks" << RESET << std::endl;
    *out << GREEN << "[INIT] Initialization complete. Starting simulation for " << cfg.run_time << " ticks..." << RESET << std::endl;

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
            *out << YELLOW << "[TICK " << time << "] Queue size - P: " << load_balancer_P.get_queue_size()
                 << " | S: " << load_balancer_S.get_queue_size()
                 << " | Servers - P: " << load_balancer_P.get_server_count()
                 << " | S: " << load_balancer_S.get_server_count() << RESET << std::endl;
        }

        time++;
    }

    // end summary
    int ending_queue = load_balancer_P.get_queue_size() + load_balancer_S.get_queue_size();

    *out << BOLD << "\n===== SIMULATION SUMMARY =====" << RESET << std::endl;
    *out << WHITE  << "Run time:                  " << cfg.run_time << " ticks" << RESET << std::endl;
    *out << WHITE  << "Task time range:           " << cfg.request_time_min << " - " << cfg.request_time_max << " ticks" << RESET << std::endl;
    *out << YELLOW << "Starting queue size:       " << initial_requests << RESET << std::endl;
    *out << YELLOW << "Ending queue size:         " << ending_queue << RESET << std::endl;
    *out << WHITE  << "Total requests generated:  " << total_requests_generated << RESET << std::endl;
    *out << WHITE  << "Routed to P:               " << router.get_routed_P() << RESET << std::endl;
    *out << WHITE  << "Routed to S:               " << router.get_routed_S() << RESET << std::endl;
    *out << RED    << "Switch rejected:           " << router.get_rejected() << RESET << std::endl;
    *out << RED    << "Firewall blocked:          " << router.get_blocked() << RESET << std::endl;
    *out << GREEN  << "Active servers (P):        " << load_balancer_P.get_server_count() << RESET << std::endl;
    *out << GREEN  << "Active servers (S):        " << load_balancer_S.get_server_count() << RESET << std::endl;
    *out << CYAN   << "Scale-up events (P):       " << load_balancer_P.get_scale_ups() << RESET << std::endl;
    *out << CYAN   << "Scale-up events (S):       " << load_balancer_S.get_scale_ups() << RESET << std::endl;
    *out << MAGENTA << "Scale-down events (P):    " << load_balancer_P.get_scale_downs() << RESET << std::endl;
    *out << MAGENTA << "Scale-down events (S):    " << load_balancer_S.get_scale_downs() << RESET << std::endl;
    *out << RED    << "Servers deallocated (P):   " << load_balancer_P.get_inactive_count() << RESET << std::endl;
    *out << RED    << "Servers deallocated (S):   " << load_balancer_S.get_inactive_count() << RESET << std::endl;
    *out << RED    << "Rejected requests (P):     " << load_balancer_P.get_rejected() << RESET << std::endl;
    *out << RED    << "Rejected requests (S):     " << load_balancer_S.get_rejected() << RESET << std::endl;

    if (file_out.is_open()) {
        file_out.close();
    }

    return 0;
}