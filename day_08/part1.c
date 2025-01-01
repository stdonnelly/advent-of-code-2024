#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../c-data-structures/vector/vector_template.h"

// Define point using shorts instead of int or size_t because the real input is only 50x50
// This allows us to make other structures that are smaller than 64-bit for efficiency
typedef struct Point
{
    short row;
    short col;
} Point;

DEF_VEC(Point)

// A map from a frequency identifier, signified by a number, uppercase letter, or lowercase letter
typedef struct FrequencyMap
{
    Point_Vec table[10 + 26 + 26];
} FrequencyMap;

// Define char and char* vectors
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

int parse_input(string_Vec *map, FrequencyMap *antenna_map);
void print_map(char **map, size_t row_count);
FrequencyMap newFrequencyMap();
void deleteFrequencyMap(FrequencyMap *map);
int add_antenna(FrequencyMap *map, char frequency, Point antenna);
int frequency_map_index(char frequency);
int choose_2_doubled(int n);
int count_antinodes(char **map, size_t map_size, FrequencyMap antenna_map);
Point *find_possible_antinodes(Point *antennas, size_t antennas_size, size_t *possible_antinodes_size);

int main(int argc, char const *argv[])
{
    // The map of spaces (as '.'), antennas (as alphanumeric), and antinodes (as '#')
    string_Vec map;
    // The map from frequency to antenna locations
    FrequencyMap antenna_map;

    if (parse_input(&map, &antenna_map))
        return 1;

    int antinode_count = count_antinodes(map.arr, map.len, antenna_map);

    // Print the map after the update
    printf("Map after updates:\n");
    print_map(map.arr, map.len);

    delete_string_vec(&map);
    deleteFrequencyMap(&antenna_map);

    printf("\nNumber of distinct antinodes: %d\n", antinode_count);
    return 0;
}

