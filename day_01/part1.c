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
    printf("Total distance: %d\n", similarity_score(left_list.arr, right_list.arr, left_list.len));
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

/// @brief Get the total distance between each index in the arrays after sorting
/// @param left The left array
/// @param right The right array
/// @param n The length of both arrays
/// @return The total distance
int similarity_score(int *left, int *right, size_t n)
{
    printf("Sorting\n");
    // Sort the arrays
    merge_sort(left, n);
    merge_sort(right, n);
    printf("Done sorting\n");

    // Sum the distances in the sorted arrays
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        int distance = left[i] - right[i];
        // Absolute value
        if (distance < 0)
            distance = -distance;
        sum += distance;
    }

    return sum;
}
