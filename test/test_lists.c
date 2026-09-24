#include <assert.h>
#include <stdio.h>
#include "memory_scanner.h"

static void test_address_list_growth(void) {
    AddressList *l = create_address_list();
    assert(l && get_address_count(l) == 0);

    for (unsigned long i = 0; i < 100; i++)   /* crosses several grow steps */
        assert(add_address(l, i) == 0);
    assert(get_address_count(l) == 100);

    free_addr_list(l);
    assert(get_address_count(NULL) == 0);
    assert(add_address(NULL, 1) != 0);
}

static void test_parse_maps_line(void) {
    RegionList *r = create_region_list();
    char rw[] = "55d0c000-55d0d000 rw-p 00000000 00:00 0 [heap]";
    char ro[] = "55d0d000-55d0e000 r--p 00000000 00:00 0 /lib/libc.so";
    char rx[] = "55d0e000-55d0f000 r-xp 00000000 00:00 0 /lib/libc.so";
    char bad[] = "garbage";

    parse_maps_line(rw, r);
    parse_maps_line(ro, r);
    parse_maps_line(rx, r);
    parse_maps_line(bad, r);

    assert(get_region_count(r) == 1);   /* only the rw region is kept */
    free_region(r);
}

static void test_scan_buffer(void) {
    RegionList *r = create_region_list();
    char line[] = "1000-1040 rw-p 00000000 00:00 0";
    parse_maps_line(line, r);

    int data[16] = {0};
    data[3] = 42;
    data[9] = 42;

    AddressList *hits = create_address_list();
    scan_buffer((unsigned char *)data, sizeof data, r, 0, 42, hits);
    assert(get_address_count(hits) == 2);

    free_addr_list(hits);
    free_region(r);
}

int main(void) {
    test_address_list_growth();
    test_parse_maps_line();
    test_scan_buffer();
    puts("\nAll unit tests passed.");
    return 0;
}