SHELL := /bin/bash
COST_WRAPPER = .././COST/cxx
CC = g++
CXXFLAGS = -Wall -g -std=c++98

# Robust Windows Detection
ifdef OS
   EXEC_EXT = .exe
else
   ifdef COMSPEC
      EXEC_EXT = .exe
   else
      EXEC_EXT =
   endif
endif

# File names
SRC_CC = komondor_main.cc
SRC_CXX = komondor_main.cxx
BIN = komondor_main$(EXEC_EXT)

# --- FIX: Track all source/header files ---
# This grabs every .cc and .h file in the current folder
PROJECT_SOURCES = $(wildcard *.cc *.h)

.PHONY: all clean

all: $(BIN)

# Build the executable
# We add $(PROJECT_SOURCES) here so if ANY file changes, it rebuilds
$(BIN): $(SRC_CXX) $(PROJECT_SOURCES)
	@echo "==> Compiling generated C++ code..."
	$(CC) $(CXXFLAGS) -o $@ $(SRC_CXX)
	@echo "==> Build complete: $(BIN)"

# Build the generated .cxx file
# We also add them here, in case the code generator uses other files
$(SRC_CXX): $(SRC_CC) $(COST_WRAPPER) $(PROJECT_SOURCES)
	@echo "==> Generating C++ code from .cc file..."
	rm -f $(SRC_CXX)
	$(COST_WRAPPER) $(SRC_CC)

clean:
	@echo "==> Cleaning up..."
	rm -rf $(BIN) $(SRC_CXX)