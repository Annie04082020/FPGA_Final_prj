#include "sketch_filter.h"

// 終極進階版 HLS IP: 高斯模糊 + 顏色加亮 (Color Dodge) 鉛筆素描加速器
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

    // 1. 宣告自訂的 5x5 Line Buffer 與 Window Buffer
    MyLineBuffer<5, MAX_WIDTH, unsigned char> line_buf;
    MyWindow<5, 5, unsigned char> win_buf;
    
    // 2. 將陣列打碎以達成完全管線化
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

            // 5. 更新 5x5 Window Buffer
            win_buf.shift_right();
            win_buf.insert(line_buf.getval(0, c), 0, 4);
            win_buf.insert(line_buf.getval(1, c), 1, 4);
            win_buf.insert(line_buf.getval(2, c), 2, 4);
            win_buf.insert(line_buf.getval(3, c), 3, 4);
            win_buf.insert(line_buf.getval(4, c), 4, 4);

            // 預設輸出為白底 (素描紙)
            unsigned char out_pixel = 255;
            
            // 6. 5x5 Gaussian Blur 與 Color Dodge 演算法
            if (r >= 4 && c >= 4) {
                // 計算 5x5 高斯模糊 (B)
                int B = 
                  1*(win_buf.getval(0,0)+win_buf.getval(0,4)+win_buf.getval(4,0)+win_buf.getval(4,4)) +
                  4*(win_buf.getval(0,1)+win_buf.getval(0,3)+win_buf.getval(1,0)+win_buf.getval(4,1)+win_buf.getval(4,3)+win_buf.getval(3,0)+win_buf.getval(1,4)+win_buf.getval(3,4)) +
                  6*(win_buf.getval(0,2)+win_buf.getval(4,2)+win_buf.getval(2,0)+win_buf.getval(2,4)) +
                  16*(win_buf.getval(1,1)+win_buf.getval(1,3)+win_buf.getval(3,1)+win_buf.getval(3,3)) +
                  24*(win_buf.getval(1,2)+win_buf.getval(2,1)+win_buf.getval(3,2)+win_buf.getval(2,3)) +
                  36*win_buf.getval(2,2);
                
                B = B >> 8; // 總權重 256，無損除法
                
                // 原圖中心畫素 (A)
                int A = win_buf.getval(2,2);
                
                // 顏色加亮 (Color Dodge) 素描演算法: Result = min(255, (A * 256) / (B + 1))
                int res = (A << 8) / (B + 1);
                out_pixel = (res > 255) ? 255 : res;
            }

            // 7. 將計算結果寫回外部 DDR
            dst_mem[r * cols + c] = out_pixel;
        }
    }
}
