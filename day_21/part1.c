#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"

#define CODE_LENGTH 4
#define CODE_COUNT 5

// Constant to tell what row (top to bottom) a digit is on the keypad
const short ROW_OF[] = {
    3,
    2, 2, 2,
    1, 1, 1,
    0, 0, 0,
    3};

// Constant to tell which column (left to right) a digit is on the keypad
const short COL_OF[] = {
    1,
    0, 1, 2,
    0, 1, 2,
    0, 1, 2,
    2};

typedef struct Code
{
    char buttons[CODE_LENGTH];
} Code;

int parse_input(char *input_file, Code **codes, int *codes_size);
long long get_total_complexity(Code *codes, int codes_size);
int get_code_numeric_part(Code code);
long long get_shortest_sequence(Code code);
short get_row_of(char key, int is_numpad);
short get_col_of(char key, int is_numpad);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    Code *codes;
    int codes_size;

    if (parse_input(input_file, &codes, &codes_size))
        return 1;

    long long total_complexity = get_total_complexity(codes, codes_size);

    printf("\nTotal complexity: %lld\n", total_complexity);

    free(codes);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param codes Out: An array of CODE_LENGTH digit codes
/// @param codes_size Out: The number of elements in `codes`
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, Code **codes, int *codes_size)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *codes_size = CODE_COUNT;
    *codes = malloc(sizeof(*codes[0]) * *codes_size);

    for (int i = 0; i < *codes_size; i++)
    {
        // Exit early if bad read
        if (fscanf(f, "%4c\n", (*codes)[i].buttons) != 1)
        {
            free(*codes);
            fprintf(stderr, "Error reading codes\n");
            return 1;
        }
    }

    return 0;
}

/// @brief Get the "total complexity" (sum of products of (code * shortest sequence)) of all codes
/// @param codes The array of codes
/// @param codes_size The number of elements in `codes`
/// @return The total complexity of all codes
long long get_total_complexity(Code *codes, int codes_size)
{
    long long complexity_sum = 0;

    // Iterate over all codes
    for (int i = 0; i < codes_size; i++)
    {
        printf("%.4s:\n", codes[i].buttons);

        int numeric_part = get_code_numeric_part(codes[i]);
        long long shortest_sequence = get_shortest_sequence(codes[i]);

        printf("%lld\n", shortest_sequence);
        complexity_sum += shortest_sequence * numeric_part;
    }

    return complexity_sum;
}

// Get the numeric part of a code
int get_code_numeric_part(Code code)
{
    int numeric_part = 0;

    for (int i = 0; i < sizeof(code.buttons) / sizeof(code.buttons[0]); i++)
    {
        if ('0' <= code.buttons[i] && code.buttons[i] <= '9')
        {
            numeric_part *= 10;
            numeric_part += code.buttons[i] - '0';
        }
        else
            break;
    }

    return numeric_part;
}

/// @brief Get the row for a key
/// @param key The key's character
/// @param is_numpad 1 if numpad, 0 if dpad
/// @return the row for the key
short get_row_of(char key, int is_numpad)
{
    short row;
    if (is_numpad)
    {
        if (key == 'A')
            row = ROW_OF[10];
        else if ('0' <= key && key <= '9')
            row = ROW_OF[key - '0'];
        else
        {
            fprintf(stderr, "Unable to find row on numpad of '%c'\n", key);
            exit(1);
        }
    }
    else
    {
        // Keypad
        switch (key)
        {
        case '^':
        case 'A':
            row = 0;
            break;
        case '<':
        case 'v':
        case '>':
            row = 1;
            break;
        default:
            fprintf(stderr, "Unable to find row on numpad for '%c'\n", key);
            exit(1);
        }
    }

    return row;
}

