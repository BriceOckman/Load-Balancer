# Load Balancer Simulation - Brice Ockman, CSCE
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
SRC      = $(wildcard src/*.cpp)
OBJ      = $(SRC:.cpp=.o)
TARGET   = loadbalancer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: tests/test_main.cpp $(filter-out src/main.o,$(OBJ))
	$(CXX) $(CXXFLAGS) -o run_tests tests/test_main.cpp $(filter-out src/main.o,$(OBJ))
	./run_tests

docs:
	doxygen Doxyfile

clean:
	rm -f $(OBJ) $(TARGET) run_tests

.PHONY: all test docs clean
