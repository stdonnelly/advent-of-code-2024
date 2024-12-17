#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#include "../vector_template.h"

#define MOVE_COST 1
#define TURN_COST 1000
#define START_DIR EAST

// Helper to index a 1D array as 2D
#define IDX_2D(arr, row, col) arr[((row) * map_row_size) + (col)]

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

typedef enum Direction
{
    EAST = 0,
    NORTH = 1,
    WEST = 2,
    SOUTH = 3,
} Direction;

typedef struct Point
{
    short row;
    short col;
} Point;

DEF_VEC(Point)

typedef struct MazeMove
{
    int score;
    short row;
    short col;
    Direction dir;
    bool is_forward_move;
} MazeMove;

typedef struct MazeMoveHeap
{
    MazeMove *arr;
    int len;
    int cap;
} MazeMoveHeap;

typedef struct FoundSquare
{
    int scores_per_direction[4];
    // An array to tell any best path involving the square pointing in some direction involves a move instead of a turn
    bool parents_per_direction[4];
} FoundSquare;

// IO
int parse_input(char *input_file, stringVec *map, Point *start, Point *end);
void print_map(char **map, size_t map_size, Point start, Point end);

// Sub problems
int get_min_score_paths(char **map, size_t map_size, Point start, Point end);
int check_is_found(FoundSquare *found_map, int map_size, int map_row_size, int row, int col, Direction direction);
void mark_found(FoundSquare *found_map, int map_size, int map_row_size, int row, int col, int score, bool from_parent, Direction direction);
void append_if_not_exists(PointVec *vec, Point point);
void make_min_paths(FoundSquare *found_map, int map_size, int map_row_size, int row, int col, /*Direction direction,*/ PointVec *path_squares);

// Priority queue stuff
void resize_pq(MazeMoveHeap *heap, int new_cap);
void push_pq(MazeMoveHeap *heap, MazeMove val);
void pop_pq(MazeMoveHeap *heap);
void sift_down_pq(MazeMoveHeap *heap, int parent_index);
MazeMove peek_pq(MazeMoveHeap heap);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    stringVec map;
    Point start;
    Point end;

    if (parse_input(input_file, &map, &start, &end))
        return 1;

    int path_count = get_min_score_paths(map.arr, map.len, start, end);

    print_map(map.arr, map.len, start, end);

    printf("Lowest possible score path square count: %d\n", path_count);

    delete_string_vec(&map);
    return 0;
}

/// @brief Parse the input file into the map, start, and end
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param map Out: The maze map
/// @param start Out: The starting point
/// @param end Out: The ending point
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, stringVec *map, Point *start, Point *end)
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
        switch (ch)
        {
        case '.':
        case '#':
            appendchar(&row, (char)ch);
            break;
        case 'S':
            start->row = map->len;
            start->col = row.len;
            appendchar(&row, '.');
            break;
        case 'E':
            end->row = map->len;
            end->col = row.len;
            appendchar(&row, '.');
            break;
        case '\n':
            // Add terminator for the string and put it on the string vector
            appendchar(&row, '\0');
            appendstring(map, row.arr);
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
        puts(map[i]);

    // Cleanup by resetting those points
    map[start.row][start.col] = '.';
    map[end.row][end.col] = '.';
}

/// @brief Find the minimum "score" to solve the maze
/// @param map The map of the maze
/// @param map_size The number of rows in `map`
/// @param start The start point
/// @param end The end point
/// @return The number of squares in the path of the minimum score
int get_min_score_paths(char **map, size_t map_size, Point start, Point end)
{
    int min_score = INT_MAX;
    // The number of columns in `map`
    size_t map_row_size = map_size ? strlen(map[0]) : 0;
    size_t flattened_size = map_size * map_row_size;
    // A map of what elements we have already found, all zeroed because nothing has been found yet
    FoundSquare *found_map = malloc(flattened_size * sizeof(found_map[0]));
    // Set everything in the found map
    for (size_t i = 0; i < flattened_size; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            found_map[i].parents_per_direction[j] = false;
            found_map[i].scores_per_direction[j] = -1;
        }
    }
    MazeMoveHeap heap = {.arr = NULL, .len = 0, .cap = 0};

    bool is_forward_move = false;
    Point this_point = start;
    Direction this_direction = START_DIR;
    // The start is free
    int this_score = 0;
    // Loop until we start to look at higher scores that the target point
    while (this_score <= min_score)
    {
        // Mark found because this is the next point
        mark_found(found_map, map_size, map_row_size, this_point.row, this_point.col, this_score, is_forward_move, this_direction);

        // Add move forward to the queue if it's possible and not yet found
        {
            Point next_point = this_point;
            int valid_point;
            // Find next point and check for validity
            switch (this_direction)
            {
            case EAST:
                valid_point = ++next_point.col < map_row_size;
                break;
            case NORTH:
                valid_point = --next_point.row >= 0;
                break;
            case WEST:
                valid_point = --next_point.col >= 0;
                break;
            case SOUTH:
                valid_point = ++next_point.row < map_size;
                break;
            default:
                // Panic
                fprintf(stderr, "Unknown direction %d\n", this_direction);
                goto CLEANUP;
            }

            if (valid_point && (map[next_point.row][next_point.col] == '.'))
            {
                int found_score = check_is_found(found_map, map_size, map_row_size, next_point.row, next_point.col, this_direction);
                if (found_score == this_score + MOVE_COST)
                    mark_found(found_map, map_size, map_row_size, next_point.row, next_point.col, this_score + MOVE_COST, true, this_direction);
                if (found_score == -1 || found_score == this_score + MOVE_COST)
                    push_pq(&heap, (MazeMove){.score = this_score + MOVE_COST, .row = next_point.row, .col = next_point.col, .dir = this_direction, .is_forward_move = true});
            }
        }

        // Add turn left and right to the heap with some modulo arithmetic
        // Left
        Direction next_dir = (this_direction + 1) & 0b11;
        if (check_is_found(found_map, map_size, map_row_size, this_point.row, this_point.col, next_dir) == -1)
            push_pq(&heap, (MazeMove){.score = this_score + TURN_COST, .row = this_point.row, .col = this_point.col, .dir = next_dir, .is_forward_move = false});
        // Right
        next_dir = (this_direction - 1) & 0b11;
        if (check_is_found(found_map, map_size, map_row_size, this_point.row, this_point.col, next_dir) == -1)
            push_pq(&heap, (MazeMove){.score = this_score + TURN_COST, .row = this_point.row, .col = this_point.col, .dir = next_dir, .is_forward_move = false});

        // Finally get the cheapest next move
        MazeMove next_move = peek_pq(heap);
        pop_pq(&heap);
        this_point.row = next_move.row;
        this_point.col = next_move.col;
        this_direction = next_move.dir;
        this_score = next_move.score;
        is_forward_move = next_move.is_forward_move;

        // Check for solution
        if (this_point.row == end.row && this_point.col == end.col && this_score < min_score)
            min_score = this_score;
    }

    // Temporary: print all found square parents
    // for (int i = 0; i < map_size; i++)
    // {
    //     for (int j = 0; j < map_size; j++)
    //     {
    //         int identifier = 0;
    //         for (int k = 3; k >= 0; k--)
    //         {
    //             identifier = identifier << 1;
    //             if (IDX_2D(found_map, i, j).parents_per_direction[k])
    //                 identifier |= 1;
    //         }
    //         printf("%x", identifier);
    //     }
    //     printf("\n");
    // }

    // Find all squares in the path
    PointVec paths = newPointVec();
    // Check all parent directions of the end space
    make_min_paths(found_map, map_size, map_row_size, end.row, end.col, &paths);
    for (size_t i = 0; i < paths.len; i++)
        map[paths.arr[i].row][paths.arr[i].col] = 'O';
    free(paths.arr);
