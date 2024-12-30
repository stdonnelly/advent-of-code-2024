#pragma once

#include <stdlib.h>
#include <string.h>
#include <zlib.h>

typedef struct MapEntry
{
    struct MapEntry *next;
    // Value is a union type so it can either be a 64-bit primitive or a pointer to some other data structure
    union
    {
        long long value;
        void *value_ptr;
    };
    char *key;
} MapEntry;

typedef struct HashMap
{
    // An array of buckets, which are themselves represented as singly-linked list nodes
    MapEntry **buckets;
    double load_factor;
    int size;
    int capacity;
} HashMap;

HashMap new_HashMap();
void delete_HashMap(HashMap *map);
void delete_HashMap_and_values(HashMap *map);
long long get_map(HashMap *map, char *key);
void put_map(HashMap *map, char *key, long long value);
int should_grow_HashMap(HashMap *map);
void grow_HashMap(HashMap *map);

// Generate the default hashmap
HashMap new_HashMap()
{
    HashMap map;
    // Defaults from Java standard libray
    map.capacity = 16;
    map.load_factor = 0.75;
    map.buckets = calloc(map.capacity, sizeof(map.buckets[0]));
    map.size = 0;
    return map;
}

// Free the allocated memory contained in map
void delete_HashMap(HashMap *map)
{
    for (int i = 0; i < map->capacity; i++)
    {
        MapEntry *bucket_node = map->buckets[i];
        while (bucket_node)
        {
            MapEntry *next = bucket_node->next;
            free(bucket_node->key);
            free(bucket_node);
            bucket_node = next;
        }
    }
    free(map->buckets);
}

// Free the allocated memory contained in map
void delete_HashMap_and_values(HashMap *map)
{
    for (int i = 0; i < map->capacity; i++)
    {
        MapEntry *bucket_node = map->buckets[i];
        while (bucket_node)
        {
            MapEntry *next = bucket_node->next;
            free(bucket_node->value_ptr);
            free(bucket_node->key);
            free(bucket_node);
            bucket_node = next;
        }
    }
    free(map->buckets);
}

// Get the value at `key` from the map
long long get_map(HashMap *map, char *key)
{
    long long value = -1;
    // Get CRC32 hash and apply bit mask
    unsigned long hash = crc32(0UL, (Bytef *)(key), strlen(key));
    hash &= map->capacity - 1;

    MapEntry *entry = map->buckets[hash];
    while (entry)
    {
        // If the key is equal, use this value
        if (!strcmp(entry->key, key))
        {
            value = entry->value;
            break;
        }

        // If not equal, try the next value in this bucket
        entry = entry->next;
    }

    return value;
}

// Get the value at `key` from the map
void *get_map_as_ptr(HashMap *map, char *key)
{
    void *value_ptr = NULL;
    // Get CRC32 hash and apply bit mask
    unsigned long hash = crc32(0UL, (Bytef *)(key), strlen(key));
    hash &= map->capacity - 1;

    MapEntry *entry = map->buckets[hash];
    while (entry)
    {
        // If the key is equal, use this value
        if (!strcmp(entry->key, key))
        {
            value_ptr = entry->value_ptr;
            break;
        }

        // If not equal, try the next value in this bucket
        entry = entry->next;
    }

    return value_ptr;
}

// Put the value into the map at `key`
void put_map(HashMap *map, char *key, long long value)
{
    // printf("Putting value '%d' at key '%s' in map...\n", value, key);
    // Add an element to the size and check if this should grow
    map->size++;
    if (should_grow_HashMap(map))
        grow_HashMap(map);

    // Get CRC32 hash and apply bit mask
    int key_length = strlen(key);
    unsigned long hash = crc32(0UL, (Bytef *)(key), key_length);
    hash &= map->capacity - 1;

    // Pointer to the bucket pointer so we can modify the bucket pointer instead of just the bucket
    MapEntry **bucket = &(map->buckets[hash]);
    while (*bucket)
    {
        // Check if the key is equal
        if (!strcmp((*bucket)->key, key))
        {
            // If the key matches, replace the value
            (*bucket)->value = value;
            // And note that nothing was actually added
            map->size--;
            return;
        }
        // Next bucket
        bucket = &((*bucket)->next);
    }

    // If we got to the end of the while loop without finding the key, add a new one
    *bucket = malloc(sizeof(**bucket));
    (*bucket)->next = NULL;
    (*bucket)->key = malloc(key_length + 1);
    memcpy((*bucket)->key, key, key_length + 1);
    (*bucket)->value = value;
}

// Put the value into the map at `key`
void put_map_as_ptr(HashMap *map, char *key, void *value_ptr)
{
    // printf("Putting value '%d' at key '%s' in map...\n", value, key);
    // Add an element to the size and check if this should grow
    map->size++;
    if (should_grow_HashMap(map))
        grow_HashMap(map);

    // Get CRC32 hash and apply bit mask
    int key_length = strlen(key);
    unsigned long hash = crc32(0UL, (Bytef *)(key), key_length);
    hash &= map->capacity - 1;

    // Pointer to the bucket pointer so we can modify the bucket pointer instead of just the bucket
    MapEntry **bucket = &(map->buckets[hash]);
    while (*bucket)
    {
        // Check if the key is equal
        if (!strcmp((*bucket)->key, key))
        {
            // If the key matches, replace the value
            (*bucket)->value_ptr = value_ptr;
            // And note that nothing was actually added
            map->size--;
            return;
        }
        // Next bucket
        bucket = &((*bucket)->next);
    }

    // If we got to the end of the while loop without finding the key, add a new one
    *bucket = malloc(sizeof(**bucket));
    (*bucket)->next = NULL;
    (*bucket)->key = malloc(key_length + 1);
    memcpy((*bucket)->key, key, key_length + 1);
    (*bucket)->value_ptr = value_ptr;
}

/// @brief Check if a hashmap should grow
/// @param map The map
/// @return 1 if this should grow, 0 if this should not grow
int should_grow_HashMap(HashMap *map)
{
    return ((double)(map->capacity) * map->load_factor) < (double)(map->size);
}

// Grow the capacity of the map
void grow_HashMap(HashMap *map)
{
    // Get old capacity and double it
    int old_cap = map->capacity;
    map->capacity = old_cap << 1;
    map->size = 0;

    // Pointer to the old array
    MapEntry **old_table = map->buckets;

    // Resize the array
    map->buckets = calloc(map->capacity, sizeof(map->buckets[0]));

    // Loop over all elements in the old table
    for (int i = 0; i < old_cap; i++)
    {
        // Loop over all elements in the bucket
        MapEntry *bucket = old_table[i];
        while (bucket)
        {
            // Put this into the new map
            put_map(map, bucket->key, bucket->value);

            // Get the next element and free this one
            MapEntry *next = bucket->next;
            free(bucket->key);
            free(bucket);
            bucket = next;
        }
    }

    // Finally, free the old table
    free(old_table);
}