.PHONY: clean

CXX = gcc
DEBUG_FLAGS = -Wall -fsanitize=address -g3

all: part1

part1: part1.c
	$(CXX) $(DEBUG_FLAGS) $< -o $@

clean:
	rm -f part1 *.o *.a
