#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbit.h>

#include "../vector_template.h"

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
int parse_input(char *input_file, PointVec *points);
void print_grid(char **grid, int map_size);

// Problem steps
long long get_min_steps(char **grid, int grid_size);
Point get_first_blocker(Point *obstacles, size_t obstacles_size, int grid_size);
int check_is_found(char *found_map, int map_size, int map_row_size, int row, int col);
void mark_found(char *found_map, int map_size, int map_row_size, int row, int col, Direction direction);
void mark_min_path(char **grid, char *found_map, int map_size, int map_row_size, int row, int col);

// Priority queue
void resize_pq(MazeMoveHeap *heap, int new_cap);
void push_pq(MazeMoveHeap *heap, MazeMove val);
MazeMove pop_pq(MazeMoveHeap *heap);
void sift_down_pq(MazeMoveHeap *heap, int parent_index);
// MazeMove peek_pq(MazeMoveHeap heap);

int main(int argc, char *argv[])
{
    // Input file name
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    // The number of rows/columns in the square grid
    int grid_size = (argc >= 3) ? atoi(argv[2]) : 7;
    // The number of points to read
    // int point_count = (argc >= 4) ? atoi(argv[3]) : 12;

    PointVec points;

    if (parse_input(input_file, &points))
        return 1;

    // printf("Maximum number of steps: %lld\n", get_min_steps(points, point_count, grid_size));
    Point first_blocker = get_first_blocker(points.arr, points.len, grid_size);
    printf("First blocker: %hd,%hd\n", first_blocker.col, first_blocker.row);

    free(points.arr);
    return 0;
}

/// @brief Parse input.txt
/// @param input_file The file to read
/// @param points Out: The point array to read into
/// @return 0 if success. 1 if failure.
int parse_input(char *input_file, PointVec *points)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        fclose(f);
        return 1;
    }

    // Store the original value of *point_count as max_points so we can have a limit, then zero *point_count so we can actually count
    *points = newPointVec();

    // Read while below maximum point count
    Point p;
    while (fscanf(f, "%hd,%hd\n", &p.col, &p.row) == 2)
        appendPoint(points, p);

    fclose(f);
    return 0;
}

// Print every line in `grid`
void print_grid(char **grid, int map_size)
{
    for (int i = 0; i < map_size; i++)
        puts(grid[i]);
}

/// @brief Count the maximum number of steps taken in a `grid_size` x `grid_size` map while avoiding each obstacle
/// @param grid The map of free spaces and obstacles
/// @param grid_size The number of rows/columns in the grid
/// @return The minimum number of steps needed to go from (0,0) to (`grid_size-1`,`grid_size-1`)
long long get_min_steps(char **grid, int grid_size)
{
    long long min_steps = -1;
    MazeMoveHeap heap = {.arr = NULL, .len = 0, .cap = 0};
    char *found_map = calloc(grid_size * grid_size, sizeof(found_map[0]));
    Point target = {grid_size - 1, grid_size - 1};

    // print_grid(grid, grid_size);

    MazeMove this_move = {.p = {0, 0}, .steps = 0};
    while (this_move.steps >= 0)
    {
        // Skip if this point was already found
        if (check_is_found(found_map, grid_size, grid_size, this_move.p.row, this_move.p.col))
            goto NEXT_MOVE;

        // Mark this point as found
        mark_found(found_map, grid_size, grid_size, this_move.p.row, this_move.p.col, this_move.dir);
        // grid[this_move.p.row][this_move.p.col] = 'O';

        // Check if we found the end
        if (this_move.p.row == target.row && this_move.p.col == target.col)
        {
            min_steps = this_move.steps;
            break;
        }

        // Right
        if (this_move.p.col + 1 < grid_size && grid[this_move.p.row][this_move.p.col + 1] == '.' && !check_is_found(found_map, grid_size, grid_size, this_move.p.row, this_move.p.col + 1))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row, .col = this_move.p.col + 1}, .steps = this_move.steps + 1, .dir = EAST});
        // Up
        if (this_move.p.row > 0 && grid[this_move.p.row - 1][this_move.p.col] == '.' && !check_is_found(found_map, grid_size, grid_size, this_move.p.row - 1, this_move.p.col))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row - 1, .col = this_move.p.col}, .steps = this_move.steps + 1, .dir = NORTH});
        // Left
        if (this_move.p.col > 0 && grid[this_move.p.row][this_move.p.col - 1] == '.' && !check_is_found(found_map, grid_size, grid_size, this_move.p.row, this_move.p.col - 1))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row, .col = this_move.p.col - 1}, .steps = this_move.steps + 1, .dir = WEST});
        // Down
        if (this_move.p.row + 1 < grid_size && grid[this_move.p.row + 1][this_move.p.col] == '.' && !check_is_found(found_map, grid_size, grid_size, this_move.p.row + 1, this_move.p.col))
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row + 1, .col = this_move.p.col}, .steps = this_move.steps + 1, .dir = SOUTH});

    // Next point in the heap
    NEXT_MOVE:
        this_move = pop_pq(&heap);
    }

    // printf("\n");
    // print_grid(grid, grid_size);
    // printf("\n");

    if (min_steps > 0)
        mark_min_path(grid, found_map, grid_size, grid_size, target.row, target.col);

    free(heap.arr);
    free(found_map);
    return min_steps;
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

