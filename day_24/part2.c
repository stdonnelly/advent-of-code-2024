#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vector_template.h"
#include "../hash_map.h"

#define ADDER_REF_NAME "45-bit_adder.txt"

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

typedef struct OperationTreeNode
{
    struct OperationTreeNode *lhs, *rhs;
    OperationType type;
    char result[4];
} OperationTreeNode;

// Part 1
int parse_input(char *input_file, long long *x, long long *y, OperationVec *operations);
long long perform_all_operations(long long x, long long y, Operation *operations, size_t operations_size);
OperationListNode *to_linked_list(Operation *arr, size_t size, OperationListNode **tail);

// Part 2
void print_z_equations(Operation *operations, size_t operations_size);
int print_equation(Operation *operations, size_t operations_size, char *result);
HashMap get_operation_tree(Operation *operations, size_t operations_size);
void print_operation_tree(OperationTreeNode *root);
void print_all_incorrect_operations(Operation *operations, size_t operations_size);
int are_operation_trees_equal(OperationTreeNode *expected, OperationTreeNode *actual);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    long long x, y;
    OperationVec operations;

    if (parse_input(input_file, &x, &y, &operations))
        return 1;

    long long z = perform_all_operations(x, y, operations.arr, operations.len);

    printf("z: %lld\n", z);

    print_all_incorrect_operations(operations.arr, operations.len);

    HashMap map = get_operation_tree(operations.arr, operations.len);
    delete_HashMap_and_values(&map);

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
    HashMap operations_map = get_operation_tree(operations, operations_size);
    char z_identifier[4];
    // Infinite loop, will break when we can't find an additional z
    // printf("{\n");
    for (int i = 0;; i++)
    {
// Get the identifier
// Ignore possible truncation of '%02d' because i should always be at most two digits
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
        snprintf(z_identifier, sizeof(z_identifier), "z%02d", i);
#pragma GCC diagnostic pop

        OperationTreeNode *operation_node = get_map_as_ptr(&operations_map, z_identifier);
        if (operation_node)
        {
            printf("\"%s\": ", z_identifier);
            print_operation_tree(operation_node);
            // printf(",\n");
            printf("\n");
        }
        else
            break;
    }
    // printf("}\n");

    delete_HashMap_and_values(&operations_map);
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

/// @brief Convert an array of operations into a hashmap of connected OperationTreeNodes
/// @param operations The array of operations to do
/// @param operations_size The number of elements in `operations`
/// @return A HashMap containing each operation tree
HashMap get_operation_tree(Operation *operations, size_t operations_size)
{
    HashMap map = new_HashMap();

    // Linked list
    OperationListNode *tail;
    OperationListNode *head = to_linked_list(operations, operations_size, &tail);

    // Loop over the linked list
    while (head)
    {
        OperationListNode *next = head->next;
        // char lhs, rhs, result;
        Operation op = head->op;

        OperationTreeNode *lhs = get_map_as_ptr(&map, op.lhs);
        if (!lhs)
        {
            // x.. or y.. not being found is expected, other variables should be found at some point
            if (op.lhs[0] == 'x' || op.lhs[0] == 'y')
            {
                // Make a new one
                lhs = calloc(1, sizeof(*lhs));
                memcpy(lhs->result, op.lhs, sizeof(op.lhs));

                put_map_as_ptr(&map, op.lhs, lhs);
            }
            else
            {
                // If this is an intermediate variable, wait until after it's put into the map
                head->next = NULL;
                tail = tail->next = head;
                head = next;
                continue;
            }
        }

        // Do the same thing for rhs
        OperationTreeNode *rhs = get_map_as_ptr(&map, op.rhs);
        if (!rhs)
        {
            // x.. or y.. not being found is expected, other variables should be found at some point
            if (op.rhs[0] == 'x' || op.rhs[0] == 'y')
            {
                // Make a new one
                rhs = calloc(1, sizeof(*rhs));
                memcpy(rhs->result, op.rhs, sizeof(op.rhs));

                put_map_as_ptr(&map, op.rhs, rhs);
            }
            else
            {
                // If this is an intermediate variable, wait until after it's put into the map
                head->next = NULL;
                tail = tail->next = head;
                head = next;
                continue;
            }
        }

        // If neither of the above statements caused this loop to continue, create a new operation tree node
        OperationTreeNode *this_op = malloc(sizeof(*this_op));
        this_op->lhs = lhs;
        this_op->rhs = rhs;
        memcpy(this_op->result, op.result, sizeof(op.result));
        this_op->type = op.type;

        // Store result
        put_map_as_ptr(&map, op.result, this_op);

        free(head);
        head = next;
    }

    return map;
}

