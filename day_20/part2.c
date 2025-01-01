#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbit.h>

#include "../c-data-structures/vector/vector_template.h"

// The minimum number of units to save in a cheat
#define MIN_SAVE 100
#define MAX_CHEAT_LENGTH 20

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

// Helper to index a 1D array as 2D
#define IDX_2D(arr, row, col) arr[((row) * map_row_size) + (col)]

typedef struct Point
{
    short row;
    short col;
} Point;

DEF_VEC(Point)

typedef enum Direction
{
    EAST = 0,
    NORTH = 1,
    WEST = 2,
    SOUTH = 3,
} Direction;

typedef struct MazeMove
{
    Point p;
    int steps;
    Direction dir;
} MazeMove;

typedef struct MazeMoveHeap
{
    MazeMove *arr;
    int len;
    int cap;
} MazeMoveHeap;

// IO
int parse_input(char *input_file, string_Vec *map, Point *start, Point *end);
void print_map(char **map, size_t map_size, Point start, Point end);

// Priority queue
void resize_pq(MazeMoveHeap *heap, int new_cap);
void push_pq(MazeMoveHeap *heap, MazeMove val);
MazeMove pop_pq(MazeMoveHeap *heap);
void sift_down_pq(MazeMoveHeap *heap, int parent_index);

// Problem steps
int check_is_found(char *found_map, int map_size, int map_row_size, int row, int col);
void mark_found(char *found_map, int map_size, int map_row_size, int row, int col, Direction direction);
void mark_min_path(char **grid, char *found_map, int map_size, int map_row_size, Point maze_start, Point maze_end, Point *path, int path_index);
long long count_helpful_cheats(char **map, size_t map_size, Point start, Point end);
Point *find_best_path(char **map, size_t map_size, Point start, Point end, int *return_size);
long long count_shortcuts_from_point(int *distance_map, size_t map_size, size_t map_row_size, Point start, int min_saved_from_point);
Point get_point_in_direction(Point original, Direction dir);
int contains_point(Point *arr, int arr_size, Point p);
Point *find_points_at_taxicab_distance(Point center, int distance, size_t *return_size);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    string_Vec map;
    Point start;
    Point end;

    if (parse_input(input_file, &map, &start, &end))
        return 1;

    print_map(map.arr, map.len, start, end);
    printf("\n");

    long long helpful_cheats = count_helpful_cheats(map.arr, map.len, start, end);

    printf("Total number of cheats that save at least %d picoseconds over non-cheating paths: %lld\n", MIN_SAVE, helpful_cheats);

    delete_string_vec(&map);
    return 0;
}

