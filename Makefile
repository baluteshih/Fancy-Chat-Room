CXXFLAGS = -O2 -std=c++17 -Wall
SRCS = src/helper.cpp src/http.cpp src/socket.cpp
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

test: test.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INC) $(OBJS) test.cpp -o test

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -rf $(OBJS) .depend test
