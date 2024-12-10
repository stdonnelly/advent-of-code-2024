#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define IN_FILE "input.txt"

// Doubly linked list node
typedef struct ListNode
{
    struct ListNode *prev;
    struct ListNode *next;
    // id of file, or -1 if the file is actually a free space
    int id;
    // Size of file/free space
    short size;
} ListNode;

typedef struct LinkedList
{
    ListNode *head;
    ListNode *tail;
} LinkedList;

void append_list(LinkedList *list, int id, short size);
void move_node(LinkedList *list, ListNode *source, ListNode *new_parent);
void delete_node(LinkedList *list, ListNode *node);
void delete_list(LinkedList *list);
ListNode *find_first_free_space(LinkedList *list, ListNode *file_node);

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

    LinkedList linked_list = {NULL, NULL};

    // Put the files and free spaces into the list
    for (int i = 0; i < files_size; i++)
    {
        // Append the file
        append_list(&linked_list, i, files[i]);
        // Append free space
        if (i < free_space_size && free_space[i])
            append_list(&linked_list, -1, free_space[i]);
    }

    // Loop over files backwards
    ListNode *this_file = linked_list.tail;
    // Keep track of the next id to look for to avoid trying to move files multiple times
    int expected_id = linked_list.tail->id;
    // Keep track of file sizes we know are too big from the last iteration
    short too_big = (short)10;
    while ((expected_id > 0) && this_file)
    {
        // Save the previous node for later
        // ListNode *prev = this_file->prev;
        // Don't check if this isn't a file, or we know we can't move this file
        if (/* implicit this_file->id != -1*/ this_file->id == expected_id && this_file->size <= too_big)
        {
            // Ignore 0-length files
            if (this_file->size)
            {
                // Search for a free space
                ListNode *first_free_space = find_first_free_space(&linked_list, this_file);
                if (first_free_space)
                {
                    move_node(&linked_list, this_file, first_free_space->prev);
                }
            }
            expected_id--;
        }

        // Go to the previous node for the next iteration
        this_file = this_file->prev;
    }

    // {
    //     printf("List:");
    //     ListNode *node = linked_list.head;
    //     while (node)
    //     {
    //         printf(" (%d,%hhd)", node->id, node->size);
    //         node = node->next;
    //     }
    // }

    // Now, loop forward over the compressed list
    this_file = linked_list.head;
    while (this_file)
    {
        if (this_file->id != -1)
            // If the file is a file, add to the checksum
            for (int i = 0; i < this_file->size; i++)
                checksum += (this_file->id) * (compressed_pos++);
        else
            // If the file is not a file, just increase the position
            compressed_pos += this_file->size;
        this_file = this_file->next;
    }

    delete_list(&linked_list);
    return checksum;
}

/// @brief Append an element to this list
/// @param list The linked list
/// @param id The id of the file or -1 for a free space
/// @param size The size of the file/free space
void append_list(LinkedList *list, int id, short size)
{
    ListNode *new_node = malloc(sizeof(ListNode));
    new_node->id = id;
    new_node->size = size;
    new_node->next = NULL;

    // If there is no head, make this the new head
    if (!list->head)
        list->head = new_node;

    // Append to tail
    if (list->tail)
    {
        list->tail->next = new_node;
        new_node->prev = list->tail;
    }
    else
        new_node->prev = NULL;

    // Make this the new tail
    list->tail = new_node;
}

// Free all elements in a list
void delete_list(LinkedList *list)
{
    // Loop over all nodes, starting at the head
    ListNode *node = list->head;
    ListNode *temp;
    while (node)
    {
        temp = node->next;
        free(node);
        node = temp;
    }

    // Clear head and tail
    list->head = NULL;
    list->tail = NULL;
}

/// @brief Delete node from list
/// @param list The linked list
/// @param node The node to delete
void delete_node(LinkedList *list, ListNode *node)
{
    // Verify the node is non-null
    if (!node)
        return;

    if (list->head == node)
        list->head = node->next;
    else if (node->prev)
        node->prev->next = node->next;
    else
    {
        fprintf(stderr, "Illegal state: node has no prev, but is not head\n");
        exit(1);
    }

    if (list->tail == node)
        list->tail = node->prev;
    else if (node->next)
        node->next->prev = node->prev;
    else
    {
        fprintf(stderr, "Illegal state: node has no next, but is not tail\n");
        exit(1);
    }

    free(node);
}

/// @brief Move a node from `source` to directly after `new_parent`
/// @param list The linked list
/// @param source The source node
/// @param new_parent The destination parent
void move_node(LinkedList *list, ListNode *source, ListNode *new_parent)
{
    // Panic if source or new_parent are NULL
    if (!source || !new_parent)
    {
        fprintf(stderr, "Illegal argument: attempting to move either to or from NULL\n");
        exit(1);
    }

    // Create a new node for source
    // We can't actually move because we need to leave free space
    ListNode *node = malloc(sizeof(ListNode));
    node->id = source->id;
    node->size = source->size;

    // If the new parent is the tail, update the tail to be the moved node
    if (list->tail == new_parent)
        list->tail = node;
    else if (new_parent->next)
    {
        // Shrink the free space
        new_parent->next->size -= node->size;
        // Delete the node if it's size is zero
        if (!new_parent->next->size)
            delete_node(list, new_parent->next);
    }

    if (new_parent->next)
        new_parent->next->prev = node;

    node->next = new_parent->next;
    node->prev = new_parent;
    new_parent->next = node;

    // Make source free space
    source->id = -1;
    // Merge with previous and next
    if (source->prev && source->prev->id == -1)
    {
        source->size += source->prev->size;
        delete_node(list, source->prev);
    }
    if (source->next && source->next->id == -1)
    {
        source->size += source->next->size;
        delete_node(list, source->next);
    }
}

/// @brief Find the first free space before `file_node` that will fit `file_node`
/// @param list The linked list
/// @param file_node The file node to search for a free space for
/// @return The free space node if it exists, NULL otherwise
ListNode *find_first_free_space(LinkedList *list, ListNode *file_node)
{
    ListNode *node = list->head;
    while (node && node != file_node)
    {
        if (node->id == -1 && node->size >= file_node->size)
            return node;
        node = node->next;
    }

    return NULL;
}