// // Peek at the top of the heap
// MazeMove peek_pq(MazeMoveHeap heap)
// {
//     return heap.arr[0];
// }

/// @brief Get the first Point that will completely block the grid
/// @param obstacles The array of obstacles
/// @param obstacles_size The number of elements in `obstacles`
/// @param grid_size The number of rows/columns in the grid
/// @return The first blocking point
Point get_first_blocker(Point *obstacles, size_t obstacles_size, int grid_size)
{
    // Allocate grid
    char **grid = malloc(sizeof(grid[0]) * grid_size);
    char **grid_temp = malloc(sizeof(grid_temp[0]) * grid_size);
    Point first_blocker;
    for (int i = 0; i < grid_size; i++)
    {
        // Allocate row for both grid and temporary grid
        grid[i] = malloc(sizeof(grid[0][0]) * (grid_size + 1));
        grid_temp[i] = malloc(sizeof(grid_temp[0][0]) * (grid_size + 1));
        // Set the entire grid to '.'
        memset(grid[i], '.', grid_size);
        // Set the last character to '\0' to end the string
        grid[i][grid_size] = '\0';
    }

    // Put the first 1024 obstacles because we know they are safe
    for (int i = 0; i < 1024 && i < obstacles_size; i++)
    {
        Point p = obstacles[i];
        // Ensure the point is valid
        if (p.row < 0 || p.col < 0 || p.row >= grid_size || p.col >= grid_size)
        {
            fprintf(stderr, "Unexpected point (%hd,%hd). Points axes be in the range [0,%d)\n", p.row, p.col, grid_size);
            goto CLEANUP;
        }

        grid[p.row][p.col] = '#';
    }

    // print_grid(grid, grid_size);
    // printf("\n");

    // Clone grid for each run
    for (int i = 0; i < grid_size; i++)
        memcpy(grid_temp[i], grid[i], sizeof(grid[0][0]) * (grid_size + 1));

    long long min_steps = get_min_steps(grid_temp, grid_size);
    print_grid(grid_temp, grid_size);
    printf("Min steps: %lld\n\n", min_steps);

    for (int i = 1024; i < obstacles_size; i++)
    {
        Point p = obstacles[i];
        grid[p.row][p.col] = '#';
        // Go to the next obstacle if this doesn't even change the last solution
        if (grid_temp[p.row][p.col] != 'O')
            continue;

        // Clone grid for each run
        for (int i = 0; i < grid_size; i++)
            memcpy(grid_temp[i], grid[i], sizeof(grid[0][0]) * grid_size);
        
        min_steps = get_min_steps(grid_temp, grid_size);
        if (min_steps == -1)
        {
            // Once we find a blocker, return it
            first_blocker = p;
            printf("Blocker index: %d\n", i);
            break;
        }
    }

CLEANUP:
    for (int i = 0; i < grid_size; i++)
    {
        free(grid[i]);
        free(grid_temp[i]);
    }
    free(grid);
    free(grid_temp);
    return first_blocker;
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
void mark_min_path(char **grid, char *found_map, int map_size, int map_row_size, int row, int col)
{
    // Mark
    grid[row][col] = 'O';
    // Stop at the origin
    if (!row && !col)
        return;

    // Find parent
    char parents_bits = IDX_2D(found_map, row, col);
    if (stdc_count_ones_uc(parents_bits) != 1)
    {
        fprintf(stderr, "Multiple/too few parents of (%d,%d): %hhx\n", row, col, parents_bits);
        exit(1);
    }
    Direction dir = stdc_trailing_zeros_uc(parents_bits);

    // We are checking behind, so this will be reversed (i.e. facing east means check paths to the west)
    int next_row = row;
    int next_col = col;
    switch (dir)
    {
    case EAST:
        next_col--;
        break;
    case NORTH:
        next_row++;
        break;
    case WEST:
        next_col++;
        break;
    case SOUTH:
        next_row--;
        break;
    default:
        // Panic
        fprintf(stderr, "Unknown direction %d\n", dir);
        return;
    }

    mark_min_path(grid, found_map, map_size, map_row_size, next_row, next_col);
}