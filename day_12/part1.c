#include <stdio.h>
#include <stdlib.h>

#include "../vector_template.h"

typedef char *string;

DEF_VEC(char)
DEF_VEC(string)
void delete_string_vec(stringVec *vec)
{
    for (int i = 0; i < vec->len; i++)
        free(vec->arr[i]);
    free(vec->arr);
    vec->arr = NULL;
    vec->len = 0UL;
    vec->cap = 0UL;
}

int parse_input(char *input_file, stringVec *map);
long long get_total_fencing(char **map, size_t map_size);
long long get_region_price(char **map, size_t map_size, int row, int col, long long *area);
void print_map(char **map, size_t map_size);
// Check is a row,col pair is valid on the map
int valid_point(char **map, size_t map_size, int row, int col) { return row >= 0 && col >= 0 && row < map_size && map[row][col]; }

int main(int argc, char *argv[])
{
    // Get input
    stringVec map;
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
int parse_input(char *input_file, stringVec *map)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *map = newstringVec();

    int ch;
    charVec row = newcharVec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch != '\n')
            // Anything other than EOF and newline can just get appended to the vector
            appendchar(&row, (char)ch);
        else
        {
            // Add terminator for the string and put it on the string vector
            appendchar(&row, '\0');
            appendstring(map, row.arr);
            // Generate a new char vector for the next iteration
            // Slight optimization: start with a capacity of this array's length
            row.arr = malloc(sizeof(row.arr[0]) * row.len);
            row.cap = row.len;
            row.len = 0UL;
        }
    }

    // If relevant, append the last row. Otherwise, just free the row
    if (row.len > 0)
    {
        appendchar(&row, '\0');
        appendstring(map, row.arr);
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
/// @return The total price (sum of perimeter*area for each region)
long long get_total_fencing(char **map, size_t map_size)
{
    long long total = 0LL;
    long long perimeter;
    long long area;
    for (int i = 0; i < map_size; i++)
    {
        for (int j = 0; map[i][j]; j++)
        {
            area = 0LL;
            perimeter = get_region_price(map, map_size, i, j, &area);
            total += perimeter * area;
        }
    }
    return total;
}

/// @brief Find the price of a region containing map[row][col]
/// @param map The map
/// @param map_size The number of rows in the map
/// @param row The row to look at
/// @param col The column to look at
/// @param area Out: The total area of this map. This will be added to, not replaced
/// @return The perimeter
long long get_region_price(char **map, size_t map_size, int row, int col, long long *area)
{
    // Ignore squares that have already been found and squares that don't exist
    if ((map[row][col] & (char)0x80) || !map[row][col])
        return 0;

    // Set this as found by setting the most significant bit, but save this character for comparisons
    // This will never be set in an actual ASCII character
    char this_before_found = map[row][col];
    map[row][col] |= (char)0x80;

    long long perimeter = 0LL;

// Check a side
#define CHECK_SIDE(r, c)                                                                  \
    do                                                                                    \
    {                                                                                     \
        if (!valid_point(map, map_size, r, c) || (map[r][c] & 0x7f) != this_before_found) \
            /* If this side is on the edge of the map,                                    \
            or a character that is not this character, add a fence */                     \
            perimeter++;                                                                  \
        else                                                                              \
            /* If not edge and the same character, check that character */                \
            perimeter += get_region_price(map, map_size, r, c, area);                     \
    } while (0)

    // Right
    CHECK_SIDE(row, col + 1);
    // Up
    CHECK_SIDE(row - 1, col);
    // Left
    CHECK_SIDE(row, col - 1);
    // Down
    CHECK_SIDE(row + 1, col);

#undef CHECK_SIDE
    (*area)++;
    return perimeter;
}
