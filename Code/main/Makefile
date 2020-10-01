SHELL := /bin/bash
CC = g++
CDEBUG = -g
CCFLAGS = -Wall
BIN = komondor_main
COMPCPP_DIR = ../compc++
COMPCPP_BIN = $(COMPCPP_DIR)/cxx

.PHONY: all
all: $(BIN)

$(BIN): $(BIN).cxx
	$(CC) $(CCFLAGS) -o $@ $<

%.cxx: %.cc $(COMPCPP_BIN)
	$(COMPCPP_BIN) $<

$(COMPCPP_BIN):
	$(MAKE) -C $(COMPCPP_DIR)

.PHONY: clean
clean:
	rm -rf $(BIN) $(BIN).cxx
