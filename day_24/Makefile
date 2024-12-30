CXX = gcc
DEBUG_FLAGS = -Wall -fsanitize=address -g3
OPTIMIZE_FLAGS = -O2

part1-sample : part1.c sample_input.txt
	$(CXX) $(DEBUG_FLAGS) -o part1 part1.c
	./part1 sample_input.txt

part1-final : part1.c input.txt
	$(CXX) $(DEBUG_FLAGS) -o part1 part1.c
	./part1 input.txt

part2-sample : part2.c sample_input.txt
	$(CXX) $(DEBUG_FLAGS) -lz -o part2 part2.c
	./part2 sample_input.txt

part2-final : part2.c input.txt 45-bit_adder.txt
	$(CXX) $(DEBUG_FLAGS) -lz -o part2 part2.c
	./part2 input.txt

generate_adder : generate_adder.c
	$(CXX) $(DEBUG_FLAGS) -o generate_adder generate_adder.c
	./generate_adder
