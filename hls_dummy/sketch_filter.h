#ifndef SKETCH_FILTER_H
#define SKETCH_FILTER_H

#include <ap_int.h>

// IP 介面宣告
void sketch_filter(
    unsigned char *src_mem,
    unsigned char *dst_mem,
    int rows,
    int cols
);

#endif // SKETCH_FILTER_H
