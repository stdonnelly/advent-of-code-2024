#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Point
{
    short row;
    short col;
} Point;

typedef struct MazeMove
{
    Point p;
    int steps;
} MazeMove;

typedef struct MazeMoveHeap
{
    MazeMove *arr;
    int len;
    int cap;
} MazeMoveHeap;

// IO
int parse_input(char *input_file, Point **points, int *point_count);
void print_grid(char **grid, int map_size);

// Problem steps
long long get_min_steps(Point *obstacles, int obstacles_size, int grid_size);

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
    int point_count = (argc >= 4) ? atoi(argv[3]) : 12;

    Point *points;

    if (parse_input(input_file, &points, &point_count))
        return 1;

    printf("Maximum number of steps: %lld\n", get_min_steps(points, point_count, grid_size));

    free(points);
    return 0;
}

/// @brief Parse input.txt
/// @param input_file The file to read
/// @param points Out: The point array to read into
/// @param point_count In/out: The maximum number of points to read. Will be updated to reflect the number of points actually read
/// @return 0 if success. 1 if failure.
int parse_input(char *input_file, Point **points, int *point_count)
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
    int max_points = *point_count;
    *point_count = 0;
    *points = malloc(sizeof((*points)[0]) * max_points);

    // Read while below maximum point count
    while (*point_count < max_points && fscanf(f, "%hd,%hd\n", &((*points)[*point_count].col), &((*points)[*point_count].row)) == 2)
        (*point_count)++;

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
/// @param obstacles The array of obstacles
/// @param obstacles_size The number of elements in `obstacles`
/// @param grid_size The number of rows/columns in the grid
/// @return The minimum number of steps needed to go from (0,0) to (`grid_size-1`,`grid_size-1`)
long long get_min_steps(Point *obstacles, int obstacles_size, int grid_size)
{
    long long min_steps = -1;
    MazeMoveHeap heap = {.arr = NULL, .len = 0, .cap = 0};
    Point target = {grid_size - 1, grid_size - 1};

    // Allocate grid
    char **grid = malloc(sizeof(grid[0]) * grid_size);
    for (int i = 0; i < grid_size; i++)
    {
        // Allocate row
        grid[i] = malloc(sizeof(grid[0][0]) * (grid_size + 1));
        // Set the entire grid to '.'
        memset(grid[i], '.', grid_size);
        // Set the last character to '\0' to end the string
        grid[i][grid_size] = '\0';
    }

    for (int i = 0; i < obstacles_size; i++)
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

    print_grid(grid, grid_size);

    MazeMove this_move = {.p = {0, 0}, .steps = 0};
    while (this_move.steps >= 0)
    {
        // Skip if this point was already found
        if (grid[this_move.p.row][this_move.p.col] != '.')
            goto NEXT_MOVE;

        // Mark this point as found
        grid[this_move.p.row][this_move.p.col] = 'O';

        // Right
        if (this_move.p.col + 1 < grid_size && grid[this_move.p.row][this_move.p.col + 1] == '.')
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row, .col = this_move.p.col + 1}, .steps = this_move.steps + 1});
        // Up
        if (this_move.p.row > 0 && grid[this_move.p.row - 1][this_move.p.col] == '.')
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row - 1, .col = this_move.p.col}, .steps = this_move.steps + 1});
        // Left
        if (this_move.p.col > 0 && grid[this_move.p.row][this_move.p.col - 1] == '.')
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row, .col = this_move.p.col - 1}, .steps = this_move.steps + 1});
        // Down
        if (this_move.p.row + 1 < grid_size && grid[this_move.p.row + 1][this_move.p.col] == '.')
            push_pq(&heap, (MazeMove){.p = {.row = this_move.p.row + 1, .col = this_move.p.col}, .steps = this_move.steps + 1});

    // Next point in the heap
    NEXT_MOVE:
        this_move = pop_pq(&heap);

        // Check if we found the end
        if (this_move.p.row == target.row && this_move.p.col == target.col)
        {
            min_steps = this_move.steps;
            break;
        }
    }

    printf("\n");
    print_grid(grid, grid_size);
    printf("\n");

CLEANUP:
    for (int i = 0; i < grid_size; i++)
        free(grid[i]);
    free(grid);
    free(heap.arr);
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
