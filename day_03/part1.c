#include <stdio.h>

// Assert ch == c and goes to the next character. If not, continue the while loop
#define ASSERT_CHAR(c) \
    if (ch != c)       \
        continue;      \
    else               \
        ch = getc(f)

int is_digit(int ch) { return '0' <= ch && ch <= '9'; }

int main(int argc, char const *argv[])
{
    int product_sum = 0;
    // Left and right operands
    int lhs, rhs;
    // Because file IO, I'm just putting the whole thing into one function
    FILE *f = fopen("input.txt", "r");

    // Loop over characters
    int ch = getc(f);
    while (ch != EOF)
    {
        // Seek until 'm'
        while (ch != 'm' && ch != EOF)
            ch = getc(f);

        // Assert the rest of "mul("
        ASSERT_CHAR('m');
        ASSERT_CHAR('u');
        ASSERT_CHAR('l');
        ASSERT_CHAR('(');

        // Get digits
        lhs = 0;
        while (is_digit(ch))
        {
            lhs = (lhs * 10) + (ch - '0');
            ch = getc(f);
        }
        ASSERT_CHAR(',');
        rhs = 0;
        while (is_digit(ch))
        {
            rhs = (rhs * 10) + (ch - '0');
            ch = getc(f);
        }
        ASSERT_CHAR(')');

        // Multiply left and right operands and add to product sum
        product_sum += lhs * rhs;
    }

    printf("Product sum: %d\n", product_sum);
    return 0;
}
