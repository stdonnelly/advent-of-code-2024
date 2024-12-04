#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"

// Define char and char* vectors
typedef char *string;
DEF_VEC(char)
DEF_VEC(string)
void delete_string_vec(stringVec *vec)
{
    for (int i = 0; i < vec->len; i++)
        free(vec->arr[i]);
    free(vec->arr);
    vec->arr = NULL;
    vec->len = 0UL;
    vec->cap = 0UL;
}

// Load input.txt into stringVec
stringVec parse_input();
// Count all matches in the crossword
int count_matches(char **crossword, size_t row_count);
// Returns 1 if  crossword[row][col] is the a in an "X-MAS"
int matches_with_start(char **crossword, size_t row_count, size_t row, size_t col);

int main(int argc, char const *argv[])
{
    stringVec crossword = parse_input();
    printf("Match count: %d\n", count_matches(crossword.arr, crossword.len));
    delete_string_vec(&crossword);
    return 0;
}

stringVec parse_input()
{
    // Open input.txt or panic
    FILE *f = fopen("input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        exit(1);
    }

    stringVec output = newstringVec();
    // Loop over characters in the file
    int ch;
    charVec row = newcharVec();
    while ((ch = getc(f)) != EOF)
    {
        if (ch == '\n')
        {
            // Add terminator for the string and put it on the string vector
            appendchar(&row, '\0');
            appendstring(&output, row.arr);
            // Generate a new char vector for the next iteration
            row = newcharVec();
        }
        else
            appendchar(&row, (char)ch);
    }
    // If relevant, append the last row. Otherwise, just free the row
    if (row.len > 0)
    {
        appendchar(&row, '\0');
        appendstring(&output, row.arr);
    }
    else
        free(row.arr);

    return output;
}

// Count matches of "XMAS" in any direction
int count_matches(char **crossword, size_t row_count)
{
    int sum = 0;
    // Only try to find 'A's that are not on the edge, so rows [1,row_count-1] and columns [1,?] (doesn't matter because null-terminated)
    for (size_t row = 1; row < row_count - 1; row++)
        for (size_t col = 1; crossword[row][col]; col++)
            sum += matches_with_start(crossword, row_count, row, col);
    return sum;
}

int matches_with_start(char **crossword, size_t row_count, size_t row, size_t col)
{
    // Check if 'A'
    return crossword[row][col] == 'A' &&
           /* Check for
              M..    S..
              .A. or .A.
              ..S    ..M
           */
           ((crossword[row - 1][col - 1] == 'M' && crossword[row + 1][col + 1] == 'S') ||
            (crossword[row - 1][col - 1] == 'S' && crossword[row + 1][col + 1] == 'M')) &&
           /* Check for
             ..M    ..S
             .A. or .A.
             S..    M..
           */
           ((crossword[row + 1][col - 1] == 'M' && crossword[row - 1][col + 1] == 'S') ||
            (crossword[row + 1][col - 1] == 'S' && crossword[row - 1][col + 1] == 'M'));
}
