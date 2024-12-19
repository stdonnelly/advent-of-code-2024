#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "../vector_template.h"
#include "../hash_map.h"

#define SET_WHITE 37
#define SET_GREEN 32
#define SET_RED 31

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

typedef enum TowelColor
{
    WHITE,
    BLUE,
    BLACK,
    RED,
    GREEN,
    // Dummy variant to hold the number of towel colors
    TOWEL_COLOR_COUNT,
} TowelColor;

typedef struct Trie
{
    struct Trie *children[TOWEL_COLOR_COUNT];
    bool is_tail;
} Trie;

// IO
int parse_input(char *input_file, stringVec *available_towels, stringVec *patterns);
void print_trie(Trie *root, charVec *prefix);

// Trie
void push_trie(Trie *root, char *word);
void delete_trie(Trie *root);
TowelColor get_color(char color_char);

// Problem steps
int get_valid_patterns(char **available_towels, size_t available_towels_size, char **patterns, size_t patterns_size);
bool count_valid(Trie *available_towels, char *pattern);

HashMap valid_cache;

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    stringVec available_towels;
    stringVec patterns;
    if (parse_input(input_file, &available_towels, &patterns))
        return 1;

    printf("Number of possible designs: %d\n", get_valid_patterns(available_towels.arr, available_towels.len, patterns.arr, patterns.len));

    delete_string_vec(&available_towels);
    delete_string_vec(&patterns);
    return 0;
}

/// @brief Parse input.txt
/// @param input_file The file to read
/// @param available_towels Out: The list of available towels
/// @param patterns Out: The list of patterns to make
/// @return 0 if success. 1 if failure.
int parse_input(char *input_file, stringVec *available_towels, stringVec *patterns)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        fclose(f);
        return 1;
    }

    // Parse available towels
    *available_towels = newstringVec();

    int ch;
    charVec towel = newcharVec();
    while ((ch = getc(f)) != EOF)
    {
        if (isalpha(ch))
            // Alphabetic
            appendchar(&towel, ch);
        else if (ch == ',')
        {

            // Add terminator for the string and put it on the string vector
            appendchar(&towel, '\0');
            appendstring(available_towels, towel.arr);
            towel = newcharVec();
        }
        else if (ch == '\n')
        {
            // Append the last towel and stop checking for towels
            appendchar(&towel, '\0');
            appendstring(available_towels, towel.arr);
            break;
        }
    }

    // Avoid parsing the next newline as a pattern, but check to make sure it's not EOF
    ch = getc(f);
    if (ch == EOF)
    {
        // If EOF was reached earlier than expected
        delete_string_vec(available_towels);
        fprintf(stderr, "Reached EOF before any patterns were read\n");
        fclose(f);
        return 1;
    }

    // Parse patterns
    *patterns = newstringVec();
    charVec pattern = newcharVec();
    while ((ch = getc(f)) != EOF)
    {
        if (isalpha(ch))
            // Alphabetic
            appendchar(&pattern, ch);
        else if (ch == '\n')
        {
            // Append the last towel and stop checking for towels
            appendchar(&pattern, '\0');
            appendstring(patterns, pattern.arr);
            pattern = newcharVec();
        }
    }

    // If relevant, append the last row. Otherwise, just free the row
    if (pattern.len > 0)
    {
        appendchar(&pattern, '\0');
        appendstring(patterns, pattern.arr);
    }
    else
        free(pattern.arr);

    fclose(f);
    return 0;
}

// For debugging: Print all elements of `root`
void print_trie(Trie *root, charVec *prefix)
{
    if (root->is_tail)
        printf("%s\n", prefix->arr);

    // Save the current last character index so we can change it later
    size_t this_index = prefix->len;
    // Append NULL to push the next character to after this one
    appendchar(prefix, '\0');
    // Append another NULL. This one is for a string terminator
    appendchar(prefix, '\0');
    // Reset the length so future calls will overwrite the terminator we just put
    prefix->len = this_index + 1;

    for (TowelColor i = 0; i < TOWEL_COLOR_COUNT; i++)
    {
        Trie *next = root->children[i];
        // Ignore when there is no child
        if (!next)
            continue;

        // Parse the towel color into a character
        char ch;
        switch (i)
        {
        case WHITE:
            ch = 'w';
            break;
        case BLUE:
            ch = 'u';
            break;
        case BLACK:
            ch = 'b';
            break;
        case RED:
            ch = 'r';
            break;
        case GREEN:
            ch = 'g';
            break;
        default:
            // Panic if the color is unknown
            fprintf(stderr, "Unknown color: '%d'\n", i);
            exit(1);
        }

        // Print the children with this prefix
        prefix->arr[this_index] = ch;
        print_trie(next, prefix);
    }

    // Clear this character
    prefix->arr[this_index] = '\0';
    prefix->len--;
}

