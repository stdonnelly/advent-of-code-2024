#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"

// Define char and char* vectors
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

typedef struct Position
{
    int row;
    int col;
} Position;
DEF_VEC(Position);

int parse_input(stringVec *map, Guard *guard);
int move_guard(Guard *guard, char **map, size_t row_count);
// Rotate direction right and return the rotated direction
Direction rotate_right(Direction dir) { return (dir + 1) % (LEFT + 1); }
PositionVec unique_visited_tiles(char **map, size_t row_count, Guard guard);
void print_map(char **map, size_t row_count);
char peek_guard(Guard guard, char **map, size_t row_count);
int obstructions_that_create_loops(char **map, size_t map_row_count, Position *positions_to_try, size_t positions_to_try_size, Guard guard);
int contains_loop(char **map, size_t map_row_count, Guard guard);
int move_guard_with_direction(Guard *guard, char **map, size_t row_count);

int main(int argc, char const *argv[])
{
    stringVec map;
    Guard guard;

    // Parse input
    if (parse_input(&map, &guard))
        return 1;

    // Count tiles and update the map
    PositionVec unique_tiles = unique_visited_tiles(map.arr, map.len, guard);

    // Print the map after the update
    printf("Map after updates:\n");
    print_map(map.arr, map.len);

    // Print the answer
    printf("\nUnique visited tiles: %d\n", (int)unique_tiles.len + 1);

    // Return every 'X' back to '.'
    for (size_t i = 0; i < map.len; i++)
    {
        char *row = map.arr[i];
        while (*row)
        {
            if (*row == 'X')
                *row = '.';
            row++;
        }
    }
    printf("Number of places for an obstacle that will create a loop: %d\n", obstructions_that_create_loops(map.arr, map.len, unique_tiles.arr, unique_tiles.len, guard));

    // Free everything
    free(unique_tiles.arr);
    delete_string_vec(&map);
    return 0;
}

