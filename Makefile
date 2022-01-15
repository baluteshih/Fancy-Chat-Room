CXXFLAGS = -O2 -std=c++17 -Wall -pthread
SRCS = src/helper.cpp src/http.cpp src/socket.cpp src/main_thread.cpp src/file.cpp src/db.cpp src/server_object.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
INC = -I src

all: $(OBJS)

ifeq ($(DEBUG),1)
   CXXFLAGS += -DDEBUG -g
endif

depend: .depend

.depend: $(SRCS)
	rm -f $@
	$(CXX) $(CXXFLAGS) -MM $^ -MF $@

include .depend

server: server.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INC) -c server.cpp -o server.o
	$(CXX) $(CXXFLAGS) $(INC) $(OBJS) server.o -o server

test: test.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INC) $(OBJS) test.cpp -o test

http_test: http_test.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INC) $(OBJS) http_test.cpp -o http_test

db_test: db_test.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INC) $(OBJS) db_test.cpp -o db_test -lsqlite3

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -rf $(OBJS) .depend test
