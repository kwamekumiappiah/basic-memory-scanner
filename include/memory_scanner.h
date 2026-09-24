#ifndef MEMORY_SCANNER_H
#define MEMORY_SCANNER_H

#include <stddef.h>

// Opaque structure types for strict information hiding
typedef struct RegionList RegionList;
typedef struct AddressList AddressList;

// --- Region List Interface ---
RegionList *create_region_list(void);
void free_region(RegionList *region_list);
size_t get_region_count(const RegionList *list);
void parse_maps_line(char *line, RegionList *region_list);

// --- Candidate Address List Interface ---
AddressList *create_address_list(void);
void free_addr_list(AddressList *list);
int add_address(AddressList *list, unsigned long addr);
size_t get_address_count(const AddressList *list); // Added accessor function

// --- Memory Reading & First-Pass Scanning ---
unsigned char *read_memory_segment(int mem_fd, const RegionList *list, size_t index, size_t *out_size);
void scan_buffer(const unsigned char *buffer, size_t seg_size, const RegionList *list, size_t index, int target_val, AddressList *addr_list); // Updated 6-param signature

// --- Filtering & Reporting Interface ---
AddressList *filter_addresses(int mem_fd, AddressList *old_list, int new_target_val);
void print_addresses(int mem_fd, const AddressList *list);

#endif // MEMORY_SCANNER_H