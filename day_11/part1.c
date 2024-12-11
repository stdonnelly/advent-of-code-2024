#include <stdio.h>
#include <stdlib.h>

// Singly linked list
typedef struct ListNode
{
    struct ListNode *next;
    long long val;
} ListNode;

int parse_input(char *input_file, ListNode **head);
ListNode *append(ListNode *tail, long long val);
void insert_after(ListNode *parent, long long val);
int split_digits(long long n, long long *most_sig, long long *least_sig);
int log_10(long long n);
long long get_length_and_free(ListNode *head);
void apply_operation(ListNode *node);
void apply_operations(ListNode *head);
void print_list(ListNode *head);

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

    // Apply operations `blinks` times
    for (int i = 0; i < blinks; i++)
        apply_operations(head);
    
    // print_list(head);
    printf("Final stone count: %lld\n", get_length_and_free(head));

    return 0;
}

/// @brief Parse the input file into
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
        tail = append(tail, val);
    tail->next = NULL;
    *head = pre_head.next;

    return 0;
}

/// @brief Append the value to tail
/// @param tail The tail of the list to append
/// @param val The value to append
/// @return The new tail
ListNode *append(ListNode *tail, long long val)
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

/// @brief Apply `apply_operation(...)` to all elements in the given list
/// @param head The head of the list to apply operations to
void apply_operations(ListNode *head)
{
    ListNode *next;
    while (head)
    {
        // Find the next node before applying the operation to prevent issues when the operation inserts a stone after this node
        next = head->next;
        apply_operation(head);
        // Finally, set the head node to the next node for the next operation
        head = next;
    }
}

/// @brief Apply the appropriate operation to the stone represented by `node`
/// @param node The node representing this stone
void apply_operation(ListNode *node)
{
    long long left_stone, right_stone;
    if (!(node->val))
        // If the stone is engraved with the number 0, it is replaced by a stone engraved with the number 1.
        node->val = 1;
    else if (split_digits(node->val, &left_stone, &right_stone))
    {
        // If the stone is engraved with a number that has an even number of digits, it is replaced by two stones.
        // The left half of the digits are engraved on the new left stone, and the right half of the digits are engraved on the new right stone.
        node->val = left_stone;
        insert_after(node, right_stone);
    }
    else
        // If none of the other rules apply, the stone is replaced by a new stone; the old stone's number multiplied by 2024 is engraved on the new stone.
        node->val *= 2024LL;
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
