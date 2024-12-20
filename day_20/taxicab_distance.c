// This class exists so I can test a function that finds points at a certain taxicab/Manhattan distance from an origin point
// This is useful for enumerating cheats in part 2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRID_SIZE 25
#define DISTANCE 2

typedef struct Point
{
    short row;
    short col;
} Point;

void print_map(char **map, size_t map_size);
Point *find_points_at_taxicab_distance(Point center, int distance, size_t *return_size);

int main(int argc, char const *argv[])
{
    char **map = malloc(sizeof(map[0]) * GRID_SIZE);
    for (size_t i = 0; i < GRID_SIZE; i++)
    {
        map[i] = malloc(sizeof(map[0][0]) * (GRID_SIZE + 1));
        memset(map[i], '.', sizeof(map[0][0]) * GRID_SIZE);
        map[i][GRID_SIZE] = '\0';
    }

    int center = GRID_SIZE / 2;
    size_t displaced_points_size;
    Point *displaced_points = find_points_at_taxicab_distance((Point){center, center}, DISTANCE, &displaced_points_size);
    map[center][center] = 'O';
    for (size_t i = 0; i < displaced_points_size; i++)
        map[displaced_points[i].row][displaced_points[i].col] = '#';

    print_map(map, GRID_SIZE);

    for (size_t i = 0; i < GRID_SIZE; i++)
        free(map[i]);
    free(map);
    free(displaced_points);
    return 0;
}

/// @brief Print the map to the terminal
/// @param map The map of the maze
/// @param map_size The number of rows in `map`
/// @param start The start point
/// @param end The end point
void print_map(char **map, size_t map_size)
{
    // Print map
    for (size_t i = 0; i < map_size; i++)
    {
        for (size_t j = 0; map[i][j]; j++)
        {
            char ch = map[i][j];
            int color;
            switch (ch)
            {
            case '.':
                // Empty space: gray
                color = 90;
                break;
            case 'O':
                // Center: Yellow
                color = 33;
                break;
            default:
                // Anything else: white
                color = 37;
            }
            printf("\e[%dm%c", color, ch);
        }
        printf("\e[37m\n");
    }
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