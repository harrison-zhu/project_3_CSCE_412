#include <iostream>
// probably need includes for every header or cpp file
// may need includes for other things like APIs
#include "request.h"
#include "web_server.h"
#include "load_balancer.h"

int main() {
    // likely need a way to process commands and flags
        // look back on old 312 files
    
    // probably need to initialize load_balancer
        // should load balancer have the queue of requests?
        // makes sense as for bonus, different load balancers have diff queues
        // load balancer should also manage its own web servers?
    load_balancer load_balancer_P();
    
    // need to initialize web servers
        // either array of them here or in the class
            // probs in the class
        // may have to use a REST API? one compatible for mac?
    
    // may have to loop through 10000 clock cycles
        // randomly during time, create request from request class

    return 0;
}