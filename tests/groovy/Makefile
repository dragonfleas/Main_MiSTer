# Host-side (x86) unit tests for the Groovy scandoubler decision module.
# Not part of the ARM build. Requires libcriterion-dev.
#   make run
CC       ?= gcc
CXX      ?= g++
WARN     := -Wall -Wextra
CRIT_CFLAGS := $(shell pkg-config --cflags criterion)
CRIT_LIBS   := $(shell pkg-config --libs criterion)

BIN := groovy_tests

run: $(BIN)
	./$(BIN) -j1

$(BIN): test_scandoubler.o scandoubler.o
	$(CXX) -o $@ $^ $(CRIT_LIBS)

test_scandoubler.o: test_scandoubler.c ../../support/groovy/scandoubler.h
	$(CC) -O0 -g $(WARN) $(CRIT_CFLAGS) -I../.. -c -o $@ test_scandoubler.c

scandoubler.o: ../../support/groovy/scandoubler.cpp ../../support/groovy/scandoubler.h
	$(CXX) -O0 -g $(WARN) -I../.. -c -o $@ ../../support/groovy/scandoubler.cpp

clean:
	rm -f $(BIN) *.o

.PHONY: run clean
