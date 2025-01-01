#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../c-data-structures/vector/vector_template.h"

typedef struct OrderRule
{
    short first;
    short second;
} OrderRule;

DEF_VEC(OrderRule)
DEF_VEC(short)
DEF_VEC(short_Vec)
void delete_short_vec_vec(short_Vec_Vec *vec)
{
    for (int i = 0; i < vec->len; i++)
    {
        free(vec->arr[i].arr);
    }
    free(vec->arr);
    vec->arr = NULL;
    vec->len = 0UL;
    vec->cap = 0UL;
}

int parse_input(OrderRule_Vec *order_rules, short_Vec_Vec *updates);
int sum_of_corrected_invalid_middles(OrderRule *order_rules, size_t order_rules_size, short_Vec_Vec updates);
bool is_valid_update(OrderRule *order_rules, size_t order_rules_size, short *pages, size_t pages_size);
short *pages_that_must_follow(OrderRule *active_order_rules, size_t active_order_rules_size, short n, size_t *following_pages_count);
bool arr_contains(short *arr, size_t arr_size, short n);

int main(int argc, char const *argv[])
{
    OrderRule_Vec order_rules;
    short_Vec_Vec updates;

    // Try to parse input
    if (parse_input(&order_rules, &updates))
        return 1;

    printf("Sum of valid middle pages: %d\n", sum_of_corrected_invalid_middles(order_rules.arr, order_rules.len, updates));

    free(order_rules.arr);
    delete_short_vec_vec(&updates);
    return 0;
}

/// @brief Parse input.txt for this puzzle's input
/// @param order_rules out parameter for the order rules vector
/// @param updates out parameter of updates as a 2D vector. Each row is an update consisting of multiple pages
/// @return `0` if successful. Some nonzero number if unsuccessful (i.e. IO error)
int parse_input(OrderRule_Vec *order_rules, short_Vec_Vec *updates)
{
    FILE *f = fopen("input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *order_rules = new_OrderRule_Vec();
    *updates = new_short_Vec_Vec();

    // Get the rule as a string because fscanf does weird things with whitespaces
    char rule_str[7];
    while (fgets(rule_str, sizeof(rule_str), f)[0] != '\n')
    {
        // rule_str should always be in the form "dd|dd\n"
        if (rule_str[0] < '0' || rule_str[0] > '9' ||
            rule_str[1] < '0' || rule_str[1] > '9' ||
            rule_str[2] != '|' ||
            rule_str[3] < '0' || rule_str[3] > '9' ||
            rule_str[4] < '0' || rule_str[4] > '9' ||
            rule_str[5] != '\n')
        {
            fprintf(stderr, "Unexpected line '%s' when reading rules.\n", rule_str);
            fclose(f);
            return 1;
        }
        // Parse as 2-digit base 10 numbers
        short first = ((rule_str[0] - '0') * 10) + (rule_str[1] - '0');
        short second = ((rule_str[3] - '0') * 10) + (rule_str[4] - '0');
        append_OrderRule_Vec(order_rules, (OrderRule){first, second});
    }
    if (ferror(f))
    {
        perror("Error reading from input file");
        fclose(f);
        return 1;
    }

    // Parse the updates
    // Loop over lines
    short first_page;
    while (fscanf(f, "%2hd", &first_page) == 1)
    {
        short_Vec update = {malloc(sizeof(short)), 1, 1};
        update.arr[0] = first_page;
        // Parse the rest of the update
        short page;
        while (fscanf(f, ",%2hd", &page) == 1)
            append_short_Vec(&update, page);
        // Add the new update
        append_short_Vec_Vec(updates, update);
    }

    fclose(f);
    return 0;
}

/// @brief Get the sum of the middle pages where the update is valid by is_valid_update()
/// @param order_rules The current order rules
/// @param order_rules_size The number of elements in `order_rules`
/// @param updates The list of updates
/// @return
int sum_of_corrected_invalid_middles(OrderRule *order_rules, size_t order_rules_size, short_Vec_Vec updates)
{
    int sum = 0;
    for (size_t i = 0; i < updates.len; i++)
    {
        if (!is_valid_update(order_rules, order_rules_size, updates.arr[i].arr, updates.arr[i].len))
            // Add the middle page to the sum
            sum += (int)(updates.arr[i].arr[updates.arr[i].len / 2]);
    }
    return sum;
}

/// @brief Determine if `pages` is in the correct order, given a set of order rules
/// @param order_rules The current order rules
/// @param order_rules_size The number of elements in `order_rules`
/// @param pages The pages in this update
/// @param pages_size The number of elements in `pages`
/// @return `true` if update is valid, `false` if update is invalid
bool is_valid_update(OrderRule *order_rules, size_t order_rules_size, short *pages, size_t pages_size)
{
    bool was_valid = true;
    // An order rule is "active" if the second page has already been found
    OrderRule_Vec active_order_rules = new_OrderRule_Vec();
    // Check each page
    for (size_t i = 0; i < pages_size; i++)
    {
        short page = pages[i];
        size_t following_pages_size;
        short *following_pages = pages_that_must_follow(active_order_rules.arr, active_order_rules.len, page, &following_pages_size);
        if (following_pages_size)
        {
            was_valid = false;
            // Find the first index with any of the pages that must follow
            size_t new_index = 0;
            while (!arr_contains(following_pages, following_pages_size, pages[new_index]))
                new_index++;
            
            // swap everything necessary to accommodate the rule
            for (size_t j = i; j > new_index; j--)
                pages[j] = pages[j-1];
            pages[new_index] = page;
        }
        free(following_pages);

        // Forbid subsequent pages that must be before this page
        for (size_t j = 0; j < order_rules_size; j++)
            if (order_rules[j].second == page)
                append_OrderRule_Vec(&active_order_rules, order_rules[j]);
    }

    // If no problems were found, the update is valid
    free(active_order_rules.arr);
    return was_valid;
}

// Determine if `arr` contains `n`
bool arr_contains(short *arr, size_t arr_size, short n)
{
    for (size_t i = 0; i < arr_size; i++)
        if (n == arr[i])
            return true;
    return false;
}

/// @brief Determine what pages must be after `n`
/// @param active_order_rules The array of order rules
/// @param active_order_rules_size The number of elements in `active_order_rules`
/// @param n The page to check
/// @param following_pages_size The size of the returned array
/// @return An array containing the pages that must follow `n`
short *pages_that_must_follow(OrderRule *active_order_rules, size_t active_order_rules_size, short n, size_t *following_pages_size)
{
    short_Vec following_pages = new_short_Vec();
    for (size_t i = 0; i < active_order_rules_size; i++)
        if (n == active_order_rules[i].first)
            append_short_Vec(&following_pages, active_order_rules[i].second);

    *following_pages_size = following_pages.len;
    return following_pages.arr;
}
