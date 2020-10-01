# optimization flags
## Fast optimization that is machine independent.
OPTIMIZATION = -O3

## Optimization to use when debugging
#OPTIMIZATION = -O0 -ggdb

CC = g++
CCFLAGS = -Wall $(OPTIMIZATION)

SCAN_FILE=lexer
OTHER_FILES=token parser
OBJS= $(SCAN_FILE).o $(OTHER_FILES:%=%.o)
BIN=./cxx

$(BIN):   $(OBJS)
	$(CC) -o $(BIN)  $(OBJS) -lstdc++

$(SCAN_FILE).cpp: lexer.l
	flex -o$@ $<

$(OBJS): %.o: %.cpp token.h
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(SCAN_FILE).cpp $(BIN)

