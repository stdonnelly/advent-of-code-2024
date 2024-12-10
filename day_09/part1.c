#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define IN_FILE "input.txt"

int parse_input(char **files, size_t *files_size, char **free_space, size_t *free_space_size);
unsigned long long compressed_checksum(char *files, size_t files_size, char *free_space, size_t free_space_size);

int main(int argc, char const *argv[])
{
    char *files;
    size_t files_size;
    char *free_space;
    size_t free_space_size;
    if (parse_input(&files, &files_size, &free_space, &free_space_size))
        return 1;
    printf("Files:      ");
    for (size_t i = 0; i < files_size; i++)
        printf("%hhd", files[i]);
    printf("\nFree space: ");
    for (size_t i = 0; i < free_space_size; i++)
        printf("%hhd", free_space[i]);
    printf("\n");

    printf("\nFilesystem checksum: %llu\n", compressed_checksum(files, files_size, free_space, free_space_size));

    free(files);
    free(free_space);
    return 0;
}

/// @brief Parse input into arrays `files` and `free_space`
/// @param files Out: The array of file sizes as bytes representing the numbers 0-9
/// @param files_size Out: The number of elements in `files`
/// @param free_space Out: The array of free space sizes as bytes representing the numbers 0-9
/// @param free_space_size Out: The number of elements in `free_space`
/// @return 0 if successful. Some nonzero number if unsuccessful (i.e. IO error)
int parse_input(char **files, size_t *files_size, char **free_space, size_t *free_space_size)
{
    // Use the POSIX stat to get the size of the input file
    struct stat statbuf;
    if (stat(IN_FILE, &statbuf))
    {
        perror("Error opening input file stats");
        return 1;
    }
    // Because the files and free spaces are alternating characters,
    // allocate an array for files and free_space equal to half the file size, rounded up
    size_t half_file_size = ((size_t)statbuf.st_size / 2) + (statbuf.st_size % 2);

    *files = malloc(sizeof(*files[0]) * half_file_size);
    *free_space = malloc(sizeof(*free_space[0]) * half_file_size);
    *files_size = 0UL;
    *free_space_size = 0UL;

    // Open input.txt or panic
    FILE *f = fopen(IN_FILE, "r");
    if (f == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    // Loop over the input, alternating between adding a file and adding a free space
    int ch = getc(f);
    int is_file = 1;
    while ('0' <= ch && ch <= '9')
    {
        if (is_file)
        {
            if (*files_size >= half_file_size)
            {
                printf("Unexpected file %lu. Maximum file count %lu.\n", *files_size, half_file_size);
                exit(1);
            }
            (*files)[(*files_size)++] = (char)ch - '0';
        }
        else
        {
            if (*free_space_size >= half_file_size)
            {
                printf("Unexpected free space %lu. Maximum free space count %lu.\n", *files_size, half_file_size);
                exit(1);
            }
            (*free_space)[(*free_space_size)++] = (char)ch - '0';
        }

        is_file = !is_file;
        ch = getc(f);
    }

    return 0;
}

/// @brief Find the filesystem checksum after compression
/// @param files The array of file sizes
/// @param files_size The number of elements in `files`
/// @param free_space The array of free space sizes
/// @param free_space_size The number of elements in `free_space`
/// @return The filesystem checksum
unsigned long long compressed_checksum(char *files, size_t files_size, char *free_space, size_t free_space_size)
{
    unsigned long long checksum = 0ULL;
    // The position in the compressed filesystem
    unsigned long long compressed_pos = 0ULL;
    // A cursor for the first file and free space
    size_t file_start_cursor = 0UL;
    // A cursor for the last file
    size_t file_end_cursor = files_size - 1;

    // Keep looping until we reached the file we are putting backwards
    while (file_start_cursor <= file_end_cursor)
    {
        if (files[file_start_cursor])
        {
            // If we are looking at a file at the start, use that
            // The cursor is also the file id, so we use that when getting the checksum value
            checksum += compressed_pos * file_start_cursor;
            // Decrement the size of the file as a way of counting the file size
            files[file_start_cursor]--;
        }
        else if (free_space[file_start_cursor])
        {
            // If there is no file size left, pull from the back while there is still free space
            // Don't pull from the back or update the position if there is no file left in the back
            if (!files[file_end_cursor])
            {
                file_end_cursor--;
                continue;
            }

            // If there is a file in the back, use it and decrement both the back file and the free space
            checksum += compressed_pos * file_end_cursor;
            free_space[file_start_cursor]--;
            files[file_end_cursor]--;
        }
        else
        {
            // If there is no size left in either the files or the free space, go to the next file
            file_start_cursor++;
            // And do not increment the compressed position
            continue;
        }
        compressed_pos++;
    }

    return checksum;
}
