#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"

#define ITERATIONS 2000
// 16777216-1
#define PRUNE_BITMASK 16777215

DEF_VEC(int)

int parse_input(char *input_file, intVec *nums);
int get_secret_number_sum(int *nums, size_t nums_size);
int get_secret_number_sum_with_sequence(size_t nums_size, short sequence[4]);
int find_best_change_sequence(int *nums, size_t nums_size);
int get_next_number(int num);

// 2D array of price sequences
// Row is monkey, index is the number of iterations (0 is the initial state)
short **price_sequences;
// 2D array of price sequence differences
// Row is monkey, index is price_sequences[monkey][i+1] - price_sequences[monkey][i]
short **price_diffs;

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    intVec nums;

    if (parse_input(input_file, &nums))
        return 1;

    // Allocate everything
    price_sequences = malloc(sizeof(price_sequences[0]) * nums.len);
    price_diffs = malloc(sizeof(price_diffs[0]) * nums.len);
    for (size_t i = 0; i < nums.len; i++)
    {
        price_sequences[i] = malloc(sizeof(price_sequences[0][0]) * (ITERATIONS + 1));
        price_diffs[i] = malloc(sizeof(price_diffs[0][0]) * ITERATIONS);
    }

    int secret_number_sum = get_secret_number_sum(nums.arr, nums.len);

    printf("Sum of numbers after %d iterations: %d\n", ITERATIONS, secret_number_sum);
    printf("Best sum: %d\n", find_best_change_sequence(nums.arr, nums.len));

    for (size_t i = 0; i < nums.len; i++)
    {
        free(price_sequences[i]);
        free(price_diffs[i]);
    }
    free(price_sequences);
    free(price_diffs);
    free(nums.arr);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param nums Out: the vector of numbers in the input file
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, intVec *nums)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *nums = newintVec();

    int num;
    while (fscanf(f, "%d\n", &num) == 1)
        appendint(nums, num);

    return 0;
}

/// @brief Get the sum of the numbers after `ITERATIONS` hashes
/// @param nums The array of seed numbers
/// @param nums_size The number of elements in `nums`
/// @return The sum of the numbers after the hash
int get_secret_number_sum(int *nums, size_t nums_size)
{
    int sum = 0;
    for (size_t i = 0; i < nums_size; i++)
    {
        int num = nums[i];
        price_sequences[i][0] = num % 10;
        int mod_10;
        for (int j = 0; j < ITERATIONS; j++)
        {
            num = get_next_number(num);
            mod_10 = num % 10;
            price_sequences[i][j + 1] = mod_10;
            price_diffs[i][j] = price_sequences[i][j + 1] - price_sequences[i][j];
        }
        // printf("%8d: %8d\n", nums[i], mod_10);
        sum += mod_10;
    }
    return sum;
}

/// @brief Get the next secret number using the given hash function
/// @param num The number
/// @return The hashed number
int get_next_number(int num)
{
    int temp;
    // Calculate the result of multiplying the secret number by 64. Then, mix this result into the secret number. Finally, prune the secret number.
    temp = num << 6;
    // Mix: bitwise XOR
    num ^= temp;
    // Prune: mod 16777216. 16777216 is a power of 2, so we can just AND
    num &= PRUNE_BITMASK;

    // Calculate the result of dividing the secret number by 32. Round the result down to the nearest integer. Then, mix this result into the secret number. Finally, prune the secret number.
    temp = num >> 5;
    num ^= temp;
    num &= PRUNE_BITMASK;

    // Calculate the result of multiplying the secret number by 2048. Then, mix this result into the secret number. Finally, prune the secret number.
    // This will cause an overflow, but we don't care because the overflowed bits will be discarded by prune
    temp = num << 11;
    num ^= temp;
    num &= PRUNE_BITMASK;

    return num;
}

/// @brief Find the best sequence of changes
/// @param nums The array of numbers
/// @param nums_size The number of elements in `nums`
/// @return The change sequence that will yield the largest secret number sum
int find_best_change_sequence(int *nums, size_t nums_size)
{
    short this_change_sequence[4];
    int best_sum = -1;

    // Check every change sequence
    for (short i = -9; i <= 9; i++)
    {
        printf("%d\n", i);
        this_change_sequence[0] = i;
        for (short j = -9; j <= 9; j++)
        {
            printf("\t%d\n", j);
            this_change_sequence[1] = j;
            for (short k = -9; k <= 9; k++)
            {
                this_change_sequence[2] = k;
                for (short l = -9; l <= 9; l++)
                {
                    this_change_sequence[3] = l;
                    int this_sum = get_secret_number_sum_with_sequence(nums_size, this_change_sequence);
                    if (this_sum > best_sum)
                        best_sum = this_sum;
                }
            }
        }
    }

    return best_sum;
}

int get_secret_number_sum_with_sequence(size_t nums_size, short sequence[4])
{
    int sum = 0;
    for (int i = 0; i < nums_size; i++)
    {
        for (int j = 0; j <= ITERATIONS - 4; j++)
        {
            if (!memcmp(price_diffs[i] + j, sequence, sizeof(short) * 4))
            {
                sum += price_sequences[i][j+4];
                break;
            }
        }
    }
    return sum;
}
