#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../c-data-structures/vector/vector_template.h"

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

int parse_input(char *input_file, char ***codes, int *codes_size);
long long get_total_complexity(char **codes, int codes_size);
int get_code_numeric_part(char *code);
long long get_shortest_sequence(char *code, int is_numpad, int depth);
short get_row_of(char key, int is_numpad);
short get_col_of(char key, int is_numpad);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    char **codes;
    int codes_size;

    if (parse_input(input_file, &codes, &codes_size))
        return 1;

    long long total_complexity = get_total_complexity(codes, codes_size);

    printf("\nTotal complexity: %lld\n", total_complexity);

    for (int i = 0; i < codes_size; i++)
        free(codes[i]);
    free(codes);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param codes Out: An array of CODE_LENGTH digit codes
/// @param codes_size Out: The number of elements in `codes`
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, char ***codes, int *codes_size)
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
        (*codes)[i] = malloc(sizeof((*codes)[0][0]) * (CODE_LENGTH + 1));
        // Exit early if bad read
        if (fscanf(f, "%4s\n", (*codes)[i]) != 1)
        {
            for (int j = 0; j < i; j++)
                free((*codes)[j]);
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
long long get_total_complexity(char **codes, int codes_size)
{
    long long complexity_sum = 0;

    // Iterate over all codes
    for (int i = 0; i < codes_size; i++)
    {
        printf("%.4s: ", codes[i]);

        int numeric_part = get_code_numeric_part(codes[i]);
        long long shortest_sequence = get_shortest_sequence(codes[i], 1, 2);

        printf("%lld\n", shortest_sequence);
        complexity_sum += shortest_sequence * numeric_part;
    }

    return complexity_sum;
}

// Get the numeric part of a code
int get_code_numeric_part(char *code)
{
    int numeric_part = 0;

    for (int i = 0; code[i]; i++)
    {
        if ('0' <= code[i] && code[i] <= '9')
        {
            numeric_part *= 10;
            numeric_part += code[i] - '0';
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
            col = COL_OF[key - '0'];
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
            break;
        case '^':
        case 'v':
            col = 1;
            break;
        case 'A':
        case '>':
            col = 2;
            break;
        default:
            fprintf(stderr, "Unable to find column on keypad for '%c'\n", key);
            exit(1);
        }
    }

    return col;
}

/// @brief Get the shortest sequence of inputs that can produce the given code
/// @param code The code to produce
/// @param is_numpad 1 if this is a numpad, 0 if this is a dpad
/// @param depth The amount of indirection
/// @return The length of the shortest sequence of inputs that will produce `code`
long long get_shortest_sequence(char *code, int is_numpad, int depth)
{
    long long sequence_cost = 0;
    // Loop over desired keys
    char last_key = 'A';
    for (int i = 0; code[i]; i++)
    {
        // Parse the button in a way that it can be indexed in the lookup arrays
        char this_key = code[i];
        short row_displacement = get_row_of(this_key, is_numpad) - get_row_of(last_key, is_numpad);
        short col_displacement = get_col_of(this_key, is_numpad) - get_col_of(last_key, is_numpad);

        /*
           Some move orders can be optimized further, so try all permutations.
           When doing that, it can be assumed that there will be at most 2 directional inputs per number press in the optimal solution.
           It can probably also be assumed that splitting directions will never create an optimal solution (eg. ^>^> is never better than ^^>> or >>^^).
           Which would leave at most 2 orders to try. (eg. ^^>> and >>^^)
        */

        // Only do the following if there is any depth left
        if (depth)
        {
            // Either repeating < or repeating >
            char right_left_moves[3] = {0};
            // Either repeating ^ or repeating v
            char up_down_moves[4] = {0};
            char all_moves[7] = {0};

            if (col_displacement > 0)
            {
                // Right
                for (int i = 0; i < col_displacement; i++)
                    right_left_moves[i] = '>';
            }
            else if (col_displacement < 0)
            {
                // Left
                for (int i = 0; i < -col_displacement; i++)
                    right_left_moves[i] = '<';
            }
            if (row_displacement < 0)
            {
                // Up
                for (int i = 0; i < -row_displacement; i++)
                    up_down_moves[i] = '^';
            }
            else if (row_displacement > 0)
            {
                // Down
                for (int i = 0; i < row_displacement; i++)
                    up_down_moves[i] = 'v';
            }

            // Find the best way to organize it

            int up_down_first_cost;
            int right_left_first_cost;

            // Ensure up_down first won't cause panic
            if (
                // Numpad dead zone
                (is_numpad && (get_col_of(last_key, 1) == 0) && (get_row_of(this_key, 1) == 3)) ||
                // dpad dead zone
                (!is_numpad && (last_key == '<') && (get_row_of(this_key, 0) == 0)))
            {
                up_down_first_cost = INT_MAX;
            }
            else
            {
                snprintf(all_moves, sizeof(all_moves), "%.*s%.*sA", (int)sizeof(up_down_moves), up_down_moves, (int)sizeof(right_left_moves), right_left_moves);
                up_down_first_cost = get_shortest_sequence(all_moves, 0, depth - 1);
            }

            // Ensure right-left first won't cause panic
            if (
                // Numpad dead zone
                (is_numpad && (get_row_of(last_key, 1) == 3) && (get_col_of(this_key, 1) == 0)) ||
                // dpad dead zone
                (!is_numpad && (get_row_of(last_key, 0) == 0) && (this_key == '<')))
            {
                // Will cause panic, so we set the cost to INT_MAX to prevent this from being used
                right_left_first_cost = INT_MAX;
            }
            else
            {
                snprintf(all_moves, sizeof(all_moves), "%.*s%.*sA", (int)sizeof(right_left_moves), right_left_moves, (int)sizeof(up_down_moves), up_down_moves);
                right_left_first_cost = get_shortest_sequence(all_moves, 0, depth - 1);
            }

            if (up_down_first_cost <= right_left_first_cost)
                sequence_cost += up_down_first_cost;
            else
                sequence_cost += right_left_first_cost;
        }
        else
            sequence_cost += ((row_displacement >= 0) ? row_displacement : -row_displacement) + ((col_displacement >= 0) ? col_displacement : -col_displacement) + 1;

        last_key = this_key;
    }

    return sequence_cost;
}
