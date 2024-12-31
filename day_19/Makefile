.PHONY: clean

CXX = gcc
DEBUG_FLAGS = -Wall -fsanitize=address -g3
LINKER_FLAGS = -lz

all: part1 part2

part1: part1.c
	$(CXX) $(DEBUG_FLAGS) $(LINKER_FLAGS) -o part1 part1.c

part2: part2.c
	$(CXX) $(DEBUG_FLAGS) $(LINKER_FLAGS) -o part2 part2.c

clean:
	rm -f part1 part2 *.o *.a

