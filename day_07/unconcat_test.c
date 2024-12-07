#include <stdlib.h>
#include <stdio.h>

long long un_concat_10(long long operand, long long sum);

int main(int argc, char const *argv[])
{
    if (argc > 2)
        printf("%lld\n", un_concat_10(atoll(argv[1]), atoll(argv[2])));
    return 0;
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