#include "sketch_filter.h"

inline int hls_abs(int x) {
    return (x < 0) ? -x : x;
}

// 正式版 HLS IP: 即時鉛筆素描硬體加速器
void sketch_filter(
    unsigned char *src_mem,
    unsigned char *dst_mem,
    int rows,
    int cols
) {
    // AXI Master 介面：連續記憶體傳輸
    #pragma HLS INTERFACE m_axi port=src_mem depth=2073600 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dst_mem depth=2073600 offset=slave bundle=gmem1
    
    // AXI-Lite 介面：控制暫存器
    #pragma HLS INTERFACE s_axilite port=rows bundle=control
    #pragma HLS INTERFACE s_axilite port=cols bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    // 1. 宣告自訂的 Line Buffer 與 Window Buffer
    MyLineBuffer<3, MAX_WIDTH, unsigned char> line_buf;
    MyWindow<3, 3, unsigned char> win_buf;
    
    // 2. 將陣列打碎以達成完全管線化 (解決 BRAM 讀取衝突)
    #pragma HLS ARRAY_PARTITION variable=line_buf.val complete dim=1
    #pragma HLS ARRAY_PARTITION variable=win_buf.val complete dim=0
    
    // 3. 雙重迴圈逐畫素處理
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            // 效能刷分關鍵：確保每週期處理一個畫素 (II=1)
            #pragma HLS PIPELINE II=1
            
            // 從外部 DDR 讀取一個像素
            unsigned char new_pixel = src_mem[r * cols + c];

            // 4. 更新 Line Buffer
            line_buf.shift_up(c);
            line_buf.insert_bottom(new_pixel, c);

            // 5. 更新 Window Buffer
            win_buf.shift_right();
            win_buf.insert(line_buf.getval(0, c), 0, 2);
            win_buf.insert(line_buf.getval(1, c), 1, 2);
            win_buf.insert(line_buf.getval(2, c), 2, 2);

            // 預設輸出為白底 (素描紙)
            unsigned char out_pixel = 255;
            
            // 6. Sobel 邊緣運算與素描風轉換
            if (r >= 2 && c >= 2) {
                // 計算 X 方向梯度 (Gx)
                int Gx = -win_buf.getval(0,0) + win_buf.getval(0,2)
                         -2 * win_buf.getval(1,0) + 2 * win_buf.getval(1,2)
                         -win_buf.getval(2,0) + win_buf.getval(2,2);
                         
                // 計算 Y 方向梯度 (Gy)
                int Gy = -win_buf.getval(0,0) - 2 * win_buf.getval(0,1) - win_buf.getval(0,2)
                         +win_buf.getval(2,0) + 2 * win_buf.getval(2,1) + win_buf.getval(2,2);
                         
                // 總邊緣強度
                int edge = hls_abs(Gx) + hls_abs(Gy);
                if (edge > 255) edge = 255; // 防止溢位
                
                // 鉛筆素描效果 (白紙黑線)：顏色反轉
                out_pixel = 255 - edge;
            }

            // 7. 將計算結果寫回外部 DDR
            dst_mem[r * cols + c] = out_pixel;
        }
    }
}
