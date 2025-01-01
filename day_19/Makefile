.PHONY: clean

CXX = gcc
DEBUG_FLAGS = -Wall -fsanitize=address -g3
LINKER_FLAGS = -L../c-data-structures/hash_map -lhash_map

all: part1 part2

part1: part1.c ../c-data-structures/merge_sort/libhash_map.a
	$(CXX) $(DEBUG_FLAGS) $< $(LINKER_FLAGS) -o $@

part2: part2.c ../c-data-structures/merge_sort/libhash_map.a
	$(CXX) $(DEBUG_FLAGS) $< $(LINKER_FLAGS) -o $@

../c-data-structures/merge_sort/libhash_map.a:
	$(MAKE) -C ../c-data-structures/hash_map libhash_map.a

clean:
	rm -f part1 part2 *.o *.a
