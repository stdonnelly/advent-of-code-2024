#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../c-data-structures/vector/vector_template.h"

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

typedef struct Point
{
    short row;
    short col;
} Point;

int parse_input(char *input_file, string_Vec *map, char_Vec *moves, Point *robot);
void print_map(char **map, size_t map_size, Point robot);
long long get_gps_sum(char **map, size_t map_size, char *moves, Point *robot);
int get_gps(int row, int col);
int try_move(char **map, size_t map_size, int row, int col, char direction);

int main(int argc, char *argv[])
{
    string_Vec map;
    char_Vec moves;
    Point robot;
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    if (parse_input(input_file, &map, &moves, &robot))
        return 1;

    long long gps_sum = get_gps_sum(map.arr, map.len, moves.arr, &robot);

    print_map(map.arr, map.len, robot);
    for (size_t i = 0; moves.arr[i]; i++)
    {
        if (!(i % 70))
            putchar('\n');
        putchar(moves.arr[i]);
    }
    printf("\n\nRobot: (%hd,%hd)\n\n", robot.row, robot.col);
    printf("Sum of GPS's: %lld\n", gps_sum);

    delete_string_vec(&map);
    free(moves.arr);
    return 0;
}

// Print the map to stdout
void print_map(char **map, size_t map_size, Point robot)
{
    map[robot.row][robot.col] = '@';
    for (size_t i = 0; i < map_size; i++)
        puts(map[i]);
    map[robot.row][robot.col] = '.';
}

/// @brief Parse the input file into the map, moves, and the robot location
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param map Out: The map that the robot is in
/// @param moves Out: All of the moves that will be taken by the robot
/// @param robot Out: The location of the robot
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, string_Vec *map, char_Vec *moves, Point *robot)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }
    *map = new_string_Vec();
    *moves = new_char_Vec();

    // Used to detect double newline, which indicates the start of the next section
    int last_char = 0;
    int ch;
    char_Vec row = new_char_Vec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch != '\n')
        {
            // If robot, note its position and set the space to empty
            if (ch == '@')
            {
                robot->row = map->len;
                robot->col = row.len;
                append_char_Vec(&row, '.');
            }
            else
                append_char_Vec(&row, ch);
        }
        // Put the new line only if the double newline has not been reached
        else if (last_char != '\n')
        {
            append_char_Vec(&row, '\0');
            append_string_Vec(map, row.arr);
            // Generate a new char vector for the next iteration
            // Slight optimization: start with a capacity of this array's length
            row.arr = malloc(sizeof(row.arr[0]) * row.len);
            row.cap = row.len;
            row.len = 0UL;
        }
        else
            break;
        last_char = ch;
    }

    free(row.arr);

    // Put the rest of the file in the moves vector
    while ((ch = getc(f)) != EOF)
        if (ch != '\n')
            append_char_Vec(moves, ch);
    append_char_Vec(moves, '\0');

    return 0;
}

// The GPS coordinate of a box is equal to 100 times its distance from the top edge of the map
// plus its distance from the left edge of the map.
int get_gps(int row, int col)
{
    return (100 * row) + col;
}

/// @brief Get the sum of the gps's of the boxes after moving the robot
/// @param map The map
/// @param map_size The number of rows in `map`
/// @param moves Null-terminated list of moves
/// @param robot The location of the robot. This is passed by reference so the caller can know it's destination.
/// @return The sum of the GPS's of the boxes destinations
long long get_gps_sum(char **map, size_t map_size, char *moves, Point *robot)
{
    long long gps_sum = 0LL;
    // Do moves
    // Loop over the array of moves
    while (*moves)
    {
        // Figure out where the robot "wants" to go and ensure that isn't off the map
        char move = *moves;
        Point destination = *robot;
        switch (move)
        {
        case '>':
            // Right
            // Ensure this move is not off the map
            if (!map[destination.row][++destination.col])
                goto NEXT;
            break;
        case '^':
            // Up
            if (--destination.row < 0)
                goto NEXT;
            break;
        case '<':
            // Left
            if (--destination.col < 0)
                goto NEXT;
            break;
        case 'v':
            // Down
            if (++destination.row >= map_size)
                goto NEXT;
            break;
        default:
            // Something is wrong, so end the function early
            fprintf(stderr, "Unexpected move: '%c'\n", move);
            return -1;
        }

        // Determine what's there and act accordingly
        switch (map[destination.row][destination.col])
        {
        case 'O':
            // Box
            if (try_move(map, map_size, destination.row, destination.col, move))
                // Put an empty space where the box was because we don't keep track of robot on the map
                map[destination.row][destination.col] = '.';
            else
                break;
            // If the move was possible, fallthrough
        case '.':
            // Free space, set the robot to the destination
            *robot = destination;
            break;
        case '#':
            // Wall: do nothing
            break;
        default:
            fprintf(stderr, "Unexpected object: '%c'\n", move);
            return -1;
        }

    NEXT:
        // Go to the next move
        moves++;
    }

    // Find gps sum by finding every box
    // Loop over rows
    for (int i = 0; i < map_size; i++)
    {
        // Loop over columns
        for (int j = 0; map[i][j]; j++)
        {
            if (map[i][j] == 'O')
                gps_sum += get_gps(i, j);
        }
    }
    return gps_sum;
}

/// @brief Try to move the box (represented by O) in the given direction. Fails if this action would push a box into a wall.
/// @param map The map
/// @param map_size The number of rows in the map
/// @param row The row of the box to move
/// @param col The column of the box to move
/// @param direction The direction, represented by one of [>^<v], in which to move the box
/// @return 1 if box is successfully moved, 0 if a wall was hit
int try_move(char **map, size_t map_size, int row, int col, char direction)
{
    Point destination = {.row = row, .col = col};
    switch (direction)
    {
    case '>':
        // Right
        // Ensure this move is not off the map
        if (!map[destination.row][++destination.col])
            return 0;
        break;
    case '^':
        // Up
        if (--destination.row < 0)
            return 0;
        break;
    case '<':
        // Left
        if (--destination.col < 0)
            return 0;
        break;
    case 'v':
        // Down
        if (++destination.row >= map_size)
            return 0;
        break;
    default:
        // Something is wrong, so end the function early
        fprintf(stderr, "Unexpected move: '%c'\n", direction);
        return -1;
    }

    // Determine what's there and act accordingly
    switch (map[destination.row][destination.col])
    {
    case 'O':
        // Box
        return try_move(map, map_size, destination.row, destination.col, direction);
        break;
    case '.':
        // Empty space
        map[destination.row][destination.col] = 'O';
        return 1;
        break;
    case '#':
        // Wall: do nothing
        return 0;
        break;
    default:
        fprintf(stderr, "Unexpected object: '%c'\n", map[destination.row][destination.col]);
        return -1;
    }
}
