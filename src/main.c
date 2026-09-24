#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "memory_scanner.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <pid> <target_int_value>\n", argv[0]);
        return 1;
    }

    int target_value = atoi(argv[2]);

    // 1. Extract memory region boundaries
    char maps_path[64];
    snprintf(maps_path, sizeof(maps_path), "/proc/%s/maps", argv[1]);

    FILE *map_file = fopen(maps_path, "r");
    if (!map_file) {
        perror("Failed to open maps file");
        return 1;
    }

    RegionList *region_list = create_region_list();
    char line[256];
    while (fgets(line, sizeof(line), map_file)) {
        parse_maps_line(line, region_list);
    }
    fclose(map_file);

    // 2. Open persistent memory stream
    char mem_path[64];
    snprintf(mem_path, sizeof(mem_path), "/proc/%s/mem", argv[1]);
    int mem_fd = open(mem_path, O_RDONLY);
    if (mem_fd < 0) {
        perror("Failed to open memory handle");
        free_region(region_list);
        return 1;
    }

    // 3. Primary pass: scan segments and populate candidate pool
    AddressList *candidates = create_address_list();
    size_t total_regions = get_region_count(region_list);

    for (size_t i = 0; i < total_regions; i++) {
        size_t seg_size = 0;
        unsigned char *buffer = read_memory_segment(mem_fd, region_list, i, &seg_size);
        if (buffer) {
            scan_buffer(buffer, seg_size, region_list, i, target_value, candidates);
            free(buffer);
        }
    }

    printf("\n[+] Initial Pass Complete. Candidates Found: %zu\n", get_address_count(candidates));
    print_addresses(mem_fd, candidates);

    // 4. Iterative refinement loop
    while (candidates && get_address_count(candidates) > 1) {
        printf("\nEnter updated target value (-999 to exit): ");
        int new_val = 0;
        if (scanf("%d", &new_val) != 1 || new_val == -999) break;

        AddressList *filtered = filter_addresses(mem_fd, candidates, new_val);
        free_addr_list(candidates);
        candidates = filtered;

        printf("[+] Refinement Pass Complete. Remaining Candidates: %zu\n", get_address_count(candidates));
        print_addresses(mem_fd, candidates);
    }

    // 5. System resource release
    free_addr_list(candidates);
    free_region(region_list);
    close(mem_fd);

    return 0;
}