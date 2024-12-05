#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../vector_template.h"

typedef struct OrderRule
{
    short first;
    short second;
} OrderRule;

DEF_VEC(OrderRule)
DEF_VEC(short)
DEF_VEC(shortVec)
void delete_short_vec_vec(shortVecVec *vec)
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

int parse_input(OrderRuleVec *order_rules, shortVecVec *updates);
int sum_of_corrected_invalid_middles(OrderRule *order_rules, size_t order_rules_size, shortVecVec updates);
bool is_valid_update(OrderRule *order_rules, size_t order_rules_size, short *pages, size_t pages_size);
bool arr_contains(short *arr, size_t arr_size, short n);

int main(int argc, char const *argv[])
{
    OrderRuleVec order_rules;
    shortVecVec updates;

    // Try to parse input
    if (parse_input(&order_rules, &updates))
        return 1;
    
    printf("Sum of middle pages after corrections: %d\n", sum_of_corrected_invalid_middles(order_rules.arr, order_rules.len, updates));

    free(order_rules.arr);
    delete_short_vec_vec(&updates);
    return 0;
}

/// @brief Parse input.txt for this puzzle's input
/// @param order_rules out parameter for the order rules vector
/// @param updates out parameter of updates as a 2D vector. Each row is an update consisting of multiple pages
/// @return `0` if successful. Some nonzero number if unsuccessful (i.e. IO error)
int parse_input(OrderRuleVec *order_rules, shortVecVec *updates)
{
    FILE *f = fopen("sample_input.txt", "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *order_rules = newOrderRuleVec();
    *updates = newshortVecVec();

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
        appendOrderRule(order_rules, (OrderRule){first, second});
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
        shortVec update = {malloc(sizeof(short)), 1, 1};
        update.arr[0] = first_page;
        // Parse the rest of the update
        short page;
        while (fscanf(f, ",%2hd", &page) == 1)
            appendshort(&update, page);
        // Add the new update
        appendshortVec(updates, update);
    }

    fclose(f);
    return 0;
}

/// @brief Get the sum of the middle pages where the update is valid by is_valid_update()
/// @param order_rules The current order rules
/// @param order_rules_size The number of elements in `order_rules`
/// @param updates The list of updates
/// @return
int sum_of_corrected_invalid_middles(OrderRule *order_rules, size_t order_rules_size, shortVecVec updates)
{
    int sum = 0;
    for (size_t i = 0; i < updates.len; i++)
    {
        if (is_valid_update(order_rules, order_rules_size, updates.arr[i].arr, updates.arr[i].len))
        // Add the middle page to the sum
            sum += (int)(updates.arr[i].arr[updates.arr[i].len / 2]);
    }
    return sum;
}

/// @brief Determine if `pages` is in the correct order, given a set of order rules. If invalid, correct the order
/// @param order_rules The current order rules
/// @param order_rules_size The number of elements in `order_rules`
/// @param pages The pages in this update
/// @param pages_size The number of elements in `pages`
/// @return `true` if update was valid, `false` if update was invalid
bool is_valid_update(OrderRule *order_rules, size_t order_rules_size, short *pages, size_t pages_size)
{
    bool was_valid = true;
    // A page is forbidden if it is the first in a rule where the second already exists
    shortVec forbidden_pages = newshortVec();
    // Check each page
    for (size_t i = 0; i < pages_size; i++)
    {
        if (arr_contains(forbidden_pages.arr, forbidden_pages.len, pages[i]))
        {
            free(forbidden_pages.arr);
            return false;
        }

        // Forbid subsequent pages that must be before this page
        for (size_t j = 0; j < order_rules_size; j++)
            if (order_rules[j].second == pages[i])
                appendshort(&forbidden_pages, order_rules[j].first);
    }

    // If no problems were found, the update is valid
    free(forbidden_pages.arr);
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
