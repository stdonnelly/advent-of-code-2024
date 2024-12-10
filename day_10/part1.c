#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"

#define IN_FILE "input.txt"

// Helper to index a 1D array as 2D
#define IDX_2D(arr, row, col) arr[((row) * map_row_size) + (col)]

DEF_VEC(int)
DEF_VEC(intVec)
void deleteintVecVec(intVecVec *vec)
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
    // An array of positions of peaks that are reachable from this square
    Point *reachable_peaks;
    // The length of reachable_peaks
    int reachable_peaks_size;
    // The height of this square
    int height;
} MapSquare;

int parse_input(intVecVec *map);
long long get_total_trailhead_score(intVecVec map);
Point *get_reachable_peaks(MapSquare *map, size_t map_size, size_t map_row_size, size_t row, size_t col, int *return_size);
Point *union_point_arrays(Point *arr1, int arr1_size, Point *arr2, int arr2_size, int *return_size);

int main(int argc, char const *argv[])
{
    int is_error;
    intVecVec map;
    if (parse_input(&map))
    {
        is_error = 1;
        goto END;
    }

    printf("Total trailhead score: %lld\n", get_total_trailhead_score(map));

    // Cleanup
    is_error = 0;
    deleteintVecVec(&map);
END:
    return is_error;
}

/// @brief Parse input into a 2D vector of MapSquare
/// @param map Out: The vector of MapSquare
/// @return 0 if success, non-zero if failure
int parse_input(intVecVec *map)
{
    // Open input.txt or panic
    FILE *f = fopen(IN_FILE, "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    // Parse file
    *map = newintVecVec();
    int ch;
    intVec row = newintVec();
    while ((ch = getc(f)) != EOF)
    {
        if ('0' <= ch && ch <= '9')
        {
            appendint(&row, (int)(ch - '0'));
        }
        else if (ch == '\n')
        {
            appendintVec(map, row);
            row = newintVec();
        }
        else
        {
            // Error: cleanup and return
            deleteintVecVec(map);
            return 1;
        }
    }
    // If relevant, append the last row. Otherwise, just free the row
    if (row.len > 0)
        appendintVec(map, row);
    else
        free(row.arr);

    return 0;
}

/// @brief The the total score of all trailheads, according to day 10 part 1
/// @param map A 2d vector of map heights
/// @return The total score of all trailheads in `map`
long long get_total_trailhead_score(intVecVec map)
{
    long long total_score = 0LL;
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
            // Make the cleanup only clean up rows that actually have been written or the cleanup may cause nonexistant memory to be freed.
            flattened_size = row * map_row_size;
            fprintf(stderr, "Unexpected length for row %lu. Expected: %lu. Actual: %lu", row, map_row_size, map.arr[row].len);
            total_score = -1;
            goto CLEANUP;
        }
        for (size_t col = 0; col < map_row_size; col++)
        {
            IDX_2D(map_squares, row, col).reachable_peaks = NULL;
            // -1 indicates this hasn't been calculated yet, while 0 would indicate no reachable peaks
            IDX_2D(map_squares, row, col).reachable_peaks_size = -1;
            IDX_2D(map_squares, row, col).height = map.arr[row].arr[col];
        }
    }

    // Loop over all map squares and check score if the square is a trailhead
    for (size_t row = 0; row < map_size; row++)
    {
        for (size_t col = 0; col < map_row_size; col++)
        {
            // If trailhead, then count the reachable peaks
            if (IDX_2D(map_squares, row, col).height == 0)
            {
                int score;
                get_reachable_peaks(map_squares, map_size, map_row_size, row, col, &score);
                total_score += score;
            }
        }
    }

    // Essentially using labels as defer
CLEANUP:
    // Free everything
    for (size_t i = 0; i < flattened_size; i++)
        if (map_squares[i].reachable_peaks_size > 0)
            free(map_squares[i].reachable_peaks);
    free(map_squares);

    return total_score;
}