/// @brief Parse input into `map`, noting the locations of all antennas in antenna_map
/// @param map Out parameter: The map for the puzzle
/// @param antenna_map Out parameter: map of frequencies to antenna locations
/// @return 0 if successful. Some nonzero number if unsuccessful (i.e. IO error)
int parse_input(string_Vec *map, FrequencyMap *antenna_map)
{
    // Open input.txt or panic
    FILE *f = fopen("input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *map = new_string_Vec();
    *antenna_map = newFrequencyMap();

    // Loop over characters in the file
    int ch;
    char_Vec row = new_char_Vec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch == '.')
        {
            append_char_Vec(&row, (char)ch);
        }
        else if (
            ('0' <= ch && ch <= '9') ||
            ('A' <= ch && ch <= 'Z') ||
            ('a' <= ch && ch <= 'z'))
        {
            // Alphanumeric: antenna
            if (add_antenna(antenna_map, (char)ch, (Point){map->len, row.len}))
            {
                // Should be unreachable because we already checked if ch is alphanumeric
                fprintf(stderr, "Unexpected invalid antenna '%c'\n", (char)ch);
                exit(1);
            }
            append_char_Vec(&row, (char)ch);
        }
        else if (ch == '\n')
        {
            // Add terminator for the string and put it on the string vector
            append_char_Vec(&row, '\0');
            append_string_Vec(map, row.arr);
            // Generate a new char vector for the next iteration
            row = new_char_Vec();
        }
        else
        {
            free(row.arr);
            delete_string_vec(map);
            fprintf(stderr, "Unexpected character: '%c'\n", (char)ch);
            return 1;
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

/// @brief Prints `map` to stdout
/// @param map The map
/// @param row_count The number of rows in `map`
void print_map(char **map, size_t row_count)
{
    for (size_t i = 0; i < row_count; i++)
        printf("%s\n", map[i]);
}

// Construct a new empty frequency map
FrequencyMap newFrequencyMap()
{
    FrequencyMap m;
    for (int i = 0; i < sizeof(m.table) / sizeof(m.table[0]); i++)
    {
        m.table[i] = new_Point_Vec();
    }
    return m;
}

// Free all the arrays of a frequency map
void deleteFrequencyMap(FrequencyMap *map)
{
    for (int i = 0; i < sizeof(map->table) / sizeof(map->table[0]); i++)
    {
        free(map->table[i].arr);
        map->table[i].arr = NULL;
        map->table[i].cap = 0UL;
        map->table[i].len = 0UL;
    }
}

// Convert a frequency identifier into an index for the frequency map
// Returns -1 if the frequency is invalid
int frequency_map_index(char frequency)
{
    int index;
    if ('0' <= frequency && frequency <= '9')
        index = frequency - '0';
    else if ('A' <= frequency && frequency <= 'Z')
        // Index by uppercase letter, but offset by all the numbers
        index = frequency + 10 - 'A';
    else if ('a' <= frequency && frequency <= 'z')
        // Index by lowercase letter, but offset by all the numbers and uppercase letters
        index = frequency + 10 + 26 - 'a';
    else
        index = -1;
    return index;
}

// Add an antenna at `antenna` to a frequency map
// Returns 0 if successful, 1 if the frequency is invalid
int add_antenna(FrequencyMap *map, char frequency, Point antenna)
{
    int index = frequency_map_index(frequency);
    if (index == -1)
        return 1;

    append_Point_Vec(&(map->table[index]), antenna);
    return 0;
}

// Binomial coefficient nC2, but doubled
// Used to determine how many antinodes can be made by pairing up n antennas
// Doubled because there are two possible antinodes for every pair of nodes and nC2 would only count pairs
int choose_2_doubled(int n)
{
    return n * (n - 1); // ignoring: / 2 * 2
}

/// @brief Count the number of distinct antinodes created by antennas on the map
/// @param map The map from the puzzle input
/// @param map_size The number of rows in `map`
/// @param antenna_map The map from frequencies to antenna locations
/// @return The number of distinct antinodes
int count_antinodes(char **map, size_t map_size, FrequencyMap antenna_map)
{
    int antinode_count = 0;
    // Loop over frequencies
    for (size_t i = 0; i < sizeof(antenna_map.table) / sizeof(antenna_map.table[0]); i++)
    {
        // Don't look for antinodes if there can't be any
        if (antenna_map.table[i].len < 2)
            continue;

        size_t possible_antinodes_size;
        Point *possible_antinodes = find_possible_antinodes(antenna_map.table[i].arr, antenna_map.table[i].len, &possible_antinodes_size);

        // Filter the antinodes and add them to the map
        for (size_t j = 0; j < possible_antinodes_size; j++)
        {
            Point antinode = possible_antinodes[j];
            if (antinode.row >= 0 && antinode.col >= 0 &&
                antinode.row < map_size && antinode.col < strlen(map[antinode.row]) &&
                map[antinode.row][antinode.col] != '#')
            {
                antinode_count++;
                map[antinode.row][antinode.col] = '#';
            }
        }

        free(possible_antinodes);
    }
    return antinode_count;
}

/// @brief Find all possible antinodes from the given array of antennas
///
/// Some of the returned antinodes may be either impossible, or on an existing antinode. It is up to the caller to filter
/// @param antennas The locations of each antenna of a giver frequency
/// @param antennas_size The number of elements in `antennas`
/// @param possible_antinodes_size Out: the size of the returned array
/// @return An array of Points of size `possible_antinodes_size` representing all possible antinodes
Point *find_possible_antinodes(Point *antennas, size_t antennas_size, size_t *possible_antinodes_size)
{
    // Find the number of antinodes this will output before allocating the array
    // It will always be (antennas count) choose 2
    int antinodes_size = choose_2_doubled((int)antennas_size);
    if (antinodes_size <= 0)
    {
        // Probably redundant guard clause for when there are no possible antinodes
        *possible_antinodes_size = 0UL;
        return NULL;
    }

    *possible_antinodes_size = (size_t)antinodes_size;
    Point *antinodes = malloc(sizeof(Point) * antinodes_size);
    int cursor = 0;
    // Loop over "first" antenna (order is irrelevant)
    for (size_t i = 0; i < antennas_size - 1; i++)
    {
        Point antenna1 = antennas[i];
        // Loop over "second" antenna
        for (size_t j = i + 1; j < antennas_size; j++)
        {
            Point antenna2 = antennas[j];

            /* Find both antinodes
               For example:
               - (3,4),(5,5) -> (1,3),(7,6)
               - (4,8),(5,5) -> (6,2),(3,11)
            */

            // Find differences in each coordinate
            // Combined, they will be the difference vector
            short row_diff = antenna2.row - antenna1.row;
            short col_diff = antenna2.col - antenna1.col;

            // Panic if we miscalculated the number of antinodes
            if (cursor + 2 > antinodes_size)
            {
                fprintf(stderr, "Array overflow adding an antinode");
                exit(1);
            }

            // Since the difference if antenna2 - antenna1,
            // we can find the antinode on the other side of antenna1 by subtracting the difference again.
            antinodes[cursor++] = (Point){antenna1.row - row_diff, antenna1.col - col_diff};
            // Likewise, the antinodes on the antenna2 side can be found by *adding* the difference vector
            antinodes[cursor++] = (Point){antenna2.row + row_diff, antenna2.col + col_diff};
        }
    }

    if (cursor != antinodes_size)
    {
        fprintf(stderr, "Antinode array not filled. Expected size: %d | Actual size: %d\n", antinodes_size, cursor);
        exit(1);
    }

    return antinodes;
}
