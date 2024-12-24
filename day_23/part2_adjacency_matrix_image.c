#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zlib.h>
#include <stdint.h>

#include "../vector_template.h"

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

// Global variable storing the current largest known group
// We can skip earlier if we find a smaller group
int largest_group_size = 3;

int parse_input(char *input_file, ConnectionVec *connections);
bool **generate_adjacency_matrix(Connection *connections, size_t connections_size);
long long count_sets_of_3(Connection *connections, size_t connections_size);
long long count_adjacencies(bool **adjacency_matrix, short original_element, short this_element, short *neighbors_to_try, size_t neighbors_to_try_size);
void print_computers(short *computers, size_t computers_size);
void adjacency_matrix_to_image(bool **adjacency_matrix);
int print_deflated(FILE *out_file, unsigned char *data, int data_length);

int main(int argc, char *argv[])
{
    char *input_file = (argc >= 2) ? argv[1] : NULL;
    ConnectionVec connections;

    if (parse_input(input_file, &connections))
        return 1;

    long long sets_of_3 = count_sets_of_3(connections.arr, connections.len);

    // printf("Number of sets of 3: %lld\n", sets_of_3);

    free(connections.arr);
    return 0;
}

/// @brief Parse the input file
/// @param input_file The path of the file to input from. If null, stdin will be used
/// @param connections Out: the vector of connections in input.txt
/// @return 0 if successful, 1 if unsuccessful
int parse_input(char *input_file, ConnectionVec *connections)
{
    // Open input.txt or panic
    FILE *f = input_file ? fopen(input_file, "r") : stdin;
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    *connections = newConnectionVec();

    Connection conn;
    while (fscanf(f, "%2c-%2c\n", conn.lhs, conn.rhs) == 2)
        appendConnection(connections, conn);
    return 0;
}

// Print the list of computers
void print_computers(short *computers, size_t computers_size)
{
    // Do th first one without a comma
    if (computers_size)
        printf("%c%c", (computers[0] / 26) + 'a', (computers[0] % 26) + 'a');
    for (int i = 1; i < computers_size; i++)
        printf(",%c%c", (computers[i] / 26) + 'a', (computers[i] % 26) + 'a');
    printf("\n");
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
            print_computers((short[]){original_element, this_element, neighbors_to_try[i]}, 3);
            adjacencies++;
        }
    }
    return adjacencies;
}

/// @brief Generate an adjacency matrix for the given list of connections
/// @param connections The array of connections
/// @param connections_size The number of elements in `connections`
/// @return A 2D `ADJACENCY_MAT_SIZE`x`ADJACENCY_MAT_SIZE` array containing every adjacency
bool **generate_adjacency_matrix(Connection *connections, size_t connections_size)
{
    // Allocate
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

    return adjacency_matrix;
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
    bool **adjacency_matrix = generate_adjacency_matrix(connections, connections_size);

    adjacency_matrix_to_image(adjacency_matrix);

    // for (int i = 0; i < ADJACENCY_MAT_SIZE; i++)
    // {
    //     int degree = 0;
    //     for (int j = 0; j < ADJACENCY_MAT_SIZE; j++)
    //         if (adjacency_matrix[i][j])
    //             degree++;
    //     if (degree)
    //         printf("%03d %c%c\n", degree, (i / 26) + 'a', (i % 26) + 'a');
    // }

    // shortVec neighbors = newshortVec();

    // // Loop over all computers starting with t
    // for (int i = (('t' - 'a') * 26); i <= (('t' - 'a') * 26) + ('z' - 'a'); i++)
    // {
    //     // Loop over neighbors of this
    //     for (int j = 0; j < ADJACENCY_MAT_SIZE; j++)
    //     {
    //         // Skip later t* neighbors because they will be counted later
    //         if ((j / 26 == 't' - 'a') && (j >= i))
    //         {
    //             j = (('t' - 'a') * 26) + ('z' - 'a');
    //             continue;
    //         }
    //         if (adjacency_matrix[i][j])
    //         {
    //             // Check its neighbors
    //             set_count += count_adjacencies(adjacency_matrix, i, j, neighbors.arr, neighbors.len);
    //             // Add it to the vector of known adjacencies
    //             appendshort(&neighbors, j);
    //         }
    //     }

    //     // Reset neighbors vector without freeing it
    //     neighbors.len = 0;
    // }

    // free(neighbors.arr);
    for (int i = 0; i < ADJACENCY_MAT_SIZE; i++)
        free(adjacency_matrix[i]);
    free(adjacency_matrix);
    return set_count;
}

#define PRINT_BE(f, o)                                              \
    do                                                              \
    {                                                               \
        for (int i = sizeof(o) - 1; i >= 0; i--)                    \
        {                                                           \
            typeof(o) bitmask = 0xff << (i * 8);                    \
            putc((unsigned char)(((o) & bitmask) >> (i * 8)), (f)); \
        }                                                           \
    } while (0)

