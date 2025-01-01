#include <stdio.h>
#include <stdlib.h>

// Singly linked list
typedef struct ListNode
{
    struct ListNode *next;
    long long val;
} ListNode;

typedef struct Trie
{
    // Array of children
    // Size is arbitrary, but should probably be a power of 2 for optimization.
    // 4,8,16 seem to all be fairly fast
    // More children per node means that fewer nodes are needed, but the nodes are bigger.
    struct Trie *children[8];
    long long val;
} Trie;

/* Memoization! That's what was missing. Thanks u/Sagitario2_5
   Global variable to cache
   This will be an array of tries, indexed by the number of blinks remaining
   This is my alternative to caching in a hashmap of (long long blinks, long long stone) -> count
*/
Trie **cache;

int parse_input(char *input_file, ListNode **head);
ListNode *append__list_Vec(ListNode *tail, long long val);
void insert_after(ListNode *parent, long long val);
int split_digits(long long n, long long *most_sig, long long *least_sig);
int log_10(long long n);
long long get_length_and_free(ListNode *head);
long long children_of_the_stone(long long stone, int blinks);
long long count_stones(ListNode *head, int blinks);
void print_list(ListNode *head);
void insert_trie(Trie *root, long long key, long long val);
void delete_trie(Trie *root);
long long get_trie(Trie *root, long long key);

int main(int argc, char *argv[])
{
    // The head of the list of stones
    ListNode *head = NULL;
    // The number of "blinks" to execute
    int blinks;
    // The file to read for input
    char *input_file;

    // Get input file and blink count from args
    if (argc >= 2)
        input_file = argv[1];
    else
        input_file = NULL;
    if (argc >= 3)
        blinks = atoi(argv[2]);
    else
    {
        printf("Blink count not recognized, using the default of 25.\n");
        blinks = 25;
    }

    if (parse_input(input_file, &head))
        return 1;

    // print_list(head);
    printf("Final stone count: %lld\n", count_stones(head, blinks));

    // Cleanup
    get_length_and_free(head);
    return 0;
}

/// @brief Parse the input file into
///
/// A linked list is used because that was how the brute force solution was implemented and I don't need to change this part
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param head Out: The head of the linked list of numbers that have been read
/// @return 0 if success, non-zero if failure
int parse_input(char *input_file, ListNode **head)
{
    // Open input.txt or stdin or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    // Dummy node to point to the future head
    // This feels like a dumb way to do this, but I can't think of a better way right now
    ListNode pre_head;
    ListNode *tail = &pre_head;
    long long val;
    while (fscanf(f, "%lld", &val) != EOF)
        tail = append__list_Vec(tail, val);
    tail->next = NULL;
    *head = pre_head.next;

    return 0;
}

/// @brief Append the value to tail
/// @param tail The tail of the list to append__Vec
/// @param val The value to append__Vec
/// @return The new tail
ListNode *append__list_Vec(ListNode *tail, long long val)
{
    tail = tail->next = malloc(sizeof(ListNode));
    tail->val = val;
    return tail;
}

/// @brief Insert a node containing `val` after this node
/// @param parent The parent node of the new node
/// @param val The value to insert
void insert_after(ListNode *parent, long long val)
{
    ListNode *node = malloc(sizeof(ListNode));
    node->next = parent->next;
    node->val = val;
    parent->next = node;
}

/// @brief For debugging: print the contents of a linked list
/// @param head The head of the list
void print_list(ListNode *head)
{
    while (head)
    {
        printf("%lld ", head->val);
        head = head->next;
    }
    printf("\n");
}

/// @brief Get the length of a linked list, freeing all elements in the process
/// @param head The head of the list
/// @return The length of the list starting at head
long long get_length_and_free(ListNode *head)
{
    long long count = 0LL;
    ListNode *next;
    while (head)
    {
        count++;
        next = head->next;
        free(head);
        head = next;
    }
    return count;
}

