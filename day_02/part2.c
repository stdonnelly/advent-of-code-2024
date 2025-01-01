#include <stdio.h>
#include <stdlib.h>

#include "../c-data-structures/vector/vector_template.h"

DEF_VEC(int)

void print_arr(int *arr, int size);
int_Vec parse_input_row(FILE *input_text);
int is_safe(int *report, size_t report_size);

int main(int argc, char const *argv[])
{
    FILE *f = fopen("input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }
    // Get the first row
    int_Vec row = parse_input_row(f);

    // For counting safe reports
    int safe_count = 0;

    // Loop while the row contains any data
    while (row.len)
    {
        print_arr(row.arr, row.len);
        printf("\n");
        if (is_safe(row.arr, row.len))
            safe_count++;
        free(row.arr);
        row = parse_input_row(f);
    }

    printf("Safe report count: %d\n", safe_count);
    return 0;
}

void print_arr(int *arr, int size)
{
    if (size <= 0)
    {
        printf("[]");
        return;
    }

    printf("[%d", arr[0]);
    for (int i = 1; i < size; i++)
    {
        printf(" %d", arr[i]);
    }
    printf("]");
}

int_Vec parse_input_row(FILE *input_text)
{
    int_Vec report = new_int_Vec();
    int current_number = 0;
    int ch = getc(input_text);
    // Parse character by character
    while (ch != '\n' && ch != EOF)
    {
        if (ch == ' ')
        {
            // If a space is found, commit the last number and restart
            append_int_Vec(&report, current_number);
            current_number = 0;
        }
        else
        {
            // If it is a digit instead, concatenate to the current number
            current_number = (current_number * 10) + (ch - '0');
        }
        ch = getc(input_text);
    }
    // Assuming there was any numbers, append__Vec the last number
    if (current_number)
        append_int_Vec(&report, current_number);

    return report;
}

#define MIN_CHANGE 1
#define MAX_CHANGE 3
int safe_pair(int decreasing, int left, int right);

int is_safe(int *report, size_t report_size)
{
    // Get a consensus of if the report should increase or decrease using the first 3 changes
    // This is crude, but gets the job done
    int decreasing = (report[0] > report[1]) + (report[1] > report[2]) + (report[2] > report[3]);
    decreasing = decreasing >= 2;
    int unsafe_ignored = 0;
    // Loop over all adjacent pairs in the report
    for (int i = 0; i < report_size - 1; i++)
    {
        // Check if the two adjacent reports are unsafe
        if (!safe_pair(decreasing, report[i], report[i + 1]))
        {
            // If we have already ignored an unsafety, just return false
            if (unsafe_ignored)
                return 0;
            
            // If we haven't already ignored one unsafety, try to remove either offending report
            if (i == report_size - 2 || safe_pair(decreasing, report[i], report[i + 2]))
            {
                i++;
                unsafe_ignored = 1;
            }
            else if (i == 0 || safe_pair(decreasing, report[i - 1], report[i + 1]))
            {
                unsafe_ignored = 1;
            }
            else
            {
                return 0;
            }
        }
    }

    // Assume safe if nothing unsafe is found
    return 1;
}

// Test if a pair is safe
int safe_pair(int decreasing, int left, int right)
{
    // If it should be decreasing, determine the amount it is decreasing by
    // If it should be increasing, determine the amount it is increasing by
    int change = decreasing ? (left - right) : (right - left);
    return MIN_CHANGE <= change && change <= MAX_CHANGE;
}