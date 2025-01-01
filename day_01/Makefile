.PHONY: clean

CXX = gcc
DEBUG_FLAGS = -Wall -fsanitize=address -g3

all: part1 part2

part1: part1.c ../c-data-structures/merge_sort/libmerge_sort.a
	$(CXX) $(DEBUG_FLAGS) $< -L../c-data-structures/merge_sort -lmerge_sort -o $@

part2: part2.c ../c-data-structures/merge_sort/libmerge_sort.a
	$(CXX) $(DEBUG_FLAGS) $< -L../c-data-structures/merge_sort -lmerge_sort -o $@

../c-data-structures/merge_sort/libmerge_sort.a:
	$(MAKE) -C ../c-data-structures/merge_sort libmerge_sort.a

clean:
	rm -f part1 part2 *.o *.a
