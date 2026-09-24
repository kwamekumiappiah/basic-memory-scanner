#include <stdlib.h>
#include <stdio.h>
#include "memory_scanner.h"

typedef struct {
    unsigned long start;
    unsigned long end;
} MemoryRegion;

typedef struct {
    MemoryRegion *regions; // Pointer to dynamically allocated array
    size_t count;          // Current number of stored regions
    size_t capacity;       // Maximum capacity before needing to resize
} RegionList;

RegionList *create_region_list(void) {
    RegionList *region_list = malloc(sizeof(RegionList));
    if (!region_list) return NULL;
    region_list->regions = NULL;
    region_list->capacity = 0;
    region_list->count = 0;
    return region_list;
}

void free_region(RegionList *region_list) {
    if (!region_list) return;
    free(region_list->regions);
    free(region_list);
}

int add_region(RegionList *list, unsigned long start, unsigned long end) {
    if (!list) return 1;

    // 1. Check if the array is full
    if (list->count == list->capacity) {
        // Handle initial capacity if 0, otherwise double it
        size_t new_capacity = (list->capacity == 0) ? 10 : list->capacity * 2;

        // Reallocate memory for list->regions using total size in bytes
        MemoryRegion *temp = realloc(list->regions, new_capacity * sizeof(MemoryRegion));
        if (!temp) {
            perror("Failed to reallocate memory regions");
            return 1;
        }

        list->regions = temp;
        list->capacity = new_capacity;
    }

    // 2. Assign values directly to the next available slot
    list->regions[list->count].start = start;
    list->regions[list->count].end = end;
    list->count++;

    return 0; // Success
}


void parse_maps_line(char *line, RegionList *region_list) {
    unsigned long start, end;
    char perms[5]; // Stores strings like "rw-p" plus '\0'

    // Format: "start-end perms ..."
    if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3) {
        if ((perms[0] == 'r') && (perms[1] == 'w')) {
            add_region(region_list, start, end);
            printf("Range: 0x%lx - 0x%lx | Perms: %s\n", start, end, perms);
        }
    }
}