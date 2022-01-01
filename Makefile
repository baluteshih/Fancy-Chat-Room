CXXFLAGS = -O2 -std=c++17 -Wall -std=c++17
SRCS = helper.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

all: $(OBJS)

depend: .depend

.depend: $(SRCS)
	rm -f $@
	$(CXX) $(CXXFLAGS) -MM $^ -MF $@

include .depend

%.o: %.cpp
	$(CXX) -c $< 
clean:
	rm -rf *.o .depend
