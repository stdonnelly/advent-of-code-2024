#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../c-data-structures/vector/vector_template.h"

#define MOVE_COST 1
#define TURN_COST 1000
#define START_DIR EAST

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

typedef struct MazeMove
{
    int score;
    short row;
    short col;
    Direction dir;
} MazeMove;

typedef struct MazeMoveHeap
{
    MazeMove *arr;
    int len;
    int cap;
} MazeMoveHeap;

typedef struct PathNode
{
    struct PathNode *next[4];
    Point location;
} PathNode;

typedef PathNode *p_PathNode;
DEF_VEC(p_PathNode)

// IO
int parse_input(char *input_file, string_Vec *map, Point *start, Point *end);
void print_map(char **map, size_t map_size, Point start, Point end);

// Sub problems
int get_min_score_paths(char **map, size_t map_size, Point start, Point end);
int check_is_found(char *found_map, int map_size, int map_row_size, int row, int col, Direction direction);
void mark_found(char *found_map, int map_size, int map_row_size, int row, int col, Direction direction);

// Priority queue stuff
void resize_pq(MazeMoveHeap *heap, int new_cap);
void push_pq(MazeMoveHeap *heap, MazeMove val);
void pop_pq(MazeMoveHeap *heap);
void sift_down_pq(MazeMoveHeap *heap, int parent_index);
MazeMove peek_pq(MazeMoveHeap heap);

// Path node
void put_path_node(PathNode *head, Point source, Point destination, Direction direction, char **map);
PathNode *find_path_node(PathNode *head, Point source, char **map);
void delete_path(PathNode *head);
int mark_all_solutions(PathNode *head, char **map, Point end);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    string_Vec map;
    Point start;
    Point end;

    if (parse_input(input_file, &map, &start, &end))
        return 1;

    int min_score = get_min_score_paths(map.arr, map.len, start, end);

    print_map(map.arr, map.len, start, end);

    printf("Lowest possible score: %d\n", min_score);

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
/// @return The minimum score to solve the maze
int get_min_score_paths(char **map, size_t map_size, Point start, Point end)
{
    int min_score = INT_MAX;
    // The number of columns in `map`
    size_t map_row_size = map_size ? strlen(map[0]) : 0;
    // A map of what elements we have already found, all zeroed because nothing has been found yet
    char *found_map = calloc(map_size * map_row_size, sizeof(found_map[0]));
    MazeMoveHeap heap = {.arr = NULL, .len = 0, .cap = 0};

    // Create the graph of paths
    PathNode *paths_head = calloc(1, sizeof(PathNode));
    paths_head->location = start;

    // Mark the start
    Point this_point = start;
    Point last_point;
    Direction last_direction;
    Direction this_direction = START_DIR;
    // The start is free
    int this_score = 0;
    while (this_score <= min_score)
    {
        if (this_score && (last_point.row != this_point.row || last_point.col != this_point.col))
            // Prevent this from happening on the first iteration
            put_path_node(paths_head, last_point, this_point, last_direction, map);

        // Mark found because this is the next point
        mark_found(found_map, map_size, map_row_size, this_point.row, this_point.col, this_direction);

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

            if (valid_point && (map[next_point.row][next_point.col] == '.') && !check_is_found(found_map, map_size, map_row_size, next_point.row, next_point.col, this_direction))
            {
                push_pq(&heap, (MazeMove){.score = this_score + MOVE_COST, .row = next_point.row, .col = next_point.col, .dir = this_direction});
            }
        }

        // Add turn left and right to the heap with some modulo arithmetic
        // Left
        Direction next_dir = (this_direction + 1) & 0b11;
        if (!check_is_found(found_map, map_size, map_row_size, this_point.row, this_point.col, next_dir))
            push_pq(&heap, (MazeMove){.score = this_score + TURN_COST, .row = this_point.row, .col = this_point.col, .dir = next_dir});
        // Right
        next_dir = (this_direction - 1) & 0b11;
        if (!check_is_found(found_map, map_size, map_row_size, this_point.row, this_point.col, next_dir))
            push_pq(&heap, (MazeMove){.score = this_score + TURN_COST, .row = this_point.row, .col = this_point.col, .dir = next_dir});

        // Finally get the cheapest next move
        MazeMove next_move = peek_pq(heap);
        pop_pq(&heap);
        last_point = this_point;
        this_point.row = next_move.row;
        this_point.col = next_move.col;
        last_direction = this_direction;
        this_direction = next_move.dir;
        this_score = next_move.score;

        // Check for solution
        if (this_point.row == end.row && this_point.col == end.col && this_score < min_score)
            min_score = this_score;
    }
    mark_all_solutions(paths_head, map, end);
    int square_count = 0;
    for (size_t i = 0; i < map_size; i++)
        for (size_t j = 0; map[i][j]; j++)
            if (map[i][j] == 'O')
                square_count++;
CLEANUP:
    delete_path(paths_head);
    free(heap.arr);
    free(found_map);
    return square_count;
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
    IDX_2D(found_map, row, col) |= 1 << direction;
}

/// @brief Check if a position has been marked as found
/// @param found_map The map of found positions
/// @param map_size The number of rows in the map
/// @param map_row_size The number of columns in the map
/// @param row The row to check
/// @param col The column to check
/// @param direction The direction to check
/// @return 0 if not marked, (1 << direction) (i.e. some nonzero value) if found
int check_is_found(char *found_map, int map_size, int map_row_size, int row, int col, Direction direction)
{
    return IDX_2D(found_map, row, col) & (1 << direction);
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

// Put a path from source to destination into the graph
void put_path_node(PathNode *head, Point source, Point destination, Direction direction, char **map)
{
    PathNode *destination_node = find_path_node(head, destination, map);
    if (!destination_node)
    {
        destination_node = calloc(1, sizeof(*destination_node));
        destination_node->location = destination;
    }

    PathNode *source_node = find_path_node(head, source, map);
    if (!source_node)
    {
        // Panic
        fprintf(stderr, "No source node %hd,%hd\n", source.row, source.col);
        exit(1);
    }
    source_node->next[direction] = destination_node;
}

// Find a source node in the path graph
PathNode *find_path_node(PathNode *head, Point source, char **map)
{
    if (!head || map[head->location.row][head->location.col] == 'T')
        return NULL;

    if (head->location.row == source.row && head->location.col == source.col)
        return head;

    // Temporary mark to prevent loops
    map[head->location.row][head->location.col] = 'T';

    PathNode *node;
    for (int i = 0; i < sizeof(head->next) / sizeof(head->next[0]); i++)
    {
        node = find_path_node(head->next[i], source, map);
        if (node)
            break;
    }

    // Unmark
    map[head->location.row][head->location.col] = '.';

    return node;
}

// TODO: Delete a path graph
void delete_path(PathNode *head)
{
    // TODO
}

int mark_all_solutions(PathNode *head, char **map, Point end)
{
    if (!head || map[head->location.row][head->location.col] == 'T')
        return 0;

    if (end.row == head->location.row && end.col == head->location.col)
    {
        map[head->location.row][head->location.col] = 'O';
        return 1;
    }

    if (map[head->location.row][head->location.col] == 'O')
        return 1;

    // Temporary mark to prevent loops
    map[head->location.row][head->location.col] = 'T';

    int should_mark = 0;
    for (int i = 0; i < 4; i++)
    {
        if (mark_all_solutions(head->next[i], map, end))
            should_mark = 1;
    }

    map[head->location.row][head->location.col] = should_mark ? 'O' : '.';

    return should_mark;
}