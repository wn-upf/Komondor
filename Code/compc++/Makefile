include ../optimizations.mk
include ../definitions.mk

SCAN_FILE=lexer
OTHER_FILES=token parser
OBJS= $(SCAN_FILE).o $(OTHER_FILES:%=%.o)

../bin/cxx:   $(OBJS)
	$(CC) -o ../bin/cxx  $(OBJS) -lstdc++

$(SCAN_FILE).cpp: lexer.l
	flex -o$@ $<

$(OBJS): %.o: %.cpp token.h
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(SCAN_FILE).cpp

