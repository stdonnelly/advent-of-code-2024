#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../c-data-structures/vector/vector_template.h"

// Helper to index a 1D array as 2D
#define IDX_2D(arr, row, col) arr[((row) * map_row_size) + (col)]

typedef char *string;

DEF_VEC(char)
DEF_VEC(string)
void delete_string_vec(string_Vec *vec)
{
    for (int i = 0; i < vec->len; i++)
        free(vec->arr[i]);
    free(vec->arr);
    vec->arr = NULL;
    vec->len = 0UL;
    vec->cap = 0UL;
}

// Plant is found
#define FOUND (char)0b1
// Plant has right fence
#define R_FENCE (char)0b10
// Plant has up fence6
#define U_FENCE (char)0b100
// Plant has left fence
#define L_FENCE (char)0b1000
// Plant has down fence
#define D_FENCE (char)0b10000

typedef struct Plant
{
    char ch;
    // A bitset of attributes
    char attr;
} Plant;

int parse_input(char *input_file, string_Vec *map);
long long get_total_fencing(char **map, size_t map_size);
long long get_region_price(Plant *map, size_t map_size, size_t map_row_size, int row, int col, long long *area);
void print_map(char **map, size_t map_size);
// Check is a row,col pair is valid on the map
int valid_point(size_t map_size, size_t map_row_size, int row, int col) { return row >= 0 && col >= 0 && row < map_size && col < map_row_size; }
int add_fence(Plant *map, size_t map_size, size_t map_row_size, int row, int col, char fence_location);

int main(int argc, char *argv[])
{
    // Get input
    string_Vec map;
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    if (parse_input(input_file, &map))
        return 1;

    long long total_price = get_total_fencing(map.arr, map.len);
    print_map(map.arr, map.len);

    printf("\e[37m\nTotal price: %lld\n", total_price);

    // Clean up
    delete_string_vec(&map);
    return 0;
}

/// @brief Parse the input file into `map`
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param map Out: the file after it's loaded
/// @return 0 if success, non-zero if failure
int parse_input(char *input_file, string_Vec *map)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *map = new_string_Vec();

    int ch;
    char_Vec row = new_char_Vec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch != '\n')
            // Anything other than EOF and newline can just get append_ed_Vec to the vector
            append_char_Vec(&row, (char)ch);
        else
        {
            // Add terminator for the string and put it on the string vector
            append_char_Vec(&row, '\0');
            append_string_Vec(map, row.arr);
            // Generate a new char vector for the next iteration
            // Slight optimization: start with a capacity of this array's length
            row.arr = malloc(sizeof(row.arr[0]) * row.len);
            row.cap = row.len;
            row.len = 0UL;
        }
    }

    // If relevant, append__Vec the last row. Otherwise, just free the row
    if (row.len > 0)
    {
        append_char_Vec(&row, '\0');
        append_string_Vec(map, row.arr);
    }
    else
        free(row.arr);

    return 0;
}

/// @brief Print the map to stdout, graying out characters with the most significant bit set
/// @param map The map
/// @param map_size The number of rows in `map`
void print_map(char **map, size_t map_size)
{
    char last_char = '\0';
    for (size_t i = 0; i < map_size; i++)
    {
        char *row = map[i];
        while (*row)
        {
            // ch & 0x80 indicates used, which should be grayed out unless this is already grayed out
            if ((*row & (char)0x80) && !(last_char & (char)0x80))
                printf("\e[90m");
            // Reset back to white if the last character was gray
            else if (!(*row & (char)0x80) && (last_char & (char)0x80))
                printf("\e[37m");
            last_char = *row;
            putchar(*row & (char)0x7f);
            row++;
        }
        putchar('\n');
    }
}

