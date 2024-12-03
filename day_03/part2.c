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
    int enabled = 1;
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
        while (ch != 'm' && ch != 'd' && ch != EOF)
            ch = getc(f);

        // If 'm' and enabled, do what the original program did
        if (ch == 'm')
        {
            if (enabled)
            {
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
            else
                // If disabled, just get the next character and continue
                ch = getc(f);
        }
        else
        {
            // If do/don't, try to toggle
            ASSERT_CHAR('d');
            ASSERT_CHAR('o');
            if (enabled)
            {
                // If enabled, we are only looking for don't()
                // If disabled, we are only looking for do()
                ASSERT_CHAR('n');
                ASSERT_CHAR('\'');
                ASSERT_CHAR('t');
            }
            ASSERT_CHAR('(');
            ASSERT_CHAR(')');
            // If we haven't hit a continue yet, flip enabled
            enabled = !enabled;
        }
    }

    printf("Product sum: %d\n", product_sum);
    return 0;
}
