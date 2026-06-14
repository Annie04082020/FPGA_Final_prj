#include "sketch_filter.h"

// 空殼 HLS IP: 將 src_mem 的資料直接複製到 dst_mem
void sketch_filter(
    unsigned char *src_mem,
    unsigned char *dst_mem,
    int rows,
    int cols
) {
    // AXI Master 介面：負責對應 PYNQ 的 CMA 連續實體記憶體 (支援高達 1080p 解析度)
    #pragma HLS INTERFACE m_axi port=src_mem depth=2073600 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dst_mem depth=2073600 offset=slave bundle=gmem1
    
    // AXI-Lite 介面：負責控制 IP 啟動與參數傳遞
    #pragma HLS INTERFACE s_axilite port=rows bundle=control
    #pragma HLS INTERFACE s_axilite port=cols bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    int total_pixels = rows * cols;
    
    // Day 1 空殼迴圈：直接把來源影像複製到輸出
    for (int i = 0; i < total_pixels; i++) {
        #pragma HLS PIPELINE II=1
        dst_mem[i] = src_mem[i];
    }
}