/// @brief Get all peaks reachable from this square
/// @param map The map of MapSquares
/// @param map_size The number of rows in `map`
/// @param map_row_size The number of columns in `map`
/// @param row The row to find the reachable peaks for
/// @param col The column to find the reachable peaks for
/// @param return_size Out: The size of the returned array
/// @return An array of points representing all the peaks reachable from this square
Point *get_reachable_peaks(MapSquare *map, size_t map_size, size_t map_row_size, size_t row, size_t col, int *return_size)
{
    // See if this has been memoized already
    if (IDX_2D(map, row, col).reachable_peaks_size != -1)
    {
        *return_size = IDX_2D(map, row, col).reachable_peaks_size;
        // Do not memoize this because we already have
        return IDX_2D(map, row, col).reachable_peaks;
    }

    Point *reachable_peaks;
    int height = IDX_2D(map, row, col).height;
    // If this is a peak, return self
    if (height == 9)
    {
        reachable_peaks = malloc(sizeof(reachable_peaks[0]));
        reachable_peaks[0].x = (short)row;
        reachable_peaks[0].y = (short)col;

        *return_size = 1;
        // Memoize and return
        goto MEMOIZE_RETURN;
    }

    // Check neighbors
    // Increment height because we are looking at neighbors height, not this height.
    height++;
    reachable_peaks = NULL;
    *return_size = 0;
    Point *neighbor_peaks;
    int neighbor_peaks_size;
    // Check right
    if (col + 1 < map_row_size && IDX_2D(map, row, col + 1).height == height)
    {
        neighbor_peaks = get_reachable_peaks(map, map_size, map_row_size, row, col + 1, &neighbor_peaks_size);
        reachable_peaks = union_point_arrays(reachable_peaks, *return_size, neighbor_peaks, neighbor_peaks_size, return_size);
    }
    // Check up
    if (row > 0 && IDX_2D(map, row - 1, col).height == height)
    {
        neighbor_peaks = get_reachable_peaks(map, map_size, map_row_size, row - 1, col, &neighbor_peaks_size);
        reachable_peaks = union_point_arrays(reachable_peaks, *return_size, neighbor_peaks, neighbor_peaks_size, return_size);
    }
    // Check left
    if (col > 0 && IDX_2D(map, row, col - 1).height == height)
    {
        neighbor_peaks = get_reachable_peaks(map, map_size, map_row_size, row, col - 1, &neighbor_peaks_size);
        reachable_peaks = union_point_arrays(reachable_peaks, *return_size, neighbor_peaks, neighbor_peaks_size, return_size);
    }
    // Check down
    if (row + 1 < map_row_size && IDX_2D(map, row + 1, col).height == height)
    {
        neighbor_peaks = get_reachable_peaks(map, map_size, map_row_size, row + 1, col, &neighbor_peaks_size);
        reachable_peaks = union_point_arrays(reachable_peaks, *return_size, neighbor_peaks, neighbor_peaks_size, return_size);
    }

MEMOIZE_RETURN:
    IDX_2D(map, row, col).reachable_peaks = reachable_peaks;
    IDX_2D(map, row, col).reachable_peaks_size = *return_size;
    return reachable_peaks;
}

/// @brief Find the union of arr1 and arr2
/// @param arr1 The first array. If the second array is [], this will be returned with no cloning. This array will be freed or directly returned by this process
/// @param arr1_size The number of elements in `arr1`
/// @param arr2 The second array. This one will never be freed by this function.
/// @param arr2_size The number of elements in `arr2`
/// @param return_size Out: The size of the returned array, will always be <= arr1_size + arr2_size
/// @return An ordered array which is the set union of the two input arrays
Point *union_point_arrays(Point *arr1, int arr1_size, Point *arr2, int arr2_size, int *return_size)
{
    Point *arr_union;
    if (!arr2_size)
    {
        // If there are no elements in the second array, do nothing to the first one
        arr_union = arr1;
        *return_size = arr1_size;
        goto END;
    }
    else if (!arr1_size)
    {
        // If there are elements in the second array, but not the first one, just clone the second array
        arr_union = realloc(arr1, sizeof(arr_union[0]) * arr2_size);
        memcpy(arr_union, arr2, sizeof(arr_union[0]) * arr2_size);
        *return_size = arr2_size;
        goto END;
    }

    // Merge the arrays
    // Start by assuming the maximum. The worst case is that the arrays are equal, resulting in twice the required memory allocation.
    *return_size = arr1_size + arr2_size;
    arr_union = malloc(sizeof(arr_union[0]) * (*return_size));
    int left_cursor = 0;
    int right_cursor = 0;
    int destination_cursor = 0;
    while (left_cursor < arr1_size && right_cursor < arr2_size)
    {
        int comparison = memcmp(&(arr1[left_cursor]), &(arr2[right_cursor]), sizeof(arr1[0]));
        // Put the first in lexicographical order into the array
        // This decision is arbitrary, but consistency is necessary
        if (comparison < 0)
            arr_union[destination_cursor++] = arr1[left_cursor++];
        else if (comparison > 0)
            arr_union[destination_cursor++] = arr2[right_cursor++];
        else
        {
            // Use left but increment both input cursors
            arr_union[destination_cursor++] = arr1[left_cursor++];
            right_cursor++;
            // And decrease the return size because we just ignored one element
            (*return_size)--;
        }
    }

    while (left_cursor < arr1_size)
        arr_union[destination_cursor++] = arr1[left_cursor++];
    while (right_cursor < arr2_size)
        arr_union[destination_cursor++] = arr2[right_cursor++];

    free(arr1);
END:
    return arr_union;
}
