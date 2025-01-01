#include <stdio.h>
#include <stdlib.h>

#include "../c-data-structures/vector/vector_template.h"

#define MAP_WIDTH 101
#define MAP_HEIGHT 103
#define MOVE_COUNT 100

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
long long find_tree(Robot *robots, size_t robots_size);
Point get_destination(Robot robot);

int main(int argc, char *argv[])
{
    Robot_Vec robots;
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    if (parse_input(input_file, &robots))
        return 1;

    print_robots(robots.arr, robots.len);
    printf("Safety_factor: %lld\n", find_tree(robots.arr, robots.len));
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

/// @brief Get the "safety factor" from a list of robots
/// @param robots The array of robots
/// @param robots_size The number of robots in the array
/// @return The product of the number of robots in each quadrant
long long find_tree(Robot *robots, size_t robots_size)
{
    long long robots_per_quadrant[4] = {0};
    Point mid = {MAP_WIDTH / 2, MAP_HEIGHT / 2};

    for (size_t i = 0; i < robots_size; i++)
    {
        Point dest = get_destination(robots[i]);
        if (dest.x < mid.x)
        {
            // Left side
            if (dest.y < mid.y)
                // Up
                robots_per_quadrant[1]++;
            else if (dest.y > mid.y)
                // Down
                robots_per_quadrant[2]++;
        }
        else if (dest.x > mid.x)
        {
            // Right side
            if (dest.y < mid.y)
                // Up
                robots_per_quadrant[0]++;
            else if (dest.y > mid.y)
                // Down
                robots_per_quadrant[3]++;
        }
        // Else, on a midpoint: ignore
    }

    long long safety_factor = 1;
    for (int i = 0; i < sizeof(robots_per_quadrant) / sizeof(robots_per_quadrant[0]); i++)
        safety_factor *= robots_per_quadrant[i];
    return safety_factor;
}

/// @brief Get the destination of the robot after `MOVE_COUNT` seconds
/// @param robot
/// @return
Point get_destination(Robot robot)
{
    return (Point){
        ((robot.vel.x * MOVE_COUNT + robot.pos.x) % MAP_WIDTH + MAP_WIDTH) % MAP_WIDTH,
        ((robot.vel.y * MOVE_COUNT + robot.pos.y) % MAP_HEIGHT + MAP_HEIGHT) % MAP_HEIGHT,
    };
}
