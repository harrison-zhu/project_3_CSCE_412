# CC = gcc
# CFLAGS = -Wall -Werror -std=c++17

# all: myprogram
# myprogram: main.o utils.o
#     $(CC) $(CFLAGS) -o myprogram main.o utils.o

# main.o: main.c
#     $(CC) $(CFLAGS) -c main.c

# utils.o: utils.c
#     $(CC) $(CFLAGS) -c utils.c

# clean:
#     rm -f myprogram *.o`

CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17

all: myprogram

myprogram: main.o load_balancer.o web_server.o request.o
	$(CXX) $(CXXFLAGS) -o myprogram main.o load_balancer.o web_server.o request.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

load_balancer.o: load_balancer.cpp load_balancer.h web_server.h request.h
	$(CXX) $(CXXFLAGS) -c load_balancer.cpp

web_server.o: web_server.cpp web_server.h request.h
	$(CXX) $(CXXFLAGS) -c web_server.cpp

request.o: request.cpp request.h
	$(CXX) $(CXXFLAGS) -c request.cpp

clean:
	rm -f myprogram *.o