/// @brief Get the total price of fencing for a given map
/// @param map A map of characters representing garden squares
/// @param map_size The number of rows in `map`
/// @return The total price (sum of size_count*area for each region)
long long get_total_fencing(char **map, size_t map_size)
{
    size_t map_row_size = map_size ? strlen(map[0]) : 0UL;
    // Allocate a 1D array because we can to division and modulo to index it as 2D, similar to how static 2D arrays work
    Plant *map_plants = malloc(sizeof(map_plants[0]) * map_size * (map_size ? strlen(map[0]) : 0UL));
    long long total = 0LL;
    long long size_count;
    long long area;

    // Populate map plants array
    for (size_t row = 0; row < map_size; row++)
    {
        // Stop if the map is not rectangular
        if (strlen(map[row]) != map_row_size)
        {
            // Make the cleanup only clean up rows that actually have been written or the cleanup may cause nonexistant memory to be freed.
            fprintf(stderr, "Unexpected length for row '%lu'. Expected: %lu. Actual: %lu", row, map_row_size, strlen(map[row]));
            total = -1;
            goto CLEANUP;
        }
        for (size_t col = 0; col < map_row_size; col++)
        {
            IDX_2D(map_plants, row, col).ch = map[row][col];
            IDX_2D(map_plants, row, col).attr = (char)0;
        }
    }

    // Calculate per region price
    for (int i = 0; i < map_size; i++)
    {
        for (int j = 0; map[i][j]; j++)
        {
            area = 0LL;
            size_count = get_region_price(map_plants, map_size, map_row_size, i, j, &area);
            total += size_count * area;
        }
    }

CLEANUP:
    free(map_plants);
    return total;
}

/// @brief Find the price of a region containing map[row][col]
/// @param map The map
/// @param map_size The number of rows in the map
/// @param row The row to look at
/// @param col The column to look at
/// @param area Out: The total area of this map. This will be added to, not replaced
/// @return The size_count
long long get_region_price(Plant *map, size_t map_size, size_t map_row_size, int row, int col, long long *area)
{
    // Ignore squares that have already been found
    if (IDX_2D(map, row, col).attr & FOUND)
        return 0;

    // Get the character and set this as found
    char ch = IDX_2D(map, row, col).ch;
    IDX_2D(map, row, col).attr |= FOUND;

    long long size_count = 0LL;

// Check a side
#define CHECK_SIDE(r, c, f)                                                           \
    do                                                                                \
    {                                                                                 \
        if (!valid_point(map_size, map_row_size, r, c) || IDX_2D(map, r, c).ch != ch) \
        {                                                                             \
            /* If this side is on the edge of the map,                                \
            or a character that is not this character, add a fence */                 \
            size_count += add_fence(map, map_size, map_row_size, row, col, f);        \
        }                                                                             \
        else                                                                          \
            /* If not edge and the same character, check that character */            \
            size_count += get_region_price(map, map_size, map_row_size, r, c, area);  \
    } while (0)

    // Right
    CHECK_SIDE(row, col + 1, R_FENCE);
    // Up
    CHECK_SIDE(row - 1, col, U_FENCE);
    // Left
    CHECK_SIDE(row, col - 1, L_FENCE);
    // Down
    CHECK_SIDE(row + 1, col, D_FENCE);

#undef CHECK_SIDE
    (*area)++;
    return size_count;
}

/// @brief Add a fence to one size of a given plant
/// @param map The map
/// @param map_size The number of rows in map
/// @param map_row_size The number of columns in map
/// @param row The row to add the fence to
/// @param col The column to add the fence to
/// @param fence_location The location (right, up, left, down) of the new fence
/// @return The number of sides created, 1, 0, or -1 (for removal of a side, not sure if this one is actually possible)
int add_fence(Plant *map, size_t map_size, size_t map_row_size, int row, int col, char fence_location)
{
    // Add the fence
    char ch = IDX_2D(map, row, col).ch;
    IDX_2D(map, row, col).attr |= fence_location;
    // Assume one side was created
    int sides_created = 1;

    if (fence_location & (L_FENCE | R_FENCE))
    {
        // If left or right, check up and down for existing fences.
        // If there are existing fences on the given side, this operation does not add a side
        if (valid_point(map_size, map_row_size, row - 1, col) && (IDX_2D(map, row - 1, col).ch == ch) && (IDX_2D(map, row - 1, col).attr & fence_location))
            sides_created--;
        if (valid_point(map_size, map_row_size, row + 1, col) && (IDX_2D(map, row + 1, col).ch == ch) && (IDX_2D(map, row + 1, col).attr & fence_location))
            sides_created--;
    }

    if (fence_location & (U_FENCE | D_FENCE))
    {
        // If up or down, check left and right for existing fences.
        // If there are existing fences on the given side, this operation does not add a side
        if (valid_point(map_size, map_row_size, row, col - 1) && (IDX_2D(map, row, col - 1).ch == ch) && (IDX_2D(map, row, col - 1).attr & fence_location))
            sides_created--;
        if (valid_point(map_size, map_row_size, row, col + 1) && (IDX_2D(map, row, col + 1).ch == ch) && (IDX_2D(map, row, col + 1).attr & fence_location))
            sides_created--;
    }

    return sides_created;
}
