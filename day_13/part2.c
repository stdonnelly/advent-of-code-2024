#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "../vector_template.h"

#define A_PRICE 3
#define B_PRICE 1
#define INCREASE_AMOUNT 10000000000000LL

typedef struct Point
{
    long long x;
    long long y;
} Point;

typedef struct ClawMachine
{
    Point button_a;
    Point button_b;
    Point target;
} ClawMachine;

DEF_VEC(ClawMachine)

int parse_input(char *input_file, ClawMachineVec *claw_machines);
long long count_total_tokens(ClawMachine *claw_machines, size_t claw_machines_size);
long long min_tokens(ClawMachine claw_machine);
void print_claw_machine(ClawMachine claw_machine);
long long min(long long a, long long b);

int main(int argc, char *argv[])
{
    ClawMachineVec claw_machines;
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    if (parse_input(input_file, &claw_machines))
        return 1;

    // for (size_t i = 0; i < claw_machines.len; i++)
    // {
    //     print_claw_machine(claw_machines.arr[i]);
    //     printf("\n");
    // }
    printf("Total number of tokens: %lld\n", count_total_tokens(claw_machines.arr, claw_machines.len));
    free(claw_machines.arr);
    return 0;
}

/// @brief Parse the input file into `claw_machines`
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param claw_machines Out: The vector of claw machines
/// @return 0 if success, non-zero if failure
int parse_input(char *input_file, ClawMachineVec *claw_machines)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *claw_machines = newClawMachineVec();

    ClawMachine claw_machine;
    while (fscanf(
               f,
               "Button A: X%lld, Y%lld\nButton B: X%lld, Y%lld\nPrize: X=%lld, Y=%lld\n",
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
        claw_machine.target.x += INCREASE_AMOUNT;
        claw_machine.target.y += INCREASE_AMOUNT;
        appendClawMachine(claw_machines, claw_machine);
    }
    return 0;
}

/// @brief For debugging. Print the claw machine information
/// @param claw_machine
void print_claw_machine(ClawMachine claw_machine)
{
    printf("Button A: X%+lld, Y%+lld\n", claw_machine.button_a.x, claw_machine.button_a.y);
    printf("Button B: X%+lld, Y%+lld\n", claw_machine.button_b.x, claw_machine.button_b.y);
    printf("Prize: X=%lld, Y=%lld\n", claw_machine.target.x, claw_machine.target.y);
}

long long min(long long a, long long b)
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
    {
        print_claw_machine(claw_machines[i]);
        long long tokens = min_tokens(claw_machines[i]);
        printf("Tokens: %lld\n\n", tokens);
        total_tokens += tokens;
    }

    return total_tokens;
}

/// @brief Count the fewest total number of tokens spent to get a prize, if possible
/// @param claw_machine The claw machine
/// @return The minimum number of tokens to get the prize, it possible. 0 otherwise.
long long min_tokens(ClawMachine claw_machine)
{
    /*
    Given:
    1. target.x = (button_a.x * a_presses) + (button_b.x * b_presses)
    2. target.y = (button_a.y * a_presses) + (button_b.y * b_presses)
    3. price = 3*a_presses + b_presses
    */

    // Store the divisor for the formula that will generate a_presses in a variable so we can check if it's zero before dividing
    long long divisor = (claw_machine.button_b.y * claw_machine.button_a.x) - (claw_machine.button_b.x * claw_machine.button_a.y);
    if (!divisor)
        return 0;

    long long dividend = (claw_machine.button_b.y * claw_machine.target.x - claw_machine.button_b.x * claw_machine.target.y);
    if (dividend % divisor)
        return 0;

    long long a_presses = dividend / divisor;
    long long b_presses = claw_machine.target.y - claw_machine.button_a.y * a_presses;
    if (b_presses % claw_machine.button_b.y)
        return 0;
    b_presses /= claw_machine.button_b.y;
    printf("A: %lld\n", a_presses);
    printf("B: %lld\n", b_presses);
    return a_presses * 3 + b_presses;
}