/// @brief Parse the input file into the map, start, and end
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param map Out: The maze map
/// @param start Out: The starting point
/// @param end Out: The ending point
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, string_Vec *map, Point *start, Point *end)
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
        switch (ch)
        {
        case '.':
        case '#':
            append_char_Vec(&row, (char)ch);
            break;
        case 'S':
            start->row = map->len;
            start->col = row.len;
            append_char_Vec(&row, '.');
            break;
        case 'E':
            end->row = map->len;
            end->col = row.len;
            append_char_Vec(&row, '.');
            break;
        case '\n':
            // Add terminator for the string and put it on the string vector
            append_char_Vec(&row, '\0');
            append_string_Vec(map, row.arr);
            // Generate a new char vector for the next iteration
            // Slight optimization: start with a capacity of this array's length
            row.arr = malloc(sizeof(row.arr[0]) * row.len);
            row.cap = row.len;
            row.len = 0UL;
            break;
        default:
            // Error message, cleanup, and exit
            fprintf(stderr, "Unexpected character '%c'\n", (char)ch);
            delete_string_vec(map);
            free(row.arr);
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

/// @brief Print the map to the terminal
/// @param map The map of the maze
/// @param map_size The number of rows in `map`
/// @param start The start point
/// @param end The end point
void print_map(char **map, size_t map_size, Point start, Point end)
{
    // Set start and end points in the map itself
    map[start.row][start.col] = 'S';
    map[end.row][end.col] = 'E';

    // Print map
    for (size_t i = 0; i < map_size; i++)
    {
        for (size_t j = 0; map[i][j]; j++)
        {
            char ch = map[i][j];
            if (start.row == i && start.col == j)
                ch = 'S';
            else if (end.row == i && end.col == j)
                ch = 'E';

            int color;
            switch (ch)
            {
            case '.':
                // Empty space: gray
                color = 90;
                break;
            case 'O':
                // Shortest path: blue
                color = 34;
                break;
            case 'S':
                // Start: green
                color = 32;
                break;
            case 'E':
                // End: red
                color = 31;
                break;
            default:
                // Anything else: white
                color = 37;
            }
            printf("\e[%dm%c", color, ch);
        }
        printf("\e[37m\n");
    }

    // Cleanup by resetting those points
    map[start.row][start.col] = '.';
    map[end.row][end.col] = '.';
}

/// @brief Resize a heap
/// @param heap The heap
/// @param new_cap The new capacity
void resize_pq(MazeMoveHeap *heap, int new_cap)
{
    heap->arr = realloc(heap->arr, sizeof(heap->arr[0]) * new_cap);
    heap->cap = new_cap;
}

/// @brief Push a value onto the heap
/// @param heap The heap
/// @param val The new value
void push_pq(MazeMoveHeap *heap, MazeMove val)
{
    /* From Wikipedia https://en.wikipedia.org/wiki/Binary_heap Heap operations
       1. Add the element to the bottom level of the heap at the leftmost open space.
       2. Compare the added element with its parent; if they are in the correct order, stop.
       3. If not, swap the element with its parent and return to the previous step.
    */
    int this_element = heap->len;
    int parent = (this_element - 1) / 2;

    // Double capacity if necessary
    if (heap->len + 1 > heap->cap)
        resize_pq(heap, heap->cap ? heap->cap * 2 : 1);

    // Add the element to the bottom level of the heap at the leftmost open space.
    heap->arr[heap->len++] = val;

    // Compare the added element with its parent; if they are in the correct order, stop.
    // `&& thisElement` to stop when thisElement is root
    while ((heap->arr[this_element].steps < heap->arr[parent].steps) && this_element)
    {
        // If not, swap the element with its parent and return to the previous step.
        MazeMove temp = heap->arr[this_element];
        heap->arr[this_element] = heap->arr[parent];
        heap->arr[parent] = temp;

        // For previous step
        this_element = parent;
        parent = (this_element - 1) / 2;
    }
}

/// @brief Pop the top value off of the heap
/// @param heap The heap
/// @return The old top
MazeMove pop_pq(MazeMoveHeap *heap)
{
    // Do nothing if the heap is empty
    // Indicate the heap is empty with negative steps, though
    if (heap->len <= 0)
        return (MazeMove){.p = {0, 0}, .steps = -1};
    MazeMove top = heap->arr[0];
    MazeMove new_root = heap->arr[--(heap->len)];
    heap->arr[0] = new_root;
    sift_down_pq(heap, 0);
    return top;
}

/// @brief Sift down operation on heap
/// @param heap The heap
/// @param index The index of the parent element to sift down
void sift_down_pq(MazeMoveHeap *heap, int parent_index)
{
    int left = (parent_index * 2) + 1;
    int right = left + 1;
    int parent_val = heap->arr[parent_index].steps;
    int min_val = parent_val;
    int min_index = parent_index;

    // Check if left is min
    if (left < heap->len)
    {
        int left_val = heap->arr[left].steps;
        if (left_val < min_val)
        {
            min_index = left;
            min_val = left_val;
        }
    }

    // Check if right is min
    if (right < heap->len)
    {
        int right_val = heap->arr[right].steps;
        if (right_val < min_val)
        {
            min_index = right;
            min_val = right_val;
        }
    }

    // Check if parent is still min
    if (min_index != parent_index)
    {
        // Swap
        MazeMove temp = heap->arr[parent_index];
        heap->arr[parent_index] = heap->arr[min_index];
        heap->arr[min_index] = temp;
        sift_down_pq(heap, min_index);
    }
}

/// @brief Check if a position has been marked as found
/// @param found_map The map of found positions
/// @param map_size The number of rows in the map
/// @param map_row_size The number of columns in the map
/// @param row The row to check
/// @param col The column to check
/// @return 0 if not marked, (1 << direction) (i.e. some nonzero value) if found
int check_is_found(char *found_map, int map_size, int map_row_size, int row, int col)
{
    return IDX_2D(found_map, row, col); // & (1 << direction);
}

/// @brief Mark a position on the map as found
/// @param found_map The map of found positions
/// @param map_size The number of rows in the map
/// @param map_row_size The number of columns in the map
/// @param row The row to mark
/// @param col The column to mark
/// @param direction The direction to mark
void mark_found(char *found_map, int map_size, int map_row_size, int row, int col, Direction direction)
{
    // Mark found with a bit, offset by the direction
    if (direction >= 0)
        IDX_2D(found_map, row, col) |= 1 << direction;
}

/// @brief Mark this square, and any parents of this square
/// @param grid The grid to mark
/// @param found_map The map of found positions
/// @param map_size The number of rows in the map
/// @param map_row_size The number of columns in the map
/// @param row The row to mark
/// @param col The column to mark
/// @param path Out: a path from start to end
/// @param path_index The index along the path to insert a point
void mark_min_path(char **grid, char *found_map, int map_size, int map_row_size, Point maze_start, Point maze_end, Point *path, int path_index)
{
    // Mark
    grid[maze_end.row][maze_end.col] = 'O';
    path[path_index] = maze_end;
    // Stop at the origin
    if ((maze_end.row == maze_start.row) && (maze_end.col == maze_start.col))
        return;

    // Find parent
    char parents_bits = IDX_2D(found_map, maze_end.row, maze_end.col);
    if (stdc_count_ones_uc(parents_bits) != 1)
    {
        fprintf(stderr, "Multiple/too few parents of (%d,%d): %hhx\n", maze_end.row, maze_end.col, parents_bits);
        exit(1);
    }
    Direction dir = stdc_trailing_zeros_uc(parents_bits);

    // We are checking behind, so this will be reversed (i.e. facing east means check paths to the west)
    Point next = {.row = maze_end.row, .col = maze_end.col};
    switch (dir)
    {
    case EAST:
        next.col--;
        break;
    case NORTH:
        next.row++;
        break;
    case WEST:
        next.col++;
        break;
    case SOUTH:
        next.row--;
        break;
    default:
        // Panic
        fprintf(stderr, "Unknown direction %d\n", dir);
        return;
    }

    mark_min_path(grid, found_map, map_size, map_row_size, maze_start, next, path, path_index - 1);
}

/// @brief Find the best path from `start` to `end` and mark it on the map
/// @param map The input map
/// @param map_size The number of rows in `map`
/// @param start The start point
/// @param end The end point
/// @param return_size The number of elements returned (the minimum distance to travel to reach the end + 1)
/// @return An array of Points from start to end along the best path
Point *find_best_path(char **map, size_t map_size, Point start, Point end, int *return_size)
{
    int min_steps = -1;
    size_t map_row_size = map_size ? strlen(map[0]) : 0;
    MazeMoveHeap heap = {.arr = NULL, .len = 0, .cap = 0};
    char *found_map = calloc(map_size * map_row_size, sizeof(found_map[0]));

    MazeMove this_move = {.p = start, .steps = 0};
    while (this_move.steps >= 0)
    {
        // Skip if this point was already found
        if (check_is_found(found_map, map_size, map_row_size, this_move.p.row, this_move.p.col))
            goto NEXT_MOVE;

        mark_found(found_map, map_size, map_row_size, this_move.p.row, this_move.p.col, this_move.dir);

        // Check if we found the end
        if (this_move.p.row == end.row && this_move.p.col == end.col)
        {
            min_steps = this_move.steps;
            break;
        }

        // Right
        if (this_move.p.col + 1 < map_row_size && map[this_move.p.row][this_move.p.col + 1] == '.' && !check_is_found(found_map, map_size, map_row_size, this_move.p.row, this_move.p.col + 1))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row, .col = this_move.p.col + 1}, .steps = this_move.steps + 1, .dir = EAST});
        // Up
        if (this_move.p.row > 0 && map[this_move.p.row - 1][this_move.p.col] == '.' && !check_is_found(found_map, map_size, map_row_size, this_move.p.row - 1, this_move.p.col))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row - 1, .col = this_move.p.col}, .steps = this_move.steps + 1, .dir = NORTH});
        // Left
        if (this_move.p.col > 0 && map[this_move.p.row][this_move.p.col - 1] == '.' && !check_is_found(found_map, map_size, map_row_size, this_move.p.row, this_move.p.col - 1))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row, .col = this_move.p.col - 1}, .steps = this_move.steps + 1, .dir = WEST});
        // Down
        if (this_move.p.row + 1 < map_size && map[this_move.p.row + 1][this_move.p.col] == '.' && !check_is_found(found_map, map_size, map_row_size, this_move.p.row + 1, this_move.p.col))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row + 1, .col = this_move.p.col}, .steps = this_move.steps + 1, .dir = SOUTH});

        // Next point in the heap
    NEXT_MOVE:
        this_move = pop_pq(&heap);
    }

    Point *path;
    if (min_steps > 0)
    {
        *return_size = min_steps + 1;
        path = malloc(sizeof(path[0]) * *return_size);
        mark_min_path(map, found_map, map_size, map_row_size, start, end, path, min_steps);
    }
    else
    {
        *return_size = 0;
        path = NULL;
    }

    free(heap.arr);
    free(found_map);
    return path;
}

