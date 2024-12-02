#include <stdlib.h>
#include <string.h>

void merge_sort(int *arr, int arr_size)
{
    // A length 1 array or an empty array are always sorted
    if (arr_size <= 1)
    {
        return;
    }

    // Put the left and right halves in new temporary arrays to be sorted by recursive call
    int mid = arr_size / 2;
    int *left = malloc(sizeof(int) * mid);
    int *right = malloc(sizeof(int) * arr_size - mid);
    memcpy(left, arr, sizeof(int) * mid);
    memcpy(right, arr + mid, sizeof(int) * (arr_size - mid));

    // Sort the left and right halves
    merge_sort(left, mid);
    merge_sort(right, arr_size - mid);

    // Merge
    int left_cursor = 0;
    int right_cursor = 0;
    int destination_cursor = 0;

    while (left_cursor < mid && right_cursor < arr_size - mid)
    {
        // Merge the smaller number into the destination array
        if (left[left_cursor] < right[right_cursor])
            arr[destination_cursor++] = left[left_cursor++];
        else
            arr[destination_cursor++] = right[right_cursor++];
    }

    // Copy the rest, if applicable
    while (left_cursor < mid)
        arr[destination_cursor++] = left[left_cursor++];
    while (right_cursor < arr_size - mid)
        arr[destination_cursor++] = right[right_cursor++];

    // Free the temporary arrays
    free(left);
    free(right);
}