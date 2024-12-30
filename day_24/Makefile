CXX = gcc
DEBUG_FLAGS = -Wall -fsanitize=address -g3
OPTIMIZE_FLAGS = -O2

part1-sample :
	$(CXX) $(DEBUG_FLAGS) -o part1 part1.c
	./part1 sample_input.txt

part1-final :
	$(CXX) $(DEBUG_FLAGS) -o part1 part1.c
	./part1 input.txt

part2-sample :
	$(CXX) $(DEBUG_FLAGS) -lz -o part2 part2.c
	./part2 sample_input.txt

part2-final :
	$(CXX) $(DEBUG_FLAGS) -lz -o part2 part2.c
	./part2 input.txt