/// @brief Count the total number of stones after applying `children_of_the_stone(...)` to each stone `blinks` times
/// @param head The list of stones in the base case
/// @param blinks The number of blinks to perform
/// @return The total number of stones after `blinks` operations to each stone
long long count_stones(ListNode *head, int blinks)
{
    // Initialize cache array
    cache = malloc(blinks * sizeof(cache[0]));
    for (int i = 0; i < blinks; i++)
    {
        cache[i] = calloc(1, sizeof(*cache[0]));
        // Indicate that stone of 0 is unknown
        cache[i]->val = -1;
    }

    long long total_stones = 0;
    ListNode *next;
    while (head)
    {
        // Find the next node before applying the operation to prevent issues when the operation inserts a stone after this node
        next = head->next;
        total_stones += children_of_the_stone(head->val, blinks);
        // Finally, set the head node to the next node for the next operation
        head = next;
    }
    // Free cache
    for (int i = 0; i < blinks; i++)
        delete_trie(cache[i]);
    free(cache);
    return total_stones;
}

/// @brief Count the number of stones that will exist after applying `blinks` operations to this stone
///
/// Terrible as a function name, but would be great as a band name
/// @param stone The number on the stone
/// @param blinks The number of iterations
/// @return The number of stones that will replace this stone
long long children_of_the_stone(long long stone, int blinks)
{
    // If there are no more blinks, this is the stone
    if (!blinks)
    {
        // for (int i = 0; i < 5 - blinks; i++)
        //     printf(" ");
        // printf("Stone %lld, blinks %d: %lld\n", stone, blinks, 1LL);
        return 1;
    }

    // There is now one less blink left
    blinks--;

    // Check for a memoized count
    long long child_count = get_trie(cache[blinks], stone);
    if (child_count != -1)
        return child_count;

    long long left_stone, right_stone;
    if (!stone)
        // If the stone is engraved with the number 0, it is replaced by a stone engraved with the number 1.
        child_count = children_of_the_stone(1, blinks);
    else if (split_digits(stone, &left_stone, &right_stone))
        // If the stone is engraved with a number that has an even number of digits, it is replaced by two stones.
        // The left half of the digits are engraved on the new left stone, and the right half of the digits are engraved on the new right stone.
        child_count = children_of_the_stone(left_stone, blinks) + children_of_the_stone(right_stone, blinks);
    else
        // If none of the other rules apply, the stone is replaced by a new stone; the old stone's number multiplied by 2024 is engraved on the new stone.
        child_count = children_of_the_stone(2024 * stone, blinks);

    // Memoize
    insert_trie(cache[blinks], stone, child_count);
    // for (int i = 0; i < 5 - (blinks + 1); i++)
    //     printf(" ");
    // printf("Stone %lld, blinks %d: %lld\n", stone, blinks + 1, child_count);
    return child_count;
}

/// @brief If `n` has an even number of digits, split it into the most significant and least significant halves
/// @param n The number to try to split
/// @param most_sig The most significant/left half of the digits in `n`
/// @param least_sig The least significant/right half of the digits in `n`
/// @return 1 if the split happened, 0 if not
int split_digits(long long n, long long *most_sig, long long *least_sig)
{
    int log = log_10(n);
    if (log % 2)
    {
        // Log is odd, therefore the number of digits is even (log is digit count - 1)
        // Get half of the number of digits
        log = (log + 1) / 2;
        long long power_of_10 = 1;
        for (int i = 0; i < log; i++)
            power_of_10 *= 10;
        *most_sig = n / power_of_10;
        *least_sig = n % power_of_10;
        return 1;
    }
    else
    {
        // Number of digits is odd
        return 0;
    }
}

// Base 10 logarithm of n
int log_10(long long n)
{
    // n must be positive
    if (n <= 0)
    {
        fprintf(stderr, "log_10(%lld) is not a number\n", n);
        return -1;
    }

    int log = 0;
    while (n >= 10)
    {
        n /= 10;
        log++;
    }
    return log;
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