CLEANUP:
    free(heap.arr);
    free(found_map);
    return min_score;
}

/// @brief Mark a position on the map as found
/// @param found_map The map of found positions
/// @param map_size The number of rows in the map
/// @param map_row_size The number of columns in the map
/// @param row The row to mark
/// @param col The column to mark
/// @param score The score to mark
/// @param from_parent should be `true` if this is a move, or `false` if this is a turn
/// @param direction The direction to mark
void mark_found(FoundSquare *found_map, int map_size, int map_row_size, int row, int col, int score, bool from_parent, Direction direction)
{
    // Mark found with a bit, offset by the direction
    IDX_2D(found_map, row, col).scores_per_direction[direction] = score;
    IDX_2D(found_map, row, col).parents_per_direction[direction] = from_parent;
}

/// @brief Check if a position has been marked as found
/// @param found_map The map of found positions
/// @param map_size The number of rows in the map
/// @param map_row_size The number of columns in the map
/// @param row The row to check
/// @param col The column to check
/// @param direction The direction to check
/// @return The score of the point, direction if already found, -1 otherwise
int check_is_found(FoundSquare *found_map, int map_size, int map_row_size, int row, int col, Direction direction)
{
    return IDX_2D(found_map, row, col).scores_per_direction[direction];
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
    while ((heap->arr[this_element].score < heap->arr[parent].score) && this_element)
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
void pop_pq(MazeMoveHeap *heap)
{
    MazeMove new_root = heap->arr[--(heap->len)];
    heap->arr[0] = new_root;
    sift_down_pq(heap, 0);
}

/// @brief Sift down operation on heap
/// @param heap The heap
/// @param index The index of the parent element to sift down
void sift_down_pq(MazeMoveHeap *heap, int parent_index)
{
    int left = (parent_index * 2) + 1;
    int right = left + 1;
    int parent_val = heap->arr[parent_index].score;
    int min_val = parent_val;
    int min_index = parent_index;

    // Check if left is min
    if (left < heap->len)
    {
        int left_val = heap->arr[left].score;
        if (left_val < min_val)
        {
            min_index = left;
            min_val = left_val;
        }
    }

    // Check if right is min
    if (right < heap->len)
    {
        int right_val = heap->arr[right].score;
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

// Peek at the top of the heap
MazeMove peek_pq(MazeMoveHeap heap)
{
    return heap.arr[0];
}

// Append a value to the vector if it doesn't already exist
void append_if_not_exists(PointVec *vec, Point point)
{
    for (size_t i = 0; i < vec->len; i++)
        if (vec->arr[i].row == point.row && vec->arr[i].col == point.col)
            return;
    appendPoint(vec, point);
}

void make_min_paths(FoundSquare *found_map, int map_size, int map_row_size, int row, int col, /*Direction next_direction,*/ PointVec *path_squares)
{
    // Append this
    append_if_not_exists(path_squares, (Point){.row = row, .col = col});
    FoundSquare found_square = IDX_2D(found_map, row, col);
    int next_row = row;
    int next_col = col;

    for (Direction i = EAST; i <= SOUTH; i++)
    {
        if (found_square.parents_per_direction[i])
        {
            // We are checking behind, so this will be reversed (i.e. facing east means check paths to the west)
            switch (i)
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
                fprintf(stderr, "Unknown direction %d\n", i);
                return;
            }

            make_min_paths(found_map, map_size, map_row_size, next_row, next_col, path_squares);
        }
    }
}
