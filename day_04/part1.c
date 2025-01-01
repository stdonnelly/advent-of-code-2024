#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../c-data-structures/vector/vector_template.h"

// Define char and char* vectors
typedef char *string;
DEF_VEC(char)
DEF_VEC(string)
void delete_string_vec(string_Vec *vec)
{
    for (int i = 0; i < vec->len; i++)
        free(vec->arr[i]);
    free(vec->arr);
    vec->arr = NULL;
    vec->len = 0UL;
    vec->cap = 0UL;
}

// Target word is "XMAS"
#define TARGET_WORD "XMAS"

// Load input.txt into string_Vec
string_Vec parse_input();
// Count all matches in the crossword
int count_matches(char **crossword, size_t row_count);
// Count all matches starting at a particular point
int matches_with_start(char **crossword, size_t row_count, size_t row, size_t col);
// Check if a row and column are valid, then check if the character matches at that index
int valid_and_matches(char **crossword, size_t row_count, int row, int col, char ch);

int main(int argc, char const *argv[])
{
    string_Vec crossword = parse_input();
    printf("Match count: %d\n", count_matches(crossword.arr, crossword.len));
    delete_string_vec(&crossword);
    return 0;
}

string_Vec parse_input()
{
    // Open input.txt or panic
    FILE *f = fopen("input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        exit(1);
    }

    string_Vec output = new_string_Vec();
    // Loop over characters in the file
    int ch;
    char_Vec row = new_char_Vec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch == '\n')
        {
            // Add terminator for the string and put it on the string vector
            append_char_Vec(&row, '\0');
            append_string_Vec(&output, row.arr);
            // Generate a new char vector for the next iteration
            row = new_char_Vec();
        }
        else
            append_char_Vec(&row, (char)ch);
    }
    // If relevant, append__Vec the last row. Otherwise, just free the row
    if (row.len > 0)
    {
        append_char_Vec(&row, '\0');
        append_string_Vec(&output, row.arr);
    }
    else
        free(row.arr);

    return output;
}

// Count matches of "XMAS" in any direction
int count_matches(char **crossword, size_t row_count)
{
    int sum = 0;
    for (size_t row = 0; row < row_count; row++)
        for (size_t col = 0; crossword[row][col]; col++)
            sum += matches_with_start(crossword, row_count, row, col);
    return sum;
}

int matches_with_start(char **crossword, size_t row_count, size_t row, size_t col)
{
    // Return early if this doesn't even start with 'X'
    if (crossword[row][col] != TARGET_WORD[0])
        return 0;

    // Start by assuming every direction matches
    int sum = 8;

    // Increment sum if matches in a particular direction
#define MATCH_DIRECTION(x, y)                                                       \
    for (int i = 1; i < sizeof(TARGET_WORD) - 1; i++)                               \
    {                                                                               \
        if (!valid_and_matches(crossword, row_count, row x, col y, TARGET_WORD[i])) \
        {                                                                           \
            sum--;                                                                  \
            break;                                                                  \
        }                                                                           \
    }
    // Up
    MATCH_DIRECTION(+i, )
    // Up-left
    MATCH_DIRECTION(+i, -i)
    // Left
    MATCH_DIRECTION(, -i)
    // Down-left
    MATCH_DIRECTION(-i, -i)
    // Down
    MATCH_DIRECTION(-i, )
    // Down-right
    MATCH_DIRECTION(-i, +i)
    // Right
    MATCH_DIRECTION(, +i)
    // Up-right
    MATCH_DIRECTION(+i, +i)

#undef MATCH_DIRECTION

    // Check downward
    return sum;
}

int valid_and_matches(char **crossword, size_t row_count, int row, int col, char ch)
{
    return (row >= 0) && (col >= 0) && (row < row_count) && (crossword[row][col] == ch);
}
