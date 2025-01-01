#include <stdio.h>
#include <stdlib.h>

#include "../c-data-structures/vector/vector_template.h"

#define ITERATIONS 2000
// 16777216-1
#define PRUNE_BITMASK 16777215

DEF_VEC(int)

int parse_input(char *input_file, int_Vec *nums);
long long get_secret_number_sum(int *nums, size_t nums_size);
int get_next_number(int num);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    int_Vec nums;

    if (parse_input(input_file, &nums))
        return 1;

    long long secret_number_sum = get_secret_number_sum(nums.arr, nums.len);

    printf("Sum of numbers after %d iterations: %lld\n", ITERATIONS, secret_number_sum);

    free(nums.arr);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param nums Out: the vector of numbers in the input file
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, int_Vec *nums)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *nums = new_int_Vec();

    int num;
    while (fscanf(f, "%d\n", &num) == 1)
        append_int_Vec(nums, num);

    return 0;
}

/// @brief Get the sum of the numbers after `ITERATIONS` hashes
/// @param nums The array of seed numbers
/// @param nums_size The number of elements in `nums`
/// @return The sum of the numbers after the hash
long long get_secret_number_sum(int *nums, size_t nums_size)
{
    long long sum = 0;
    for (size_t i = 0; i < nums_size; i++)
    {
        int num = nums[i];
        for (int j = 0; j < ITERATIONS; j++)
            num = get_next_number(num);
        printf("%8d: %8d\n", nums[i], num);
        sum += num;
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
