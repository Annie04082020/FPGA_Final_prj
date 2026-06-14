#include <iostream>
#include <vector>
#include "sketch_filter.h"

int main() {
    // 預設影像大小 (改為 1080p)
    int rows = 1080;
    int cols = 1920;
    int total_pixels = rows * cols;

    // 配置記憶體給輸入與輸出影像
    std::vector<unsigned char> src_img(total_pixels);
    std::vector<unsigned char> dst_img(total_pixels);

    // 產生簡單的測試資料 (例如：漸層)
    for (int i = 0; i < total_pixels; i++) {
        src_img[i] = i % 256;
    }

    // 清空輸出記憶體
    for (int i = 0; i < total_pixels; i++) {
        dst_img[i] = 0;
    }

    std::cout << "Starting Dummy HLS IP Simulation..." << std::endl;

    // 呼叫 HLS 函數
    sketch_filter(src_img.data(), dst_img.data(), rows, cols);

    std::cout << "Simulation Completed." << std::endl;

    // 驗證輸出結果是否與輸入一致
    bool pass = true;
    for (int i = 0; i < total_pixels; i++) {
        if (dst_img[i] != src_img[i]) {
            std::cout << "Mismatch at index " << i << ": expected " 
                      << (int)src_img[i] << ", got " << (int)dst_img[i] << std::endl;
            pass = false;
            break;
        }
    }

    if (pass) {
        std::cout << "---------------------------------" << std::endl;
        std::cout << "Test Passed! The dummy IP correctly copies data." << std::endl;
        std::cout << "---------------------------------" << std::endl;
        return 0; // 成功
    } else {
        std::cout << "---------------------------------" << std::endl;
        std::cout << "Test Failed!" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        return 1; // 失敗
    }
}
