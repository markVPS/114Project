CXX = g++
CXXFLAGS = -std=c++17 -Wall

SRC = src/main.cpp src/PCB.cpp src/Scheduler.cpp src/MemoryManager.cpp src/ResourceManager.cpp src/Simulator.cpp

OUT = os_sim

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)