#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"

typedef enum OperationType
{
    AND,
    OR,
    XOR,
} OperationType;

typedef struct Operation
{
    OperationType type;
    // These should only store 3 chars each, but I'm using char[4] for alignment
    char lhs[4], rhs[4], result[4];
} Operation;

// List node of operation
typedef struct OperationListNode
{
    struct OperationListNode *next;
    Operation op;
} OperationListNode;

DEF_VEC(Operation)

// Part 1
int parse_input(char *input_file, long long *x, long long *y, OperationVec *operations);
long long perform_all_operations(long long x, long long y, Operation *operations, size_t operations_size);
OperationListNode *to_linked_list(Operation *arr, size_t size, OperationListNode **tail);

// Part 2
void print_z_equations(Operation *operations, size_t operations_size);
int print_equation(Operation *operations, size_t operations_size, char *result);
Operation *find_operation(Operation *operations, size_t operations_size, char *target_result);
int are_operations_equivalent(Operation *operations, size_t operations_size, char *a, char *b);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    long long x, y;
    OperationVec operations;

    if (parse_input(input_file, &x, &y, &operations))
        return 1;

    long long z = perform_all_operations(x, y, operations.arr, operations.len);

    printf("z: %lld\n", z);

    print_z_equations(operations.arr, operations.len);

    free(operations.arr);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param x Out: The x value
/// @param y Out: The y value
/// @param operations Out: A vector of operations to perform
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, long long *x, long long *y, OperationVec *operations)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    // Parse x and y initial state
    {
        *x = 0;
        *y = 0;
        int variable_name = getc(f);
        int index;
        int value;
        long long *variable;
        while (variable_name != '\n' && variable_name != EOF)
        {
            // Find variable by name
            if (variable_name == 'x')
                variable = x;
            else if (variable_name == 'y')
                variable = y;
            else
            {
                fprintf(stderr, "Unexpected variable: '%c'\n", (char)variable_name);
                fclose(f);
                return 1;
            }

            fscanf(f, "%2d: %1d", &index, &value);

            // Put the value into the variable
            if (value)
                *variable |= 1LL << index;

            // getc(...) to check for double newline
            getc(f);
            variable_name = getc(f);
        }
    }

    // Parse operations
    {
        *operations = newOperationVec();
        Operation op;
        char operation_type[4];

        while (fscanf(f, "%3s %3s %3s -> %3s", op.lhs, operation_type, op.rhs, op.result) == 4)
        {
            // Parse operation type
            if (!strcmp("AND", operation_type))
                op.type = AND;
            else if (!strcmp("OR", operation_type))
                op.type = OR;
            else if (!strcmp("XOR", operation_type))
                op.type = XOR;
            else
            {
                fprintf(stderr, "Unknown operation type '%4s'\n", operation_type);
                free(operations->arr);
                return 1;
            }
            appendOperation(operations, op);
        }
    }

    fclose(f);
    return 0;
}

/// @brief Convert operation array to linked list
/// @param arr The array of operations
/// @param size Tne number of elements in `operations`
/// @param tail Out: The tail node of the linked list
/// @return The head node of the linked list
OperationListNode *to_linked_list(Operation *arr, size_t size, OperationListNode **tail)
{
    // Return NULL,NULL if there is no array
    if (size <= 0)
    {
        *tail = NULL;
        return NULL;
    }

    // Put the head
    OperationListNode *head = malloc(sizeof(*head));
    head->op = arr[0];

    // Put the rest of the nodes
    OperationListNode *node = head;
    for (size_t i = 1; i < size; i++)
    {
        node = node->next = malloc(sizeof(*node));
        node->op = arr[i];
    }
    // Finish the tail node
    node->next = NULL;
    *tail = node;

    return head;
}

#define INDEX_BY_STR(arr, str) (arr)[(((str)[0] - 'a') * 26 * 26) + (((str)[1] - 'a') * 26) + ((str[2] - 'a'))]

