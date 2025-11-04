SHELL := /bin/bash
COST_WRAPPER = .././COST/cxx # Path to the .cxx file from COST (code generator)
CC = g++	# Compiler used
CXXFLAGS = -Wall -g -std=c++98 # Compiler flags

# File names
SRC_CC = komondor_main.cc
SRC_CXX = komondor_main.cxx
BIN = komondor_main

.PHONY: all
all: $(BIN)

# Build the executable
$(BIN): $(SRC_CXX)
	@echo "==> Compiling generated C++ code..."
	$(CC) $(CXXFLAGS) -o $@ $(SRC_CXX)
	@echo "==> Build complete: $(BIN)"

# Build the generated .cxx file
$(SRC_CXX): $(SRC_CC) $(COST_WRAPPER)
	@echo "==> Generating C++ code from .cc file..."
	$(COST_WRAPPER) $(SRC_CC)

# Clean files
.PHONY: clean
clean:
	@echo "==> Cleaning up..."
	rm -rf $(BIN) $(SRC_CXX)