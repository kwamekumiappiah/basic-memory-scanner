#ifndef MEMORY_SCANNER_H
#define MEMORY_SCANNER_H

#include <stddef.h>

// Opaque structure pointer for information hiding
typedef struct RegionList RegionList;

RegionList *create_region_list(void);
void free_region(RegionList *region_list);
void parse_maps_line(char *line, RegionList *region_list);

// Accessor function to preserve opacity in main.c
size_t get_region_count(const RegionList *list);

// High-efficiency scanning interface using pre-opened file descriptor
unsigned char *read_memory_segment(int mem_fd, const RegionList *list, size_t index, size_t *out_size);
void scan_buffer(const unsigned char *buffer, size_t seg_size, const RegionList *list, size_t index, int target_val);

#endif