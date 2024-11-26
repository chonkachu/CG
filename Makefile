CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lglfw -lGL -lGLEW -lGLU -lpthread -ldl 

CXXFILES = $(wildcard *.cpp)
CXXOBJS = $(patsubst %.cpp, %.o, $(CXXFILES))

all: $(CXXOBJS)
	$(CXX) $(CXXFLAGS) $(CXXOBJS) -o main $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o main
