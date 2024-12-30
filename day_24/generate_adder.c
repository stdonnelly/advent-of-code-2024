#include <stdio.h>

#define WIDTH 46

int main(int argc, char const *argv[])
{
    // 0 is special because it's only a half adder
    // Sum bit
    printf("x%02d XOR y%02d -> z%02d\n", 0, 0, 0);
    // Carry bit
    printf("x%02d AND y%02d -> c%02d\n", 0, 0, 0);

    // Loop over defining the full adders
    for (int i = 1; i < WIDTH; i++)
    {
        // Half sum
        printf("x%02d XOR y%02d -> h%02d\n", i, i, i);
        // Find the full sum by XORing the half sum and last carry
        printf("h%02d XOR c%02d -> z%02d\n", i, i-1, i);

        // Carry
        // From this x AND y
        printf("x%02d AND y%02d -> a%02d\n", i, i, i);
        // From last carry AND half sum
        printf("h%02d AND c%02d -> b%02d\n", i, i-1, i);
        // Or those two
        printf("a%02d OR b%02d -> c%02d\n", i, i, i);
    }
    return 0;
}