// Deflate data and print to file
int print_deflated(FILE *out_file, unsigned char *data, int data_length)
{
    // Return value for a zlib function
    int returned_value;
    // Output buffer
    unsigned char deflate_output[65535];
    // The size of the output data
    int output_size;

    uint32_t crc32_sum = crc32(0, (unsigned char *)"IDAT", 4);

    // Initialize stream
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    returned_value = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (returned_value != Z_OK)
    {
        fprintf(stderr, "deflateInit returned %d\n", returned_value);
        return returned_value;
    }

    strm.avail_in = data_length;
    strm.next_in = data;

    // Keep running deflate until finished
    strm.avail_out = sizeof(deflate_output);
    strm.next_out = deflate_output;

    do
    {
        returned_value = deflate(&strm, Z_FINISH);
        output_size = sizeof(deflate_output) - strm.avail_out;
        // Keep track of the crc32 as well
        crc32_sum = crc32(crc32_sum, deflate_output, output_size);
        fwrite(deflate_output, 1, output_size, out_file);
    } while (!strm.avail_out);

    PRINT_BE(out_file, crc32_sum);
    deflateEnd(&strm);
    return 0;
}

// Convert adjacency matrix to png image
void adjacency_matrix_to_image(bool **adjacency_matrix)
{
    // Row size+1 for filter byte on each row * number of rows
    const int UNCOMPRESSED_BITMAP_SIZE = ((ADJACENCY_MAT_SIZE / 8) + ((ADJACENCY_MAT_SIZE % 8) ? 1 : 0) + 1) * ADJACENCY_MAT_SIZE;
    unsigned char uncompressed_bits[UNCOMPRESSED_BITMAP_SIZE];
    FILE *out_file = fopen("adjacency_matrix.png", "wb+");
    // Write the header
    fwrite("\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 1, 8, out_file);

    // IHDR
    long chunk_beginning = ftell(out_file);
    uint32_t length = 0;
    // Temporarily print length as 0, will be overwritten later
    PRINT_BE(out_file, length);
    fwrite("IHDR", 1, 4, out_file);
    /* IHDR layout from https://en.wikipedia.org/wiki/PNG
        width (4 bytes)
        height (4 bytes)
        bit depth (1 byte, values 1, 2, 4, 8, or 16)
        color type (1 byte, values 0, 2, 3, 4, or 6)
        compression method (1 byte, value 0)
        filter method (1 byte, value 0)
        interlace method (1 byte, values 0 "no interlace" or 1 "Adam7 interlace") (13 data bytes total).[11]
    */
    // Width
    PRINT_BE(out_file, (uint32_t)ADJACENCY_MAT_SIZE);
    // Height
    PRINT_BE(out_file, (uint32_t)ADJACENCY_MAT_SIZE);
    // Bit depth: 1
    PRINT_BE(out_file, (uint8_t)1);
    // Color type: grayscale
    PRINT_BE(out_file, (uint8_t)0);
    // Compression method: always 0
    PRINT_BE(out_file, (uint8_t)0);
    // Filter method: always 0
    PRINT_BE(out_file, (uint8_t)0);
    // Interlace method: no interlace
    PRINT_BE(out_file, (uint8_t)0);

    // The length of the data
    length = ftell(out_file) - 8 - chunk_beginning;
    // Go back to the start to overwrite the length
    fseek(out_file, chunk_beginning, SEEK_SET);
    PRINT_BE(out_file, length);
    // Allocate the size of IHDR+data
    char *header_and_data = malloc(length + 4);
    fread(header_and_data, 1, length + 4, out_file);
    uint32_t chunk_crc = crc32(0L, (unsigned char *)header_and_data, length + 4);
    PRINT_BE(out_file, chunk_crc);

    // IDAT
    length = 0;
    chunk_beginning = ftell(out_file);
    PRINT_BE(out_file, length);
    fwrite("IDAT", 1, 4, out_file);
    // Populate uncompressed bits
    for (int i = 0; i < ADJACENCY_MAT_SIZE; i++)
    {
        int row_offset = i * ((ADJACENCY_MAT_SIZE / 8) + ((ADJACENCY_MAT_SIZE % 8) ? 1 : 0) + 1);
        uncompressed_bits[row_offset] = '\0';
        for (int j = 0; j < ADJACENCY_MAT_SIZE; j++)
            // pack into the bitmap
            if (!adjacency_matrix[i][j])
                uncompressed_bits[(j / 8) + 1 + row_offset] |= 0b10000000 >> (j % 8);
    }
    print_deflated(out_file, uncompressed_bits, UNCOMPRESSED_BITMAP_SIZE);

    // Offset from the start position minus length field, identifier, and crc
    length = ftell(out_file) - chunk_beginning - 12;
    fseek(out_file, chunk_beginning, SEEK_SET);
    PRINT_BE(out_file, length);
    fseek(out_file, 0, SEEK_END);

    // IEND
    fwrite("\0\0\0\0IEND", 1, 8, out_file);
    chunk_crc = crc32(0, (unsigned char *)"IEND", 4);
    PRINT_BE(out_file, chunk_crc);

    free(header_and_data);
    fclose(out_file);
}
