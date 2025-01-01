#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../c-data-structures/vector/vector_template.h"

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

// Define guard so we can make methods to move the guard

typedef enum Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT,
} Direction;
typedef struct Guard
{
    short row;
    short col;
    Direction dir;
} Guard;

int parse_input(string_Vec *map, Guard *guard);
int move_guard(Guard *guard, char **map, size_t row_count);
// Rotate direction right and return the rotated direction
Direction rotate_right(Direction dir) { return (dir + 1) % (LEFT + 1); }
int unique_visited_tiles(char **map, size_t row_count, Guard *guard);
void print_map(char **map, size_t row_count);
char peek_guard(Guard guard, char **map, size_t row_count);

int main(int argc, char const *argv[])
{
    string_Vec map;
    Guard guard;

    // Parse input
    if (parse_input(&map, &guard))
        return 1;

    // Count tiles and update the map
    int unique_tiles = unique_visited_tiles(map.arr, map.len, &guard);

    // Print the map after the update
    printf("Map after updates:\n");
    print_map(map.arr, map.len);

    // Print the answer
    printf("\nUnique visited tiles: %d\n", unique_tiles);

    delete_string_vec(&map);
    return 0;
}

/// @brief Parse input.txt, returning a map and the guard
/// @param map Out parameter: The map for the puzzle
/// @param guard Out parameter: location and direction of the guard on the map
/// @return 0 if successful. Some nonzero number if unsuccessful (i.e. IO error)
int parse_input(string_Vec *map, Guard *guard)
{
    // Open input.txt or panic
    FILE *f = fopen("input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *map = new_string_Vec();
    // Loop over characters in the file
    int ch;
    char_Vec row = new_char_Vec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch == '.' || ch == '#')
        {
            append_char_Vec(&row, (char)ch);
        }
        else if (ch == '^' || ch == '>' || ch == 'v' || ch == '<')
        {
            switch (ch)
            {
            case '^':
                guard->dir = UP;
                break;
            case '>':
                guard->dir = RIGHT;
                break;
            case 'v':
                guard->dir = DOWN;
                break;
            default: // '<'
                guard->dir = LEFT;
            }
            guard->row = map->len;
            guard->col = row.len;
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

/// @brief Count unique tiles visited by guard, including the one it's on. Will also turn those tiles into 'X' on the board
/// @param map The map
/// @param row_count The number of rows in `map`
/// @param guard The guard location and direction
/// @return The number of unique visited tiles
int unique_visited_tiles(char **map, size_t row_count, Guard *guard)
{
    // Start at 1 because the guard is already on one tile
    int unique_tiles = 1;
    map[guard->row][guard->col] = 'X';
    // Get the maximum number of tiles as a safeguard in case we get stuck
    int max_tiles = row_count * strlen(map[0]);
    for (int i = 0; i < max_tiles; i++)
    {
        int move = move_guard(guard, map, row_count);
        if (move == -1)
            break;
        else if (move == 1)
            unique_tiles++;
    }
    return unique_tiles;
}

/// @brief Move a guard one step
/// @param guard The guard's location and direction
/// @param map The map as a 2D character array
/// @param row_count The number of rows in map (columns is irrelevant because each row is a null-terminated string)
/// @return 0 if the new square has already been visited, 1 if the new square has not been visited, -1 if the new square is off the map or the guard is stuck
int move_guard(Guard *guard, char **map, size_t row_count)
{
    int new_tile;
    switch (peek_guard(*guard, map, row_count))
    {
    case '\0':
        // Off the map
        return -1;
    case '#':
        // Obstacle rotate and try again
        guard->dir = rotate_right(guard->dir);
        // Return instead of moving again
        return move_guard(guard, map, row_count);
    case 'X':
        // Not a new tile
        new_tile = 0;
        break;
    default:
        // New tile
        new_tile = 1;
    }

    // Move the guard
    switch (guard->dir)
    {
    case UP:
        guard->row--;
        break;
    case RIGHT:
        guard->col++;
        break;
    case DOWN:
        guard->row++;
        break;
    default: // LEFT
        guard->col--;
    }
    map[guard->row][guard->col] = 'X';

    return new_tile;
}

/// @brief Determine what tile the guard will move to next to determine if the guard should spin or move forward
/// @param guard The guard's location and direction
/// @param map The map as a 2D character array
/// @param row_count The number of rows in map (columns is irrelevant because each row is a null-terminated string)
/// @return The tile the guard is looking at, or '\0' if the guard is looking off the board
char peek_guard(Guard guard, char **map, size_t row_count)
{
    char next_tile;
    switch (guard.dir)
    {
    case UP:
        // Determine if this would put the guard off the map
        if (guard.row < 1)
            next_tile = '\0';
        else
            next_tile = map[guard.row - 1][guard.col];
        break;
    case RIGHT:
        // This is the only one that doesn't need explicit validation because the string is null-terminated anyway
        next_tile = map[guard.row][guard.col + 1];
        break;
    case DOWN:
        if (guard.row + 1 >= row_count)
            next_tile = '\0';
        else
            next_tile = map[guard.row + 1][guard.col];
        break;
    default: // LEFT
        if (guard.col < 1)
            next_tile = '\0';
        else
            next_tile = map[guard.row][guard.col - 1];
    }
    return next_tile;
}