/// @brief Count the number of "cheats" that save at least `MIN_SAVE` moves
/// @param map The map
/// @param map_size The number of rows in `map`
/// @param start The start point
/// @param end The end point
/// @return The number of helpful cheats
long long count_helpful_cheats(char **map, size_t map_size, Point start, Point end)
{
    long long helpful_cheats = 0;
    int best_path_size;
    Point *best_path = find_best_path(map, map_size, start, end, &best_path_size);
    size_t map_row_size = map_size ? strlen(map[0]) : 0;

    // Map out the distances for each point in the map
    int *distance_map = calloc(map_size * map_row_size, sizeof(distance_map[0]));
    for (int i = 0; i < best_path_size; i++)
        IDX_2D(distance_map, best_path[i].row, best_path[i].col) = i;

    print_map(map, map_size, start, end);
    printf("\n");

    for (int i = 0; i + MIN_SAVE + 2 < best_path_size; i++)
    {
        helpful_cheats += count_shortcuts_from_point(distance_map, map_size, map_row_size, best_path[i], i + MIN_SAVE);
    }

    free(best_path);
    free(distance_map);
    return helpful_cheats;
}

/// @brief Get a the adjacent point in the given direction
/// @param original The original direction
/// @param dir The direction to go in
/// @return The adjacent point in the given direction
Point get_point_in_direction(Point original, Direction dir)
{
    switch (dir)
    {
    case EAST:
        original.col++;
        break;
    case NORTH:
        original.row--;
        break;
    case WEST:
        original.col--;
        break;
    case SOUTH:
        original.row++;
        break;
    default:
        fprintf(stderr, "Unknown direction: %d\n", dir);
        exit(1);
    }
    return original;
}

