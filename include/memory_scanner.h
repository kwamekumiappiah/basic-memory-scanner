#ifndef MEOMORY_SCANNER_H
#define MEMORY_SCANNER_H

typedef struct RegionList RegionList;

RegionList *create_region_list(void);

void free_region(RegionList *region_list);

void parse_maps_line(char *line, RegionList *region_list);

#endif
