// This is what I assumed part 1 was lol.
// Summation is much more natural to me than unions, but I probably overcomplicated part1 with the traversal

#include <stdio.h>
#include <stdlib.h>

#include "../c-data-structures/vector/vector_template.h"

#define IN_FILE "input.txt"

// Helper to index a 1D array as 2D
#define IDX_2D(arr, row, col) arr[((row) * map_row_size) + (col)]

DEF_VEC(int)
DEF_VEC(int_Vec)
void deleteint_Vec_Vec(int_Vec_Vec *vec)
{
    for (size_t i = 0; i < vec->len; i++)
        free(vec->arr[i].arr);
    free(vec->arr);
    vec->arr = NULL;
    vec->len = 0;
    vec->cap = 0;
}

// A coordinate on the map
typedef struct Point
{
    short x;
    short y;
} Point;

typedef struct MapSquare
{
    // The length of reachable_peaks
    int reachable_peaks_size;
    // The height of this square
    int height;
} MapSquare;

int parse_input(int_Vec_Vec *map);
long long get_total_trailhead_rating(int_Vec_Vec map);
int get_reachable_peaks(MapSquare *map, size_t map_size, size_t map_row_size, size_t row, size_t col);

int main(int argc, char const *argv[])
{
    int is_error;
    int_Vec_Vec map;
    if (parse_input(&map))
    {
        is_error = 1;
        goto END;
    }

    printf("Total trailhead rating: %lld\n", get_total_trailhead_rating(map));

    // Cleanup
    is_error = 0;
    deleteint_Vec_Vec(&map);
END:
    return is_error;
}

/// @brief Parse input into a 2D vector of MapSquare
/// @param map Out: The vector of MapSquare
/// @return 0 if success, non-zero if failure
int parse_input(int_Vec_Vec *map)
{
    // Open input.txt or panic
    FILE *f = fopen(IN_FILE, "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    // Parse file
    *map = new_int_Vec_Vec();
    int ch;
    int_Vec row = new_int_Vec();
    while ((ch = getc(f)) != EOF)
    {
        if ('0' <= ch && ch <= '9')
        {
            append_int_Vec(&row, (int)(ch - '0'));
        }
        else if (ch == '\n')
        {
            append_int_Vec_Vec(map, row);
            row = new_int_Vec();
        }
        else
        {
            // Error: cleanup and return
            deleteint_Vec_Vec(map);
            return 1;
        }
    }
    // If relevant, append__Vec the last row. Otherwise, just free the row
    if (row.len > 0)
        append_int_Vec_Vec(map, row);
    else
        free(row.arr);

    return 0;
}

/// @brief The the total rating of all trailheads, according to day 10 part 1
/// @param map A 2d vector of map heights
/// @return The total rating of all trailheads in `map`
long long get_total_trailhead_rating(int_Vec_Vec map)
{
    long long total_rating = 0LL;
    // The number of rows if map
    size_t map_size = map.len;
    // The number of columns in map
    size_t map_row_size = map_size ? map.arr[0].len : 0;

    // Allocate a 1D array because we can to division and modulo to index it as 2D, similar to how static 2D arrays work
    size_t flattened_size = map_size * map_row_size;
    MapSquare *map_squares = malloc(flattened_size * sizeof(map_squares[0]));
    for (size_t row = 0; row < map_size; row++)
    {
        // Stop if the map is not rectangular
        if (map.arr[row].len != map_row_size)
        {
            fprintf(stderr, "Unexpected length for row %lu. Expected: %lu. Actual: %lu", row, map_row_size, map.arr[row].len);
            total_rating = -1;
            goto CLEANUP;
        }
        for (size_t col = 0; col < map_row_size; col++)
        {
            // -1 indicates this hasn't been calculated yet, while 0 would indicate no reachable peaks
            IDX_2D(map_squares, row, col).reachable_peaks_size = -1;
            IDX_2D(map_squares, row, col).height = map.arr[row].arr[col];
        }
    }

    // Loop over all map squares and check rating if the square is a trailhead
    for (size_t row = 0; row < map_size; row++)
        for (size_t col = 0; col < map_row_size; col++)
            // If trailhead, then count the reachable peaks
            if (IDX_2D(map_squares, row, col).height == 0)
            {
                int rating = get_reachable_peaks(map_squares, map_size, map_row_size, row, col);
                printf("Rating: of (%lu,%lu): %d\n", row, col, rating);
                total_rating += rating;
            }

    // Essentially using labels as defer
CLEANUP:
    // Free everything
    free(map_squares);

    return total_rating;
}

/// @brief Get all peaks reachable from this square
/// @param map The map of MapSquares
/// @param map_size The number of rows in `map`
/// @param map_row_size The number of columns in `map`
/// @param row The row to find the reachable peaks for
/// @param col The column to find the reachable peaks for
/// @return The number of distinct paths from the square at (row,col) to a peak
int get_reachable_peaks(MapSquare *map, size_t map_size, size_t map_row_size, size_t row, size_t col)
{
    // See if this has been memoized already
    if (IDX_2D(map, row, col).reachable_peaks_size != -1)
        return IDX_2D(map, row, col).reachable_peaks_size;

    int height = IDX_2D(map, row, col).height;
    // If this is a peak, return self
    if (height == 9)
        return 1;

    // Check neighbors
    // Increment height because we are looking at neighbors height, not this height.
    height++;
    int reachable_peaks = 0;
    // Check right
    if (col + 1 < map_row_size && IDX_2D(map, row, col + 1).height == height)
        reachable_peaks += get_reachable_peaks(map, map_size, map_row_size, row, col + 1);
    // Check up
    if (row > 0 && IDX_2D(map, row - 1, col).height == height)
        reachable_peaks += get_reachable_peaks(map, map_size, map_row_size, row - 1, col);
    // Check left
    if (col > 0 && IDX_2D(map, row, col - 1).height == height)
        reachable_peaks += get_reachable_peaks(map, map_size, map_row_size, row, col - 1);
    // Check down
    if (row + 1 < map_row_size && IDX_2D(map, row + 1, col).height == height)
        reachable_peaks += get_reachable_peaks(map, map_size, map_row_size, row + 1, col);

    return reachable_peaks;
}
