#include <stdio.h>
#include <stdlib.h>

// Doubly linked list node
typedef struct ListNode
{
    struct ListNode *prev;
    struct ListNode *next;
    // id of file, or -1 if the file is actually a free space
    int id;
    // Size of file/free space
    char size;
} ListNode;

typedef struct LinkedList
{
    ListNode *head;
    ListNode *tail;
} LinkedList;

void append_list(LinkedList *list, int id, char size);
void move_node(LinkedList *list, ListNode *source, ListNode *new_parent);
void delete_node(LinkedList *list, ListNode *node);
void delete_list(LinkedList *list);
ListNode *find_first_free_space(LinkedList *list, ListNode *file_node);

int main(int argc, char const *argv[])
{
    LinkedList list = {NULL, NULL};
    append_list(&list, 1, 2);
    append_list(&list, 1, 6);
    append_list(&list, 5, 6);
    ListNode *node_to_use = list.tail;
    append_list(&list, -1, 8);

    ListNode *free_node = find_first_free_space(&list, node_to_use);
    if (free_node)
        printf("First free space: (%d,%hhd)\n", free_node->id, free_node->size);
    else
        printf("No free space\n");

    printf("Forwards:");
    ListNode *node = list.head;
    while (node)
    {
        printf(" (%d,%hhd)", node->id, node->size);
        node = node->next;
    }
    printf("\nBackwards:");
    node = list.tail;
    while (node)
    {
        printf(" (%d,%hhd)", node->id, node->size);
        node = node->prev;
    }
    printf("\n");

    delete_list(&list);
    return 0;
}

/// @brief Append an element to this list
/// @param list The linked list
/// @param id The id of the file or -1 for a free space
/// @param size The size of the file/free space
void append_list(LinkedList *list, int id, char size)
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

    // Remove source from wherever it came from but do not free it
    if (list->head == source)
        list->head = source->next;
    else if (source->prev)
        source->prev->next = source->next;
    else
    {
        fprintf(stderr, "Illegal state: source has no prev, but is not head\n");
        exit(1);
    }
    if (list->tail == source)
        list->tail = source->prev;
    else if (source->next)
        source->next->prev = source->prev;
    else
    {
        fprintf(stderr, "Illegal state: source has no next, but is not tail\n");
        exit(1);
    }

    // If the new parent is the tail, update the tail to be the moved node
    if (list->tail == new_parent)
        list->tail = source;

    if (new_parent->next)
        new_parent->next->prev = source;

    source->next = new_parent->next;
    source->prev = new_parent;
    new_parent->next = source;
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
