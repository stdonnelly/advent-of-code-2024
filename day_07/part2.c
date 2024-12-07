#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Long long vector
// Manually defined instead of using the template because `long long` has a space
typedef struct LLVec
{
    long long *arr;
    size_t len;
    size_t cap;
} LLVec;
void appendLLVec(LLVec *vec, long long val)
{
    if (vec->len + 1 > vec->cap)
    {
        // Double if there is any capacity, otherwise make it one
        vec->cap = vec->cap ? vec->cap * 2 : 1;
        vec->arr = realloc(vec->arr, sizeof(long long) * vec->cap);
    }
    vec->arr[vec->len++] = val;
}
LLVec newLLVec() { return (LLVec){NULL, 0UL, 0UL}; }

long long parse_input_line(FILE *f, LLVec *operands);
int valid_values(long long target, long long *operands, size_t operands_size);
void print_arr(long long *arr, size_t arr_size);
long long un_concat_10(long long operand, long long sum);

int main(int argc, char const *argv[])
{
    long long total = 0LL;
    LLVec operands = newLLVec();
    FILE *f = fopen("input.txt", "r");

    long long target = parse_input_line(f, &operands);
    while (target)
    {
        printf("%lld:", target);
        print_arr(operands.arr, operands.len);
        printf("\n");
        // Check if the values make a possible calculation
        if (valid_values(target, operands.arr, operands.len))
        {
            printf("\tPOSSIBLE\n");
            total += target;
        }
        else
            printf("\tIMPOSSIBLE\n");

        // Reset operands array
        // Don't worry about shrinking it because we are about to just put more elements into it.
        operands.len = 0UL;

        // Get the next line
        target = parse_input_line(f, &operands);
    }

    printf("Total calibration result: %lld\n", total);

    free(operands.arr);
    fclose(f);
    return 0;
}

/// @brief Parse a line of the input file to get the target value and the operands
/// @param f The input file
/// @param operands An initialized vector of `long long` where the operands will be placed
/// @return The target value, or 0 if the end of the file has been reached
long long parse_input_line(FILE *f, LLVec *operands)
{
    // Return early if we can't find the target
    long long target;
    if (fscanf(f, "%lld:", &target) == EOF)
        return 0LL;

    // Loop until a newline is found
    long long operand;
    while (getc(f) != '\n' && !feof(f) && fscanf(f, "%lld", &operand))
        appendLLVec(operands, operand);

    return target;
}

/// @brief Check if `target` can be achieved by applying multiplication and addition to the operands using the rules of AoC 2024 day 7
/// @param target The target value
/// @param operands The operands
/// @param operands_size The number of elements in `operands`
/// @return 1 if possible, 0 if impossible
int valid_values(long long target, long long *operands, size_t operands_size)
{
    // printf("\ttarget: %15lld | array size: %lu\n", target, operands_size);
    // As a small optimization, we will  tart at the end and either subtract or divide the target
    // This will avoid checking some invalid solutions

    // If there are no more operands, return true if we have already removed the target
    if (!operands_size)
        return target == 0LL;

    // If the target is already non-positive and there are more operands, there is nothing we can do to make the target from the remaining operands
    if (target <= 0LL)
        return 0;

    // Subtract the operand size for this iteration and the next one
    // Prefix decrement because the last element is size-1
    long long this_operand = operands[--operands_size];

    // Try to subtract first
    if (valid_values(target - this_operand, operands, operands_size) ||
           // If that failed, try to divide iff target is a multiple
           (!(target % this_operand) && valid_values(target / this_operand, operands, operands_size)))
        return 1;
    else
    {
        long long un_concatenated = un_concat_10(this_operand, target);
        return (un_concatenated != -1LL) && valid_values(un_concatenated, operands, operands_size);
    }
}

void print_arr(long long *arr, size_t arr_size)
{
    for (size_t i = 0; i < arr_size; i++)
        printf(" %lld", arr[i]);
}

/// @brief If possible, remove `operand` from the least significant digits of `sum`
/// @param operand The least significant digits to remove
/// @param sum The digits to remove from
/// @return `sum` shifted right (in base 10) to remove `operand` or -1 if not possible
long long un_concat_10(long long operand, long long sum)
{
    if (sum < operand)
        // Always impossible when the sum is less than the operand
        return -1;

    // Find the smallest power of 10 larger than operand
    long long power_of_10 = 1;
    while (power_of_10 <= operand)
        power_of_10 *= 10;

    if ((sum - operand) % power_of_10)
        // If the sum - operand is not divisible by 10, the unconcatenation cannot work
        return -1;
    else
        // If this is a power of 10, just truncate the least significant (log(operand)) digits
        return sum / power_of_10;
}