/// @brief Determine if `arr` contains `p`
/// @param arr The array to check
/// @param arr_size The number of elements in `arr`
/// @param p The point to find
/// @return 1 if found, 0 if not found
int contains_point(Point *arr, int arr_size, Point p)
{
    for (int i = 0; i < arr_size; i++)
        if (arr[i].row == p.row && arr[i].col == p.col)
            return 1;
    return 0;
}

/// @brief Count the number of ways to skip a wall to get to one of the points in the map
/// @param distance_map A map of distances from the start
/// @param map_size The number of rows in `distance_map`
/// @param map_row_size The number of columns in `distance_map`
/// @param start The start point that we are trying to save distance from
/// @param min_saved_from_point The minimum point that should be reached from this point via cheating is `min_saved_from_point` + (distance from this point)
/// @return The number of cheats that can be performed to skip to another point
long long count_shortcuts_from_point(int *distance_map, size_t map_size, size_t map_row_size, Point start, int min_saved_from_point)
{
    long long shortcuts_from_point = 0;

    // Theres no point checking below two units because there's no way out of the wall then
    // We check for distance < possible_ends_size because, when checking, we will cut off the first `distance` points from the start
    for (int distance = 2; distance <= MAX_CHEAT_LENGTH && distance < min_saved_from_point; distance++)
    {
        int min_point_for_this_distance = min_saved_from_point + distance;
        // The more distance we cheat by, the more time saving after the cheat is required for a saving of >=100
        size_t points_to_check_size;
        Point *points_to_check = find_points_at_taxicab_distance(start, distance, &points_to_check_size);

        for (int i = 0; i < points_to_check_size; i++)
        {
            Point p = points_to_check[i];
            // Ignore invalid and walls
            if (p.row >= 0 && p.col >= 0 && p.row < map_size && p.col < map_row_size && IDX_2D(distance_map, p.row, p.col) >= min_point_for_this_distance)
                shortcuts_from_point++;
        }

        free(points_to_check);
    }

    return shortcuts_from_point;
}

