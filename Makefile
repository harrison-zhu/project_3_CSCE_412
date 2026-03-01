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

myprogram: main.o LoadBalancer.o WebServer.o Request.o
	$(CXX) $(CXXFLAGS) -o myprogram main.o LoadBalancer.o WebServer.o Request.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

LoadBalancer.o: LoadBalancer.cpp
	$(CXX) $(CXXFLAGS) -c LoadBalancer.cpp

WebServer.o: WebServer.cpp
	$(CXX) $(CXXFLAGS) -c WebServer.cpp

Request.o: Request.cpp
	$(CXX) $(CXXFLAGS) -c Request.cpp

clean:
	rm -f myprogram *.o
