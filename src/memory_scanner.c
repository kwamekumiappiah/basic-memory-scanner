#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "memory_scanner.h"

typedef struct {
    unsigned long start;
    unsigned long end;
} MemoryRegion;

typedef struct RegionList {
    MemoryRegion *regions;
    size_t count;
    size_t capacity;
} RegionList;

typedef struct AddressList {
    unsigned long *addresses;
    size_t count;
    size_t capacity;
} AddressList;

AddressList *create_address_list(void) {
    AddressList *addr_list = malloc(sizeof(AddressList));
    if (!addr_list) return NULL;
    addr_list->addresses = NULL;
    addr_list->capacity = 0;
    addr_list->count = 0;
    return addr_list;
}

void free_addr_list(AddressList *list) {
    if (!list) return;
    free(list->addresses);
    free(list);
}

int add_address(AddressList *list, unsigned long addr) {
    if (!list) return 1;

    // 1. Check if the array needs to grow
    if (list->count == list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 10 : list->capacity * 2;
        
        // Reallocate the addresses array buffer
        unsigned long *temp = realloc(list->addresses, new_capacity * sizeof(unsigned long));
        if (!temp) return 1; // Memory reallocation failed

        list->addresses = temp;
        list->capacity = new_capacity;
    }

    // 2. Append the address and increment count
    list->addresses[list->count] = addr;
    list->count++;

    return 0; // Success
}


RegionList *create_region_list(void) {
    RegionList *region_list = malloc(sizeof(RegionList));
    if (!region_list) return NULL;
    region_list->regions = NULL;
    region_list->capacity = 0;
    region_list->count = 0;
    return region_list;
}

size_t get_region_count(const RegionList *list) {
    return list ? list->count : 0;
}

void free_region(RegionList *region_list) {
    if (!region_list) return;
    free(region_list->regions);
    free(region_list);
}

int add_region(RegionList *list, unsigned long start, unsigned long end) {
    if (!list) return 1;

    if (list->count == list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 10 : list->capacity * 2;
        MemoryRegion *temp = realloc(list->regions, new_capacity * sizeof(MemoryRegion));
        if (!temp) {
            perror("Failed to reallocate memory regions");
            return 1;
        }
        list->regions = temp;
        list->capacity = new_capacity;
    }

    list->regions[list->count].start = start;
    list->regions[list->count].end = end;
    list->count++;

    return 0;
}


void parse_maps_line(char *line, RegionList *region_list) {
    unsigned long start, end;
    char perms[5];

    if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3) {
        if ((perms[0] == 'r') && (perms[1] == 'w')) {
            add_region(region_list, start, end);
        }
    }
}

unsigned char *read_memory_segment(int mem_fd, const RegionList *list, size_t index, size_t *out_size) {
    if (!list || index >= list->count || mem_fd < 0) return NULL;

    unsigned long start = list->regions[index].start;
    unsigned long end = list->regions[index].end;
    size_t seg_size = end - start;

    unsigned char *buffer = malloc(seg_size);
    if (!buffer) return NULL;

    ssize_t bytes_read = pread(mem_fd, buffer, seg_size, start);
    if (bytes_read <= 0) {
        free(buffer);
        return NULL;
    }

    *out_size = seg_size;
    return buffer;
}

size_t get_address_count(const AddressList *list) {
    return list ? list->count : 0;
}

void scan_buffer(const unsigned char *buffer, size_t seg_size, const RegionList *list, size_t index, int target_val, AddressList *addr_list) {
    if (!buffer || !list || !addr_list || index >= list->count || seg_size < sizeof(int)) return;

    unsigned long base_addr = list->regions[index].start;

    for (size_t i = 0; i <= seg_size - sizeof(int); i++) {
        int current_val = *(int *)(buffer + i);
        if (current_val == target_val) {
            unsigned long target_addr = base_addr + i;
            add_address(addr_list, target_addr);
            printf("Found match! Target Address: 0x%lx | Value: %d\n", target_addr, current_val);
        }
    }
}

AddressList *filter_addresses(int mem_fd, AddressList *old_list, int new_target_val) {
    if (!old_list) return NULL;

    AddressList *new_list = create_address_list();
    if (!new_list) return NULL;

    for (size_t i = 0; i < old_list->count; i++) {
        int hold_value = 0;
        ssize_t bytes_read = pread(mem_fd, &hold_value, sizeof(int), old_list->addresses[i]);
        if (bytes_read <=0 ) {
            continue;
        }
        if (hold_value == new_target_val) {
            add_address(new_list, old_list->addresses[i]);
        }
    }
    return new_list;
}

void print_addresses(int mem_fd, const AddressList *list) {
    if (!list) return;

    for (size_t i = 0; i < list->count; i++) {
        int value = 0;
        ssize_t byte_read = pread(mem_fd, &value, sizeof(int), list->addresses[i]);
        
        if (byte_read == sizeof(int)) {
            printf("Address: 0x%lx | Value: %d\n", list->addresses[i], value);
        } else {
            printf("Address: 0x%lx | [Read Failed]\n", list->addresses[i]);
        }
    }
}