/// @brief Push `word` into a trie
/// @param root The trie
/// @param word The word to add
void push_trie(Trie *root, char *word)
{
    if (!*word)
    {
        // Set this as a tail and stop looking
        root->is_tail = true;
        return;
    }

    TowelColor color = get_color(*word);

    // If the correct child has not already been allocated, allocate it
    if (!root->children[color])
        root->children[color] = calloc(1, sizeof(*(root->children[0])));

    // Recursively push onto the trie the next characters
    push_trie(root->children[color], word + 1);
}

/// @brief Recursively delete a Trie
/// @param root The root node of the trie to delete
void delete_trie(Trie *root)
{
    // Recursively free children
    for (int i = 0; i < sizeof(root->children) / sizeof(root->children[0]); i++)
        if (root->children[i])
            delete_trie(root->children[i]);
    // Free self
    free(root);
}

// Parse a color into a TowelColor variant
TowelColor get_color(char color_char)
{
    TowelColor color;
    switch (color_char)
    {
    case 'w':
        color = WHITE;
        break;
    case 'u':
        color = BLUE;
        break;
    case 'b':
        color = BLACK;
        break;
    case 'r':
        color = RED;
        break;
    case 'g':
        color = GREEN;
        break;
    default:
        // Panic if the color is unknown
        fprintf(stderr, "Unknown color: '%c'\n", color_char);
        exit(1);
    }
    return color;
}

/// @brief Get the number of patterns in `patterns` that can be constructed with `available_towels`
/// @param available_towels The available towels
/// @param available_towels_size The number of elements in `available_towels`
/// @param patterns The patterns to try
/// @param patterns_size The number of elements in `patterns`
/// @return The number of possible patterns
int get_valid_patterns(char **available_towels, size_t available_towels_size, char **patterns, size_t patterns_size)
{
    int valid_pattern_count = 0;
    int text_color;

    // Construct the trie
    Trie *towels_trie = calloc(1, sizeof(*towels_trie));
    for (size_t i = 0; i < available_towels_size; i++)
        push_trie(towels_trie, available_towels[i]);

    // Initialize valid cache
    valid_cache = new_HashMap();

    // Debugging: Make sure trie is loaded correctly
    // charVec prefix_vector = newcharVec();
    // appendchar(&prefix_vector, '\0');
    // prefix_vector.len = 0;
    // print_trie(towels_trie, &prefix_vector);
    // free(prefix_vector.arr);
    // delete_trie(towels_trie);
    // return -1;

    for (size_t i = 0; i < patterns_size; i++)
    {
        if (count_valid(towels_trie, patterns[i]))
        {
            valid_pattern_count++;
            text_color = SET_GREEN;
        }
        else
            text_color = SET_RED;

        // Print pattern in red or green
        printf("\e[%dm%s\n", text_color, patterns[i]);
    }

    // Reset text color to white and print newline
    printf("\e[%dm\n", SET_WHITE);

    // Cleanup trie
    delete_trie(towels_trie);
    delete_HashMap(&valid_cache);
    return valid_pattern_count;
}

/// @brief Check if `pattern` can be constructed using towels from `available_towels`
/// @param available_towels The available towels
/// @param available_towels_size The number of elements in `available_towels`
/// @param pattern The pattern to try
/// @return true if valid, false if invalid
bool count_valid(Trie *available_towels, char *pattern)
{
    // Check cache
    int cached = get_map(&valid_cache, pattern);
    if (cached == 0)
        return false;
    else if (cached == 1)
        return true;
    // Base case: pattern == "", which is possible
    if (!*pattern)
        return true;

    char *pattern_temp = pattern;
    char ch = *pattern_temp++;
    TowelColor color = get_color(ch);
    Trie *this_towel = available_towels->children[color];

    while (this_towel)
    {
        // If the pattern has not ended yet, and we can use a new towel, check that
        if (this_towel->is_tail && count_valid(available_towels, pattern_temp))
        {
            put_map(&valid_cache, pattern, 1);
            return true;
        }

        ch = *pattern_temp++;
        // If the pattern is ended, check if this is a tail in the available towels
        if (!ch)
        {
            put_map(&valid_cache, pattern, 0);
            return false;
        }
        color = get_color(ch);
        this_towel = this_towel->children[color];
    }

    // No valid pattern was found
    put_map(&valid_cache, pattern, 0);
    return false;
}
