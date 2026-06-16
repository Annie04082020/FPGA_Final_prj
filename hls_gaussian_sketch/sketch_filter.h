#ifndef SKETCH_FILTER_H
#define SKETCH_FILTER_H

#include <ap_int.h>

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

// 自訂的 Window 結構，替代被 Vitis HLS 棄用的 hls::Window
template<int ROWS, int COLS, typename T>
struct MyWindow {
    T val[ROWS][COLS];

    void shift_right() {
#pragma HLS inline
        for (int i = 0; i < ROWS; i++) {
#pragma HLS unroll
            for (int j = 0; j < COLS - 1; j++) {
#pragma HLS unroll
                val[i][j] = val[i][j + 1];
            }
        }
    }

    void insert(T pixel, int row, int col) {
#pragma HLS inline
        val[row][col] = pixel;
    }

    T getval(int row, int col) {
#pragma HLS inline
        return val[row][col];
    }
};

// 自訂的 LineBuffer 結構，替代被 Vitis HLS 棄用的 hls::LineBuffer
template<int ROWS, int COLS, typename T>
struct MyLineBuffer {
    T val[ROWS][COLS];

    void shift_up(int col) {
#pragma HLS inline
        for (int i = 0; i < ROWS - 1; i++) {
#pragma HLS unroll
            val[i][col] = val[i + 1][col];
        }
    }

    void insert_bottom(T pixel, int col) {
#pragma HLS inline
        val[ROWS - 1][col] = pixel;
    }

    T getval(int row, int col) {
#pragma HLS inline
        return val[row][col];
    }
};

// 高階版 IP 介面宣告
void sketch_filter(
    unsigned char *src_mem,
    unsigned char *dst_mem,
    int rows,
    int cols
);

#endif // SKETCH_FILTER_H
