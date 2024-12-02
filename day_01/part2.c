#include <stdio.h>
#include <stdlib.h>

#include "../vector_template.h"
#include "../merge_sort.h"

DEF_VEC(int)

int parse_input(intVec *left_list, intVec *right_list);
int similarity_score(int *left, int *right, size_t n);

int main(int argc, char const *argv[])
{
    // Parse the input
    intVec left_list, right_list;
    if (parse_input(&left_list, &right_list))
        return 1;

    // Validate
    if (left_list.len != right_list.len)
    {
        fprintf(stderr, "Left and right lists do not have the same length.\nLeft length: %zd | Right length: %zd", left_list.len, right_list.len);
        return 1;
    }

    // Do the actual calculations and print the result
    printf("Similarity score: %d\n", similarity_score(left_list.arr, right_list.arr, left_list.len));
    free(left_list.arr);
    free(right_list.arr);
    return 0;
}

int parse_input(intVec *left_list, intVec *right_list)
{
    *left_list = newintVec();
    *right_list = newintVec();

    FILE *f = fopen("input.txt", "r");

    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    int left, right;
    while (fscanf(f, "%d   %d\n", &left, &right) == 2)
    {
        printf("%d   %d\n", left, right);
        appendint(left_list, left);
        appendint(right_list, right);
    }

    fclose(f);
    return 0;
}

/// @brief Get the similarity score of the arrays
/// @param left The left array
/// @param right The right array
/// @param n The length of both arrays
/// @return The similarity score
int similarity_score(int *left, int *right, size_t n)
{
    printf("Sorting\n");
    // Sort the arrays
    merge_sort(left, n);
    merge_sort(right, n);
    printf("Done sorting\n");

    // Sum the distances in the sorted arrays
    int sum = 0;
    int last_left_number = -1;
    int left_similarity;
    int right_cursor = 0;
    for (int i = 0; i < n; i++)
    {
        int left_number = left[i];
        if (left_number != last_left_number)
        {
            // If this is the same as the last one we tried, just use the same score
            // Otherwise, deal with the right number
            // Skip all numbers less than the left number
            left_similarity = 0;
            while (right_cursor < n && right[right_cursor] < left_number)
                right_cursor++;
            // Count the numbers on the right that are the same as the left number
            while (right_cursor < n && right[right_cursor] == left_number)
            {
                left_similarity += left_number;
                right_cursor++;
            }
        }
        sum += left_similarity;
    }

    return sum;
}