/// @brief Recursively print the tree of operations to stdout
/// @param root The root of the operations tree
void print_operation_tree(OperationTreeNode *root)
{
    if (root->result[0] == 'x' || root->result[0] == 'y')
    {
        // If the result is one of the starting variables, just print it
        // printf("\"%s\"", root->result);
        printf("%s", root->result);
    }
    else
    {
        char *type;
        switch (root->type)
        {
        case AND:
            type = "AND";
            break;
        case OR:
            type = "OR";
            break;
        case XOR:
            type = "XOR";
            break;
        }
        // If the root is an intermediate variable, recursively print it
        // printf("{\"name\":\"%s\",\"lhs\":", root->result);
        printf("(");
        print_operation_tree(root->lhs);
        // printf(",\"type\":\"%s\",\"rhs\":", type);
        printf(" %s ", type);
        print_operation_tree(root->rhs);
        printf(")");
        // printf("}");
    }
}

/// @brief Determine if operation trees are equivalent
/// @param expected The expected operation tree
/// @param actual The actual operation tree
/// @return 1 if equivalent, 0 if not
int are_operation_trees_equal(OperationTreeNode *expected, OperationTreeNode *actual)
{
    // Base cases:
    // If any nodes are NULL, we can't check for equality
    if (!expected || !actual)
    {
        return 0;
    }
    // If any nodes are equal, they must be equivalent
    if (expected->result[0] != 'z' && !memcmp(expected->result, actual->result, sizeof(actual->result)))
    {
        // printf("\t%s,%s\n", expected->result, actual->result);
        return 1;
    }

    // Incorrect type
    if (expected->type != actual->type)
    {
        return 0;
    }

    // If not are x.. or y.., compare
    // Store recursive calls for efficiency
    int lhs_lhs_equal = are_operation_trees_equal(expected->lhs, actual->lhs);
    int rhs_rhs_equal = are_operation_trees_equal(expected->rhs, actual->rhs);
    if (lhs_lhs_equal && rhs_rhs_equal)
        return 1;

    int lhs_rhs_equal = are_operation_trees_equal(expected->lhs, actual->rhs);
    int rhs_lhs_equal = are_operation_trees_equal(expected->rhs, actual->lhs);
    if (lhs_rhs_equal && rhs_lhs_equal)
        return 1;

    // If one of the above was equal, but not both in a pair, print the incorrect actual name
    if (lhs_lhs_equal || rhs_lhs_equal)
    {
        // If actual lhs was correct print rhs because it's wrong
        if (actual->rhs)
            printf("%s\n", actual->rhs->result);
        // Return 1 because this is the deepest error and we don't want to print parents
        return 1;
    }
    else if (rhs_rhs_equal || lhs_rhs_equal)
    {
        // If actual rhs was correct
        if (actual->lhs)
            printf("%s\n", actual->lhs->result);
        // Return 1 because this is the deepest error and we don't want to print parents
        return 1;
    }

    return 0;
}

/// @brief Print all erroneous operations ot stdout, as compared to the list in `ADDER_REF_NAME`
/// @param operations The array of operations to do
/// @param operations_size The number of elements in `operations`
void print_all_incorrect_operations(Operation *operations, size_t operations_size)
{
    // Parse the expected adder
    long long x, y;
    OperationVec expected_operations_vec;
    if (parse_input(ADDER_REF_NAME, &x, &y, &expected_operations_vec))
    {
        fprintf(stderr, "Error opening '%s'\n", ADDER_REF_NAME);
        return;
    }

    // Convert those into maps
    HashMap actual_operations = get_operation_tree(operations, operations_size);
    HashMap expected_operations = get_operation_tree(expected_operations_vec.arr, expected_operations_vec.len);

    // Iterate over nodes
    char z_identifier[4];
    OperationTreeNode *actual_node, *expected_node;
    for (int i = 0;; i++)
    {
        // Get the identifier
        // Ignore possible truncation of '%02d' because i should always be at most two digits
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
        snprintf(z_identifier, sizeof(z_identifier), "z%02d", i);
#pragma GCC diagnostic pop
        actual_node = get_map_as_ptr(&actual_operations, z_identifier);
        expected_node = get_map_as_ptr(&expected_operations, z_identifier);

        // Stop when we can't find any more bits
        if (!actual_node || !expected_node)
            break;

        // Determine if the bits are equivalent
        if (!are_operation_trees_equal(expected_node, actual_node))
            // If 1 is returned, this means that both children of this node is wrong, so the node itself is wrong
            printf("%s\n", z_identifier);
    }

    delete_HashMap_and_values(&actual_operations);
    delete_HashMap_and_values(&expected_operations);
    free(expected_operations_vec.arr);
}
