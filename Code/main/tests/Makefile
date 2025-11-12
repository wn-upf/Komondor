# Makefile for unit tests

CXX = g++
CXXFLAGS = -Wall -Werror -g -std=c++11 -I.. -I../../methods -I../../structures -I../../list_of_macros.h

# Test executables
TESTS = test_node_state_manager

# Source files
TEST_FRAMEWORK_SRC = test_framework.cpp
TEST_STATE_MANAGER_SRC = test_node_state_manager.cpp

# Object files
TEST_FRAMEWORK_OBJ = test_framework.o
TEST_STATE_MANAGER_OBJ = test_node_state_manager.o

# Default target
all: $(TESTS)

# Build test framework
test_framework.o: test_framework.cpp test_framework.h
	$(CXX) $(CXXFLAGS) -c test_framework.cpp -o test_framework.o

# Build state manager tests
test_node_state_manager: test_framework.o test_node_state_manager.o
	$(CXX) $(CXXFLAGS) -o test_node_state_manager test_framework.o test_node_state_manager.o

test_node_state_manager.o: test_node_state_manager.cpp test_framework.h ../node_state_manager.h
	$(CXX) $(CXXFLAGS) -c test_node_state_manager.cpp -o test_node_state_manager.o

# Run tests
test: all
	@echo "Running unit tests..."
	@./test_node_state_manager

# Clean
clean:
	rm -f $(TESTS) *.o

.PHONY: all test clean

