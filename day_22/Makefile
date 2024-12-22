CXX = gcc
DEBUG_FLAGS = -Wall -fsanitize=address -g3
OPTIMIZE_FLAGS = -O2
LINKER_FLAGS =

part1-sample :
	$(CXX) $(DEBUG_FLAGS) $(LINKER_FLAGS) -o part1 part1.c
	./part1 p1_sample_input.txt

part1-final :
	$(CXX) $(DEBUG_FLAGS) $(LINKER_FLAGS) -o part1 part1.c
	./part1 input.txt

part2-sample :
	$(CXX) $(DEBUG_FLAGS) $(LINKER_FLAGS) -o part2 part2.c
	./part2 p2_sample_input.txt

part2-final :
	$(CXX) $(DEBUG_FLAGS) $(LINKER_FLAGS) -o part2 part2.c
	./part2 input.txt
