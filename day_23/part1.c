#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../c-data-structures/vector/vector_template.h"

// 26 * 26
#define ADJACENCY_MAT_SIZE 676

// Represents a connection from lhs to rhs
typedef struct Connection
{
    char lhs[2];
    char rhs[2];
} Connection;

DEF_VEC(Connection)
DEF_VEC(short)

int parse_input(char *input_file, Connection_Vec *connections);
long long count_sets_of_3(Connection *connections, size_t connections_size);
long long count_adjacencies(bool **adjacency_matrix, short original_element, short this_element, short *neighbors_to_try, size_t neighbors_to_try_size);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    Connection_Vec connections;

    if (parse_input(input_file, &connections))
        return 1;

    long long sets_of_3 = count_sets_of_3(connections.arr, connections.len);

    printf("Number of sets of 3: %lld\n", sets_of_3);

    free(connections.arr);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param connections Out: the vector of connections in input.txt
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, Connection_Vec *connections)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *connections = new_Connection_Vec();

    Connection conn;
    while (fscanf(f, "%2c-%2c\n", conn.lhs, conn.rhs) == 2)
        append_Connection_Vec(connections, conn);
    return 0;
}

/// @brief Count elements in `neighbors_to_try` which are adjacent to `this_element`
/// @param adjacency_matrix The adjacency matrix to use
/// @param this_element The element to look for adjacencies from
/// @param neighbors_to_try The list of neighbors in which to look for adjacencies
/// @param neighbors_to_try_size The number of elements in `neighbors_to_try`
/// @return The number of elements in `neighbors_to_try` which are adjacent to `this_element`
long long count_adjacencies(bool **adjacency_matrix, short original_element, short this_element, short *neighbors_to_try, size_t neighbors_to_try_size)
{
    long long adjacencies = 0;
    for (size_t i = 0; i < neighbors_to_try_size; i++)
    {
        if (adjacency_matrix[this_element][neighbors_to_try[i]])
        {
            printf(
                "%c%c,%c%c,%c%c\n",
                (original_element / 26) + 'a',
                (original_element % 26) + 'a',
                (this_element / 26) + 'a',
                (this_element % 26) + 'a',
                (neighbors_to_try[i] / 26) + 'a',
                (neighbors_to_try[i] % 26) + 'a');
            adjacencies++;
        }
    }
    return adjacencies;
}

/// @brief Count distinct groups of three computers where at least one computer starts with 't'
/// @param connections The array of connections
/// @param connections_size The number of elements in `connections`
/// @return The number of distinct sets of 3 containing at least 1 't*' computer
long long count_sets_of_3(Connection *connections, size_t connections_size)
{
    long long set_count = 0;

    // Generate and populate adjacency matrix
    // Bitfields may be better, but there's only 676 distinct possible computers
    bool **adjacency_matrix = malloc(sizeof(bool *) * ADJACENCY_MAT_SIZE);
    for (int i = 0; i < ADJACENCY_MAT_SIZE; i++)
        adjacency_matrix[i] = calloc(ADJACENCY_MAT_SIZE, sizeof(bool));

    // Populate
    for (size_t i = 0; i < connections_size; i++)
    {
        //                            letter 1                 concatenate letter 2
        unsigned short lhs_index = ((connections[i].lhs[0] - 'a') * 26) + (connections[i].lhs[1] - 'a');
        unsigned short rhs_index = ((connections[i].rhs[0] - 'a') * 26) + (connections[i].rhs[1] - 'a');

        adjacency_matrix[lhs_index][rhs_index] = true;
        adjacency_matrix[rhs_index][lhs_index] = true;
    }

    short_Vec neighbors = new_short_Vec();

    // Loop over all computers starting with t
    for (int i = (('t' - 'a') * 26); i <= (('t' - 'a') * 26) + ('z' - 'a'); i++)
    {
        // Loop over neighbors of this
        for (int j = 0; j < ADJACENCY_MAT_SIZE; j++)
        {
            // Skip later t* neighbors because they will be counted later
            if ((j / 26 == 't' - 'a') && (j >= i))
            {
                j = (('t' - 'a') * 26) + ('z' - 'a');
                continue;
            }
            if (adjacency_matrix[i][j])
            {
                // Check its neighbors
                set_count += count_adjacencies(adjacency_matrix, i, j, neighbors.arr, neighbors.len);
                // Add it to the vector of known adjacencies
                append_short_Vec(&neighbors, j);
            }
        }

        // Reset neighbors vector without freeing it
        neighbors.len = 0;
    }

    free(neighbors.arr);
    for (int i = 0; i < ADJACENCY_MAT_SIZE; i++)
        free(adjacency_matrix[i]);
    free(adjacency_matrix);
    return set_count;
}
