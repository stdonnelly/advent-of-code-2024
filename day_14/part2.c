#include <stdio.h>
#include <stdlib.h>

#include "../c-data-structures/vector/vector_template.h"

#define MAP_WIDTH 101
#define MAP_HEIGHT 103

// Doing this ahead of time because I feel like part 2 will be similar but with larger numbers
// Just need to remember to replace "%hd" with the appropriate flag
typedef short coord_t;

typedef struct Point
{
    coord_t x;
    coord_t y;
} Point;

typedef struct Robot
{
    // Position
    Point pos;
    // Velocity
    Point vel;
} Robot;

DEF_VEC(Robot)

int parse_input(char *input_file, Robot_Vec *robots);
void print_robots(Robot *robots, size_t robots_size);
void find_tree(Robot *robots, size_t robots_size, int iterations, int triangle_size);
Point get_destination(Robot robot);
void print_map(short map[MAP_HEIGHT][MAP_WIDTH]);
int has_triangle(short map[MAP_HEIGHT][MAP_WIDTH], int triangle_size);

int main(int argc, char *argv[])
{
    Robot_Vec robots;
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    if (parse_input(input_file, &robots))
        return 1;

    int iterations = (argc >= 3) ? atoi(argv[2]) : 100;
    int triangle_size = (argc >= 4) ? atoi(argv[3]) : 3;
    // print_robots(robots.arr, robots.len);
    find_tree(robots.arr, robots.len, iterations, triangle_size);
    free(robots.arr);
    return 0;
}

/// @brief Parse the input file into `robots`
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param claw_machines Out: The vector of robots
/// @return 0 if success, non-zero if failure
int parse_input(char *input_file, Robot_Vec *robots)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *robots = new_Robot_Vec();

    Robot robot;
    while (fscanf(
               f,
               "p=%hd,%hd v=%hd,%hd\n",
               &robot.pos.x,
               &robot.pos.y,
               &robot.vel.x,
               &robot.vel.y) == 4)
    {
        if (ferror(f))
        {
            fprintf(stderr, "Error reading input\n");
            free(robots->arr);
            robots->arr = NULL;
            return 1;
        }
        append_Robot_Vec(robots, robot);
    }
    return 0;
}

// For debugging: print the array of robots
void print_robots(Robot *robots, size_t robots_size)
{
    for (size_t i = 0; i < robots_size; i++)
        printf("p=%hd,%hd v=%hd,%hd\n", robots[i].pos.x, robots[i].pos.y, robots[i].vel.x, robots[i].vel.y);
}

/// @brief Print out a tree candidate with its number of seconds
/// @param robots The array of robots
/// @param robots_size The number of robots in the array
/// @param iterations The number of iterations to try
/// @param triangle_size The minimum height of a triangle that will be considered valid
void find_tree(Robot *robots, size_t robots_size, int iterations, int triangle_size)
{
    short map[MAP_HEIGHT][MAP_WIDTH];

    // Fill the map initially
    for (size_t i = 0; i < robots_size; i++)
        map[robots[i].pos.y][robots[i].pos.x]++;

    for (int i = 0; i < iterations; i++)
    {
        if (has_triangle(map, triangle_size))
        {
            printf("\nSeconds: %d\n", i);
            print_map(map);
        }

        for (size_t j = 0; j < robots_size; j++)
        {
            // Remove robot from old location
            map[robots[j].pos.y][robots[j].pos.x]--;
            // Move robot to new location
            robots[j].pos = get_destination(robots[j]);
            map[robots[j].pos.y][robots[j].pos.x]++;
        }
    }
}

/// @brief Get the destination of the robot after one second
/// @param robot
/// @return The destination of the robot
Point get_destination(Robot robot)
{
    return (Point){
        ((robot.vel.x + robot.pos.x) % MAP_WIDTH + MAP_WIDTH) % MAP_WIDTH,
        ((robot.vel.y + robot.pos.y) % MAP_HEIGHT + MAP_HEIGHT) % MAP_HEIGHT,
    };
}

void print_map(short map[MAP_HEIGHT][MAP_WIDTH])
{
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
            putchar(map[i][j] ? '@' : ' ');
        putchar('\n');
    }
}

int has_triangle(short map[MAP_HEIGHT][MAP_WIDTH], int triangle_size)
{
    // Loop over every row
    for (int i = 0; i < MAP_HEIGHT - triangle_size; i++)
    {
        // Loop over every column within the row for triangle tops
        for (int j = triangle_size; j < MAP_WIDTH - triangle_size; j++)
        {
            // Loop over the rows in the triangle
            for (int k = 0; k < triangle_size; k++)
            {
                // Loop over the columns in each row of the triangle
                for (int l = j - k; l <= j + k; l++)
                {
                    // If anything in the next triangle does not have a robot where it should be, check the next tip in the second loop (over j)
                    if (!map[i + k][l])
                        goto CHECK_NEXT_TRIANGLE;
                }
            }
            // If everything checks out for this triangle
            return 1;
        CHECK_NEXT_TRIANGLE:
        }
    }
    return 0;
}
