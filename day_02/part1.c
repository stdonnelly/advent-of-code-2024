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

int is_safe(int *report, size_t report_size)
{
    int decreasing = report[0] > report[1];

    // Loop over all adjacent pairs in the report
    for (int i = 0; i < report_size - 1; i++)
    {
        // Get the difference
        int change = report[i+1] - report[i];
        // Flip if decreasing because we want the decrease
        if (decreasing)
            change = -change;
        
        // Unsafe
        if (change < MIN_CHANGE || change > MAX_CHANGE)
            return 0;
    }

    // Assume safe if nothing unsafe is found
    return 1;
}
