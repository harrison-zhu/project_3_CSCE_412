CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17

all: myprogram

myprogram: main.o load_balancer.o web_server.o request.o switch.o
	$(CXX) $(CXXFLAGS) -o myprogram main.o load_balancer.o web_server.o request.o switch.o

main.o: main.cpp load_balancer.h web_server.h request.h switch.h
	$(CXX) $(CXXFLAGS) -c main.cpp

load_balancer.o: load_balancer.cpp load_balancer.h web_server.h request.h
	$(CXX) $(CXXFLAGS) -c load_balancer.cpp

web_server.o: web_server.cpp web_server.h request.h
	$(CXX) $(CXXFLAGS) -c web_server.cpp

request.o: request.cpp request.h
	$(CXX) $(CXXFLAGS) -c request.cpp

switch.o: switch.cpp switch.h load_balancer.h request.h
	$(CXX) $(CXXFLAGS) -c switch.cpp

clean:
	rm -f myprogram *.o
