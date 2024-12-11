#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

typedef struct Trie
{
    // Array of children
    struct Trie *children[2];
    long long val;
} Trie;

void insert_trie(Trie *root, long long key, long long val);
void delete_trie(Trie *root);
long long get_trie(Trie *root, long long key);

int main(int argc, char const *argv[])
{
    Trie *root = calloc(1, sizeof(Trie));
    // A -1 will serve as unknown
    root->val = -1;
    long long key;
    long long val;

    for (int i = 1; i + 1 < argc; i += 2)
    {
        key = strtol(argv[i], NULL, 10);
        if (errno)
        {
            perror("Error reading argument key");
            goto CLEANUP;
        }
        val = strtol(argv[i + 1], NULL, 10);
        if (errno)
        {
            perror("Error reading argument val");
            goto CLEANUP;
        }

        insert_trie(root, key, val);
    }

    for (int i = 1; i + 1 < argc; i += 2)
    {
        key = strtol(argv[i], NULL, 10);
        if (errno)
        {
            perror("Error reading argument key");
            goto CLEANUP;
        }

        printf("%lld: %lld\n", key, get_trie(root, key));
    }

CLEANUP:
    delete_trie(root);
    return 0;
}

/// @brief Insert `val` into the trie under `key`
/// @param root The trie to insert into
/// @param key The key
/// @param val The value to insert
void insert_trie(Trie *root, long long key, long long val)
{
    if (!key)
    {
        // Once we have found the appropriate element, put the key into it
        root->val = val;
        return;
    }

    // If we have not found the appropriate element, keep going deeper
    long long subkey = key % (sizeof(root->children) / sizeof(root->children[0]));
    key /= sizeof(root->children) / sizeof(root->children[0]);

    if (!root->children[subkey])
    {
        // Initialize the child if it doesn't already exist
        root->children[subkey] = calloc(1, sizeof(*(root->children[0])));
        root->children[subkey]->val = -1;
    }

    insert_trie(root->children[subkey], key, val);
}

/// Free everything in the trie, including root
void delete_trie(Trie *root)
{
    for (size_t i = 0; i < sizeof(root->children) / sizeof(root->children[0]); i++)
        if (root->children[i])
            delete_trie(root->children[i]);
    free(root);
}

/// @brief Get the value at `key` if it exists
/// @param root The trie to search in
/// @param key The key to search for
/// @return The value contained at `key` on trie if it exists, -1 otherwise
long long get_trie(Trie *root, long long key)
{
    // Guard clause for not found or root is uninitialized
    if (!root)
        return -1;

    // This is the element
    if (!key)
        return root->val;

    // If we have not found the appropriate element, keep going deeper
    long long subkey = key % (sizeof(root->children) / sizeof(root->children[0]));
    key /= sizeof(root->children) / sizeof(root->children[0]);
    return get_trie(root->children[subkey], key);
}