/// @brief Parse input.txt, returning a map and the guard
/// @param map Out parameter: The map for the puzzle
/// @param guard Out parameter: location and direction of the guard on the map
/// @return 0 if successful. Some nonzero number if unsuccessful (i.e. IO error)
int parse_input(stringVec *map, Guard *guard)
{
    // Open input.txt or panic
    FILE *f = fopen("input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *map = newstringVec();
    // Loop over characters in the file
    int ch;
    charVec row = newcharVec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch == '.' || ch == '#')
        {
            appendchar(&row, (char)ch);
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
            appendchar(&row, (char)ch);
        }
        else if (ch == '\n')
        {
            // Add terminator for the string and put it on the string vector
            appendchar(&row, '\0');
            appendstring(map, row.arr);
            // Generate a new char vector for the next iteration
            row = newcharVec();
        }
        else
        {
            free(row.arr);
            delete_string_vec(map);
            fprintf(stderr, "Unexpected character: '%c'\n", (char)ch);
            return 1;
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
/// @return All unique visited tiles, excluding the guard start
PositionVec unique_visited_tiles(char **map, size_t row_count, Guard guard)
{
    PositionVec unique_tiles = newPositionVec();
    // Mark the starting point as visited to prevent it from being added to the vector
    map[guard.row][guard.col] = 'X';
    // Get the maximum number of tiles as a safeguard in case we get stuck
    int max_tiles = row_count * strlen(map[0]);
    for (int i = 0; i < max_tiles; i++)
    {
        int move = move_guard(&guard, map, row_count);
        if (move == -1)
            break;
        // Check if this a new tile and it's not the start tile
        else if (move == 1)
            appendPosition(&unique_tiles, (Position){(int)(guard.row), (int)(guard.col)});
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

// For debugging: count the number of maps we've tried
// int recursions = 0;

/// @brief Determine how many places we can put obstructions where a loop will be created
/// @param map The map
/// @param map_row_count The number of rows in the map
/// @param positions_to_try An array of obstruction positions to try
/// @param positions_to_try_size The number of elements in `positions_to_try`
/// @param guard The guard's original position and direction
/// @return The number of places we can put an obstruction to create a loop
int obstructions_that_create_loops(char **map, size_t map_row_count, Position *positions_to_try, size_t positions_to_try_size, Guard guard)
{
    // Base case: there are no positions to try
    if (positions_to_try_size <= 0)
        return 0;

    // Copy the map so we can check this one without causing issues for future iterations
    // Get the size of each row (+1 for the null terminator)
    size_t map_row_size = strlen(map[0]) + 1;
    char **map_copy = malloc(sizeof(char *) * map_row_count);
    for (size_t i = 0; i < map_row_count; i++)
    {
        map_copy[i] = malloc(sizeof(char) * map_row_size);
        strncpy(map_copy[i], map[i], map_row_size);
    }

    map_copy[positions_to_try[0].row][positions_to_try[0].col] = '#';

    // Check for a loop
    int first_position_creates_loop = contains_loop(map_copy, map_row_count, guard);

    // For debug, print loops
    // if (first_position_creates_loop)
    // {
    //     for (size_t i = 0; i < map_row_count; i++)
    //     {
    //         char *map_row = map_copy[i];
    //         while (*map_row)
    //         {
    //             char ch = *map_row;
    //             if (!(ch & 0xf0))
    //             {
    //                 // Check if (up or down) and (left or right)
    //                 if ((ch & 0b0101) && (ch & 0b1010))
    //                     *map_row = '+';
    //                 else if (ch & 0b0101)
    //                     // Up or down
    //                     *map_row = '|';
    //                 else
    //                     // Left or right
    //                     *map_row = '-';
    //             }
    //             map_row++;
    //         }
    //     }
    //     printf("Map %d:\n", recursions);
    //     print_map(map_copy, map_row_count);
    //     printf("\n");
    // }
    // recursions++;

    // Free the map copy
    // I could definitely re-copy and reuse this, but I don't want to yet
    for (size_t i = 0; i < map_row_count; i++)
        free(map_copy[i]);
    free(map_copy);
    // Recursion: Try the next positions
    return first_position_creates_loop + obstructions_that_create_loops(map, map_row_count, positions_to_try + 1, positions_to_try_size - 1, guard);
}

/// @brief Check if the given map contains a guard loop
/// @param map The map
/// @param map_row_count The number of rows in the map
/// @param guard The position and direction of the guard
/// @return 0 if the guard will exit the map, 1 if the guard will get stuck in a loop
int contains_loop(char **map, size_t map_row_count, Guard guard)
{
    // Mark the starting point as visited to prevent it from being added to the vector
    map[guard.row][guard.col] = 1 << guard.dir;
    // Get the maximum number of tiles as a safeguard in case we get stuck
    int max_tiles = map_row_count * strlen(map[0]);
    for (int i = 0; i < max_tiles; i++)
    {
        int move = move_guard_with_direction(&guard, map, map_row_count);
        if (move == -1)
            return 0;
        // Check if a loop was detected
        else if (move == 1)
            return 1;
    }
    // If we went too far, assume there was a loop
    return 1;
}

/// @brief Move a guard one step, but leave a bitmask of directions instead of 'X'
/// @param guard The guard's location and direction
/// @param map The map as a 2D character array
/// @param row_count The number of rows in map (columns is irrelevant because each row is a null-terminated string)
/// @return 0 if a loop was not found, 1 if a loop has been found, -1 if the new square is off the map or the guard is stuck
int move_guard_with_direction(Guard *guard, char **map, size_t row_count)
{
    // Check for obstacle or end
    switch (peek_guard(*guard, map, row_count))
    {
    case '\0':
        // Off the map
        return -1;
    case '#':
        // Obstacle rotate and try again
        guard->dir = rotate_right(guard->dir);
        // Return instead of moving again
        return move_guard_with_direction(guard, map, row_count);
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

    // Use the appropriate direction for the tile
    char old_tile = map[guard->row][guard->col];
    // All printable characters will have some bits in the first nibble
    // If this is a printable character that isn't an obstacle, we should just leave the direction here
    if (old_tile & 0xf0)
        map[guard->row][guard->col] = 1 << guard->dir;
    else if (old_tile & (1 << guard->dir))
        // Otherwise, there is already some direction here,
        // so we should determine if the guard has been here before in this direction
        return 1;
    else
        // If it's already a direction, but not this direction, add this direction
        map[guard->row][guard->col] = old_tile | (1 << guard->dir);

    return 0;
}