/// @brief Find all points that are a certain taxicab distance from `center`
/// @param center The center point to find distance around
/// @param distance The distance from the center to mark
/// @param return_size The number of elements in the returned array
/// @return The number of points that are `distance` away from center
Point *find_points_at_taxicab_distance(Point center, int distance, size_t *return_size)
{
    *return_size = distance * 4;
    Point *points = malloc(sizeof(points[0]) * *return_size);
    size_t point_cursor = 0;
    Point p = {.row = center.row, .col = center.col + distance};

    // Move east to north
    while (p.col > center.col)
    {
        // Ensure we are still in the array
        if (point_cursor >= *return_size)
        {
            fprintf(stderr, "Unexpected number of points\n");
            goto END;
        }

        // Put this point into the array
        points[point_cursor++] = p;
        // Go northwest
        p.row--;
        p.col--;
    }

    // Move north to west
    while (p.row < center.row)
    {
        // Ensure we are still in the array
        if (point_cursor >= *return_size)
        {
            fprintf(stderr, "Unexpected number of points\n");
            goto END;
        }

        // Put this point into the array
        points[point_cursor++] = p;
        // Go southwest
        p.row++;
        p.col--;
    }

    // Move west to south
    while (p.col < center.col)
    {
        // Ensure we are still in the array
        if (point_cursor >= *return_size)
        {
            fprintf(stderr, "Unexpected number of points\n");
            goto END;
        }

        // Put this point into the array
        points[point_cursor++] = p;
        // Go southeast
        p.row++;
        p.col++;
    }

    // Move south to east
    while (p.row > center.row)
    {
        // Ensure we are still in the array
        if (point_cursor >= *return_size)
        {
            fprintf(stderr, "Unexpected number of points\n");
            goto END;
        }

        // Put this point into the array
        points[point_cursor++] = p;
        // Go northwest
        p.row--;
        p.col++;
    }

END:
    return points;
}
