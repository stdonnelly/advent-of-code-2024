#include <stdio.h>
#include <stdlib.h>

int gcd(int x, int y);

int main(int argc, char const *argv[])
{
    if (argc < 3)
        return 0;

    int x = atoi(argv[1]);
    int y = atoi(argv[2]);

    printf("GCD(%d,%d) = %d\n", x, y, gcd(x, y));
    return 0;
}

int gcd(int x, int y)
{
    // If y is 0, the greatest common denominator is x because this is the end of the algorithm
    if (!y)
        return x;
    // If not end, y becomes the remainder of x/y and x becomes the old y
    // This has the same effect as dividing the larger number by the smaller number and using the remainder instead of the larger number
    else
        return gcd(y, x % y);
}