/// @brief Get the column for a key
/// @param key The key's character
/// @param is_numpad 1 if numpad, 0 if dpad
/// @return the column for the key
short get_col_of(char key, int is_numpad)
{
    short col;
    if (is_numpad)
    {
        if (key == 'A')
            col = COL_OF[10];
        else if ('0' <= key && key <= '9')
            col = ROW_OF[key - '0'];
        else
        {
            fprintf(stderr, "Unable to find column on numpad of '%c'\n", key);
            exit(1);
        }
    }
    else
    {
        // Keypad
        switch (key)
        {
        case '<':
            col = 0;
        case '^':
        case 'v':
            col = 1;
        case 'A':
        case '>':
            col = 2;
        default:
            fprintf(stderr, "Unable to find column on numpad for '%c'\n", key);
            exit(1);
        }
    }

    return col;
}

/// @brief Get the shortest sequence of inputs that can produce the given code
/// @param code The code to produce
/// @return The length of the shortest sequence of inputs that will produce `code`
long long get_shortest_sequence(Code code)
{
    long long sequence_cost = 0;
    // Loop over desired keys
    short last_key = 10;
    for (int i = 0; i < sizeof(code.buttons) / sizeof(code.buttons[0]); i++)
    {
        // Parse the button in a way that it can be indexed in the lookup arrays
        short this_key = code.buttons[i];
        if (this_key == 'A')
            this_key = 10;
        else
            this_key = this_key - '0';

        short row_displacement = get_row_of(this_key, 1) - get_row_of(last_key, 1);
        short col_displacement = get_col_of(this_key, 1) - get_col_of(last_key, 1);

        printf("\t");

        /* FIXME: This finds a solution, but not the shortest one.
           Some move orders can be optimized further, so try all permutations.
           When doing that, it can be assumed that there will be at most 2 directional inputs per number press in the optimal solution.
           It can probably also be assumed that splitting directions will never create an optimal solution (eg. ^>^> is never better than ^^>> or >>^^).
           Which would leave at most 2 orders to try. (eg. ^^>> and >>^^)
        */

        // if (col_displacement > 0)
        // {
        //     // Handle right
        //     // Displacement of 1:
        //     // numpad 1: >
        //     // numpad 2: vA^
        //     // me:       v<A>^A<A> = 9 = 8+1

        //     sequence_cost += col_displacement + 8;
        //     for (int i = 0; i < col_displacement; i++)
        //         putchar('>');
        // }

        // if (row_displacement < 0)
        // {
        //     // Handle up
        //     // displacement of 1:
        //     // numpad 1: ^
        //     // numpad 2: <A>
        //     // me:       v<<A>>^AvA^ = 11 = 10+1
        //     // displacement of 2:
        //     // numpad 1: ^^
        //     // numpad 2: <AA>
        //     // me:       v<<AA>>^AvA^ = 12 = 10+2

        //     sequence_cost += (-row_displacement) + 10;
        //     for (int i = row_displacement; i < 0; i++)
        //         putchar('^');
        // }

        // if (row_displacement > 0)
        // {
        //     // Handle down
        //     // Displacement of 1:
        //     // numpad 1: v
        //     // numpad 2: v<A>^
        //     // me:       v<A<A>>^AvA^<A> = 15 = 14 + 1

        //     sequence_cost += row_displacement + 14;
        //     for (int i = 0; i < row_displacement; i++)
        //         putchar('v');
        // }

        // if (col_displacement < 0)
        // {
        //     // Handle left
        //     // Displacement of 1:
        //     // numpad 1: <
        //     // numpad 2: v<<A>>^
        //     // me:       v<A<AA>>^AvAA^<A> = 17 = 16 + 1
        //     //           <vA<AA>>^AvAA<^A>

        //     sequence_cost += (-col_displacement) + 16;
        //     for (int i = col_displacement; i < 0; i++)
        //         putchar('<');
        // }

        // For the actual press
        sequence_cost += 1;

        last_key = this_key;

        // printf("A\n");
    }

    return sequence_cost;
}
