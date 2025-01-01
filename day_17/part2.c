#include <stdio.h>
#include <stdlib.h>

#include "../c-data-structures/vector/vector_template.h"

DEF_VEC(int)

// Instruction pointer
int ip = 0;
// "Registers"
long long ra, rb, rc;
// Output for `out` function
int output;

int parse_input(char *input_file, int_Vec *program);
void execute_program(int *program, size_t program_size, long long initial_ra);
long long find_initial_ra(int *program, size_t program_size, int this_digit_index, long long previous_digits);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    // int ra, rb, rc;
    int_Vec program;

    if (parse_input(input_file, &program))
        return 1;

    printf("Initial Register A: %lld\n", find_initial_ra(program.arr, program.len, program.len - 1, 0));

    free(program.arr);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The input file
/// @param program Out: The program array
/// @return 0 if successful, 1 if failed
int parse_input(char *input_file, int_Vec *program)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    int scanf_result = fscanf(f, "Register A: %lld\n", &ra);
    if (!scanf_result || (scanf_result == EOF))
    {
        fprintf(stderr, "Error reading register A\n");
        return 1;
    }
    scanf_result = fscanf(f, "Register B: %lld\n", &rb);
    if (!scanf_result || (scanf_result == EOF))
    {
        fprintf(stderr, "Error reading register B\n");
        return 1;
    }
    scanf_result = fscanf(f, "Register C: %lld\n", &rc);
    if (!scanf_result || (scanf_result == EOF))
    {
        fprintf(stderr, "Error reading register C\n");
        return 1;
    }

    int program_num;
    scanf_result = fscanf(f, "\nProgram: %d", &program_num);
    if (!scanf_result || (scanf_result == EOF))
    {
        fprintf(stderr, "Error reading program\n");
        return 1;
    }

    // Initialize program array
    program->arr = malloc(sizeof(program->arr[0]));
    program->len = 0;
    program->cap = 1;

    // Loop over the program array
    while (scanf_result && (scanf_result != EOF))
    {
        append_int_Vec(program, program_num);
        scanf_result = fscanf(f, ",%d", &program_num);
    }

    return 0;
}

// Solution

// A flag to determine if a comma should be printed before the `out` instruction
// int should_print_comma = 0;

// Instructions
void adv(int op);
void bxl(int op);
void bst(int op);
void jnz(int op);
void bxc(int op);
void out(int op);
void bdv(int op);
void cdv(int op);
int get_combo_op(int op);

// Array of length 8 of pointers to functions returning void that take 1 integer argument
// What the hell is this syntax, C?
const void (*instructions[])(int) = {adv, bxl, bst, jnz, bxc, out, bdv, cdv};

void execute_program(int *program, size_t program_size, long long initial_ra)
{
    ip = 0;
    ra = initial_ra;
    rb = 0;
    rc = 0;
    // The maximum valid instruction pointer (exclusive)
    int too_large_ip = program_size - 1;
    while (ip < too_large_ip)
    {
        int inst = program[ip++];
        int op = program[ip++];
        instructions[inst](op);
    }
}

int get_combo_op(int op)
{
    switch (op)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        return op;
        break;
    case 4:
        return ra;
        break;
    case 5:
        return rb;
        break;
    case 6:
        return rc;
        break;
    default:
        fprintf(stderr, "Unexpected combo operator %d\n", op);
        exit(1);
    }
}

// Division: ra = ra / (2^combo)
void adv(int op)
{
    ra >>= get_combo_op(op);
}

// Bitwise XOR: rb = rb XOR literal
void bxl(int op)
{
    rb ^= op;
}

// rb = combo mod 8
void bst(int op)
{
    rb = get_combo_op(op) & 07;
}

// Jump if not zero: Jumps to literal if ra != 0
void jnz(int op)
{
    // Disabled so we only iterate once
    // if (ra)
    //     ip = op;
}

// bitwise XOR: rb = rb XOR rc. op is ignored
void bxc(int op)
{
    rb ^= rc;
}

// Print combo mod 8
void out(int op)
{
    // Conditional logic to make sure a comma is only printed if this isn't the first `out`
    // if (should_print_comma)
    //     putchar(',');
    // else
    //     should_print_comma = 1;

    // Print combo mod 8
    // putchar((get_combo_op(op) & 07) + '0');
    output = get_combo_op(op) & 07;
}

// Division: rb = ra / (2^combo)
void bdv(int op)
{
    rb = ra >> get_combo_op(op);
}

// Division: rc = ra / (2^combo)
void cdv(int op)
{
    rc = ra >> get_combo_op(op);
}

long long find_initial_ra(int *program, size_t program_size, int this_digit_index, long long previous_digits)
{
    if (this_digit_index == -1)
        return previous_digits;
    long long initial_ra = previous_digits;
    // Loop over program backwards
    // for (int i = 0; i < program_size; i++)
    // {
    initial_ra <<= 3;
    int target = program[this_digit_index];
    long long digit;
    for (digit = 0; digit < 010; digit++)
    {
        execute_program(program, program_size, initial_ra + digit);
        if (output == target)
        {
            // break;
            long long next_iteration = find_initial_ra(program, program_size, this_digit_index - 1, initial_ra + digit);
            if (next_iteration != -1)
                return next_iteration;
        }
    }

    // Panic if a digit wasn't found
    if (digit == 8)
    {
        printf("Impossible\n");
        return -1;
    }
    else
    {
        printf("Digit: %lld\n", digit);
    }

    // initial_ra += digit;
    // }
    // printf("Impossible\n");
    return -1;
}
