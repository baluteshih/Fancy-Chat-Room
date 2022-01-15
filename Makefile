CXXFLAGS = -O2 -std=c++17 -Wall -pthread -lsqlite3
SRCS = src/helper.cpp src/http.cpp src/socket.cpp src/main_thread.cpp src/file.cpp src/db.cpp src/server_object.cpp src/crypto.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
DEPS = $(patsubst %.o, %.d, $(OBJS))
INC = -I src

all: $(OBJS)

ifeq ($(DEBUG),1)
   CXXFLAGS += -DDEBUG -g
endif

#depend: .depend

#.depend: $(SRCS)
#	rm -f $@
#	$(CXX) $(CXXFLAGS) -MM $^ -MF $@

-include $(DEPS)

server: server.cpp $(OBJS)
	$(CXX) $(INC) -DSERVER -c server.cpp -o server.o $(CXXFLAGS)
	$(CXX) $(INC) -DSERVER $(OBJS) server.o -o server $(CXXFLAGS)

client: client.cpp $(OBJS)
	$(CXX) $(INC) -DCLIENT -c client.cpp -o client.o $(CXXFLAGS) 
	$(CXX) $(INC) -DCLIENT $(OBJS) client.o -o client $(CXXFLAGS)

test: test.cpp $(OBJS)
	$(CXX) $(INC) $(OBJS) test.cpp -o test $(CXXFLAGS)

http_test: http_test.cpp $(OBJS)
	$(CXX) $(INC) $(OBJS) http_test.cpp -o http_test $(CXXFLAGS)

db_test: db_test.cpp $(OBJS)
	$(CXX) $(INC) $(OBJS) db_test.cpp -o db_test $(CXXFLAGS)

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM $< -MF $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(DEPS) test http_test db_test