/// @brief Perform all operations in `operations` and find the final value of `z`
/// @param x The initial state of x
/// @param y The initial state of y
/// @param operations The array of operations to do
/// @param operations_size The number of elements in `operations`
/// @return The final value of `z`
long long perform_all_operations(long long x, long long y, Operation *operations, size_t operations_size)
{
    long long z = 0;

    // Linked list
    OperationListNode *tail;
    OperationListNode *head = to_linked_list(operations, operations_size, &tail);
    char other_variables[26 * 26 * 26];
    // -1 means not set yet
    memset(other_variables, -1, 26 * 26 * 26);

    // Loop over the linked list
    while (head)
    {
        OperationListNode *next = head->next;
        char lhs, rhs, result;
        Operation op = head->op;

        // Get lhs
        if (op.lhs[0] == 'x')
            lhs = (x >> atoi(op.lhs + 1)) & 1;
        else if (op.lhs[0] == 'y')
            lhs = (y >> atoi(op.lhs + 1)) & 1;
        else
            lhs = INDEX_BY_STR(other_variables, op.lhs);

        // Check if the lhs has been set yet
        if (lhs == (char)-1)
        {
            // If not yet defined, put this on the end of the list
            head->next = NULL;
            tail = tail->next = head;
            head = next;
            continue;
        }

        // Get rhs
        if (op.rhs[0] == 'x')
            rhs = (x >> atoi(op.rhs + 1)) & 1;
        else if (op.rhs[0] == 'y')
            rhs = (y >> atoi(op.rhs + 1)) & 1;
        else
            rhs = INDEX_BY_STR(other_variables, op.rhs);

        // Check if the rhs has been set yet
        if (rhs == (char)-1)
        {
            // If not yet defined, put this on the end of the list
            head->next = NULL;
            tail = tail->next = head;
            head = next;
            continue;
        }

        switch (op.type)
        {
        case AND:
            result = lhs && rhs;
            break;
        case OR:
            result = lhs || rhs;
            break;
        case XOR:
            result = lhs ^ rhs;
            break;
        }

        // Store result
        if (op.result[0] == 'z')
            z |= (long long)result << atoi(op.result + 1);
        else
            INDEX_BY_STR(other_variables, op.result) = result;

        free(head);
        head = next;
    }

    return z;
}

/// @brief Print each equation that results in a z bit
/// @param operations The array of operations to do
/// @param operations_size The number of elements in `operations`
void print_z_equations(Operation *operations, size_t operations_size)
{
    char z_identifier[4];
    // Infinite loop, will break when we can't find an additional z
    for (int i = 0;; i++)
    {
        // Get the identifier
        snprintf(z_identifier, sizeof(z_identifier), "z%02d", i);
        printf("%s: ", z_identifier);

        if (!print_equation(operations, operations_size, z_identifier))
            break;
        printf("\n");
    }
    printf("\n");
}

/// @brief Recursively print an equation resulting in `result` to stdout
/// @param operations The array of operations to do
/// @param operations_size The number of elements in `operations`
/// @param result The result to use
/// @return 1 if successful, 0 if the result was not found as a valid result
int print_equation(Operation *operations, size_t operations_size, char *result)
{
    for (size_t i = 0; i < operations_size; i++)
    {
        Operation op = operations[i];
        // Check the result identifier
        // If this is not the correct one, skip this step
        if (memcmp(op.result, result, sizeof(op.result)))
            continue;

        // Recursively print lhs
        if (op.lhs[0] == 'x' || op.lhs[0] == 'y')
        {
            printf("%s", op.lhs);
        }
        else
        {
            printf("(");
            print_equation(operations, operations_size, op.lhs);
            printf(")");
        }

        // Print operation type
        switch (op.type)
        {
        case AND:
            printf(" AND ");
            break;
        case OR:
            printf(" OR ");
            break;
        case XOR:
            printf(" XOR ");
            break;
        default:
            fprintf(stderr, "\nUnexpected operation type: %d\n", op.type);
            return 0;
        }

        // Recursively print rhs
        if (op.rhs[0] == 'x' || op.rhs[0] == 'y')
        {
            printf("%s", op.rhs);
        }
        else
        {
            printf("(");
            print_equation(operations, operations_size, op.rhs);
            printf(")");
        }

        return 1;
    }

    return 0;
}

// /// @brief Find the operation that produces result `target_result`
// /// @param operations The list of operations to search in
// /// @param operations_size The size of `operations`
// /// @param target_result The result to search for
// /// @return A pointer to the operation if found, NULL otherwise
// Operation *find_operation(Operation *operations, size_t operations_size, char *target_result)
// {
//     for (size_t i = 0; i < operations_size; i++)
//     {
//         if (!memcmp(operations[i].result, target_result, sizeof(operations[i].result)))
//             return operations + i;
//     }
//     return NULL;
// }

// /// @brief Determine if the operations needed to find the given variables are equivalent.
// ///
// /// This can include swapping the operands, since all operations are commutative.
// /// @param operations The list of operations to find intermediate operations in
// /// @param operations_size The size of `operations`
// /// @param a One variable
// /// @param b The other variable
// /// @return 1 if the operations are equivalent, 0 if not
// int are_operations_equivalent(Operation *operations, size_t operations_size, char *a, char *b)
// {
//     // If they are just the same variable
//     if (!memcmp())
// }
