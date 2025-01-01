#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "../c-data-structures/vector/vector_template.h"

#define A_PRICE 3
#define B_PRICE 1
#define MAX_BUTTON_PRESSES 100

typedef struct Point
{
    short x;
    short y;
} Point;

typedef struct ClawMachine
{
    Point button_a;
    Point button_b;
    Point target;
} ClawMachine;

DEF_VEC(ClawMachine)

int parse_input(char *input_file, ClawMachine_Vec *claw_machines);
long long count_total_tokens(ClawMachine *claw_machines, size_t claw_machines_size);
int min_tokens(ClawMachine claw_machine);
void print_claw_machine(ClawMachine claw_machine);
int min(int a, int b);

int main(int argc, char *argv[])
{
    ClawMachine_Vec claw_machines;
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    if (parse_input(input_file, &claw_machines))
        return 1;

    for (size_t i = 0; i < claw_machines.len; i++)
    {
        print_claw_machine(claw_machines.arr[i]);
        printf("\n");
    }
    printf("Total number of tokens: %lld\n", count_total_tokens(claw_machines.arr, claw_machines.len));
    free(claw_machines.arr);
    return 0;
}

/// @brief Parse the input file into `claw_machines`
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param claw_machines Out: The vector of claw machines
/// @return 0 if success, non-zero if failure
int parse_input(char *input_file, ClawMachine_Vec *claw_machines)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *claw_machines = new_ClawMachine_Vec();

    ClawMachine claw_machine;
    while (fscanf(
               f,
               "Button A: X%hd, Y%hd\nButton B: X%hd, Y%hd\nPrize: X=%hd, Y=%hd\n",
               &(claw_machine.button_a.x),
               &(claw_machine.button_a.y),
               &(claw_machine.button_b.x),
               &(claw_machine.button_b.y),
               &(claw_machine.target.x),
               &(claw_machine.target.y)) == 6)
    {
        if (ferror(f))
        {
            fprintf(stderr, "Error reading input\n");
            free(claw_machines->arr);
            claw_machines->arr = NULL;
            return 1;
        }
        append_ClawMachine_Vec(claw_machines, claw_machine);
    }
    return 0;
}

/// @brief For debugging. Print the claw machine information
/// @param claw_machine
void print_claw_machine(ClawMachine claw_machine)
{
    printf("Button A: X%+hd, Y%+hd\n", claw_machine.button_a.x, claw_machine.button_a.y);
    printf("Button B: X%+hd, Y%+hd\n", claw_machine.button_b.x, claw_machine.button_b.y);
    printf("Prize: X=%hd, Y=%hd\n", claw_machine.target.x, claw_machine.target.y);
}

int min(int a, int b)
{
    if (a <= b)
        return a;
    else
        return b;
}

/// @brief Count the fewest total number of tokens spent to get all possible prizes from the given claw machines
/// @param claw_machines The array of claw machines
/// @param claw_machines_size The number of elements in `claw_machines`
/// @return The fewest total number of tokens of spent
long long count_total_tokens(ClawMachine *claw_machines, size_t claw_machines_size)
{
    long long total_tokens = 0LL;
    for (size_t i = 0; i < claw_machines_size; i++)
        total_tokens += min_tokens(claw_machines[i]);
    return total_tokens;
}

/// @brief Count the fewest total number of tokens spent to get a prize, if possible
/// @param claw_machine The claw machine
/// @return The minimum number of tokens to get the prize, it possible. 0 otherwise.
int min_tokens(ClawMachine claw_machine)
{
    int min_price = INT_MAX;
    // Because b is so much cheaper than a, just assume maximum b and minimum a
    int a_presses = 0;
    int b_presses = MAX_BUTTON_PRESSES;

    b_presses = min(b_presses, claw_machine.target.x / claw_machine.button_b.x);
    b_presses = min(b_presses, claw_machine.target.y / claw_machine.button_b.y);

    Point current_point = {
        .x = b_presses * claw_machine.button_b.x,
        .y = b_presses * claw_machine.button_b.y,
    };

    // Sliding window to find the minimum tokens
    while (a_presses <= MAX_BUTTON_PRESSES && b_presses >= 0)
    {
        if (current_point.x > claw_machine.target.x || current_point.y > claw_machine.target.y)
        {
            b_presses--;
            current_point.x -= claw_machine.button_b.x;
            current_point.y -= claw_machine.button_b.y;
        }
        else
        {
            if (current_point.x == claw_machine.target.x && current_point.y == claw_machine.target.y)
                // Equal
                min_price = min(min_price, a_presses * A_PRICE + b_presses * B_PRICE);
            a_presses++;
            current_point.x += claw_machine.button_a.x;
            current_point.y += claw_machine.button_a.y;
        }
    }

    // Return the minimum price if any possible amount of tokens was found
    // Otherwise, return 0
    return min_price != INT_MAX ? min_price : 0;
}
