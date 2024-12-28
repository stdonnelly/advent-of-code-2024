#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"

#define KEY_WIDTH 5
#define KEY_HEIGHT 7

typedef struct KeyLock
{
    char pins[KEY_WIDTH];
} KeyLock;

DEF_VEC(KeyLock)

int parse_input(char *input_file, KeyLockVec *keys, KeyLockVec *locks);
long long count_unique_valid_pairs(KeyLock *keys, size_t keys_size, KeyLock *locks, size_t locks_size);
int is_valid_pair(KeyLock key, KeyLock lock);
void print_key_lock(KeyLock key_lock);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    KeyLockVec keys, locks;

    if (parse_input(input_file, &keys, &locks))
        return 1;

    printf("Keys:\n");
    for (size_t i = 0; i < keys.len; i++)
    {
        print_key_lock(keys.arr[i]);
        printf("\n");
    }
    printf("\nLocks:\n");
    for (size_t i = 0; i < locks.len; i++)
    {

        print_key_lock(locks.arr[i]);
        printf("\n");
    }

    long long unique_valid_pair_count = count_unique_valid_pairs(keys.arr, keys.len, locks.arr, locks.len);

    printf("Unique valid pairs: %lld\n", unique_valid_pair_count);

    free(keys.arr);
    free(locks.arr);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param keys Out: A vector if keys
/// @param locks Out: A vector of locks
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, KeyLockVec *keys, KeyLockVec *locks)
{
    int is_error = 0;
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *keys = newKeyLockVec();
    *locks = newKeyLockVec();

    char input_buffer[KEY_WIDTH + 2];

    while (fgets(input_buffer, sizeof(input_buffer), f))
    {
        KeyLock this_key_lock = {{0}};
        int is_key;

        // Determine if key or lock
        if (!strcmp(input_buffer, "#####\n"))
            // The locks are schematics that have the top row filled (#)
            is_key = 0;
        else if (!strcmp(input_buffer, ".....\n"))
            // the keys have the top row empty
            is_key = 1;

        // Parse key/lock
        for (int i = 0; i < KEY_HEIGHT - 2; i++)
        {
            // Get the next line
            if (!fgets(input_buffer, sizeof(input_buffer), f))
            {
                fprintf(stderr, "Incorrectly formed key/lock found: EOF found too early.\n");
                is_error = 1;
                goto CLOSE_FILE;
            }

            // Parse the line
            for (int j = 0; (j < KEY_WIDTH) && input_buffer[j]; j++)
            {
                // If a filled space was found, increment the height of the key or lock
                if (input_buffer[j] == '#')
                    this_key_lock.pins[j]++;
                // If the space was not '#' or '.', print an error
                else if (input_buffer[j] != '.')
                {
                    fprintf(stderr, "Incorrectly formed key/lock found: Unexpected character '%c'\n", input_buffer[j]);
                    is_error = 1;
                    goto CLOSE_FILE;
                }
            }
        }

        // Get the last line of the key/lock
        if (!fgets(input_buffer, sizeof(input_buffer), f))
        {
            fprintf(stderr, "Incorrectly formed key/lock found: EOF found before end line.\n");
            is_error = 1;
            goto CLOSE_FILE;
        }

        // Assert that the last line is correct (opposite of start line)
        if (is_key)
        {
            if (strcmp(input_buffer, "#####\n"))
            {
                fprintf(stderr, "Unexpected end to key: '%s'\n", input_buffer);
                is_error = 1;
                goto CLOSE_FILE;
            }
        }
        else
        {
            if (strcmp(input_buffer, ".....\n"))
            {
                fprintf(stderr, "Unexpected end to lock: '%s'\n", input_buffer);
                is_error = 1;
                goto CLOSE_FILE;
            }
        }

        // Finally, discard the newline between keys/locks
        fgets(input_buffer, sizeof(input_buffer), f);

        // Put the key/lock into the appropriate vector
        if (is_key)
            appendKeyLock(keys, this_key_lock);
        else
            appendKeyLock(locks, this_key_lock);
    }

CLOSE_FILE:
    fclose(f);
    if (is_error)
    {
        free(keys->arr);
        free(locks->arr);
    }
    return is_error;
}

void print_key_lock(KeyLock key_lock)
{
    printf("%hhd", key_lock.pins[0]);
    for (int i = 1; i < KEY_WIDTH; i++)
        printf(",%hhd", key_lock.pins[i]);
}

/// @brief Count the unique non-overlapping pairs of keys and locks
/// @param keys The array of keys
/// @param keys_size The number of elements in `keys`
/// @param locks The array of locks
/// @param locks_size The number of elements in `locks`
/// @return The number of unique valid pairs of keys and locks
long long count_unique_valid_pairs(KeyLock *keys, size_t keys_size, KeyLock *locks, size_t locks_size)
{
    long long valid_pairs = 0;

    for (size_t i = 0; i < keys_size; i++)
        for (size_t j = 0; j < locks_size; j++)
            if (is_valid_pair(keys[i], locks[j]))
                valid_pairs++;

    return valid_pairs;
}

/// @brief Check if a pair `key` and `lock` have no overlaps
/// @param key The key
/// @param lock The lock
/// @return 1 if valid, 0 if invalid
int is_valid_pair(KeyLock key, KeyLock lock)
{
    for (int i = 0; i < KEY_WIDTH; i++)
        if (key.pins[i] + lock.pins[i] > KEY_HEIGHT - 2)
            return 0;
    return 1;
}
