# 進階 FPGA 期末專案：即時鉛筆素描風硬體加速器 (Sketch Effect)

本專案利用 Xilinx Vitis HLS 設計並合成一個可將灰階影像即時轉換為「鉛筆素描風格」的硬體加速器 IP (`sketch_filter`)。結合 Line Buffer 與 Window Buffer 架構，硬體最高可支援至 1080p 解析度，並達成每個時脈週期處理一個畫素 (Initiation Interval, II = 1) 的極致吞吐量。

## 目錄結構說明

專案目錄下包含兩個版本的 HLS 程式碼：
*   **`hls_real/`**：最終完美版（Day 2 & Day 3 進度），包含完整的管線化優化與邊緣運算邏輯。**請使用此版本的程式碼進行合成與上板。**
*   **`hls_dummy/`**：初版空殼 IP（Day 1 進度），僅包含 AXI 介面宣告與單純的記憶體複製，用來給系統端 (PS 端) 提早進行 PYNQ 環境建置與通訊測試。

---

## 核心檔案說明 (`hls_real/` 資料夾)

### 1. `sketch_filter.cpp` (硬體核心實作)
這是轉換為硬體邏輯的最核心 C++ 程式碼。負責接收影像串流並輸出素描影像。
*   **硬體介面綁定**：利用 `#pragma HLS INTERFACE` 將 `src_mem` 與 `dst_mem` 綁定為 `m_axi` (AXI Master)，使其能直接讀寫 PYNQ 的連續實體記憶體 (CMA)；影像長寬則綁定為 `s_axilite` 供 Python 軟體控制。
*   **記憶體緩衝架構**：使用自訂的 Line Buffer 與 Window Buffer，確保每次運算 $3 \times 3$ 卷積時，只要從外部 DDR 讀取「1個畫素」，大幅降低記憶體頻寬消耗。
*   **硬體極致最佳化**：
    *   `#pragma HLS ARRAY_PARTITION`：將 Window 陣列打碎成暫存器，解決 BRAM 讀取衝突。
    *   `#pragma HLS PIPELINE II=1`：指示編譯器將雙重迴圈完全管線化。
*   **演算法實作**：以移位與加法計算 X 與 Y 方向的 Sobel 邊緣梯度，並實作 `255 - Edge` 達成白紙黑線的鉛筆素描效果；同時具備邊界判斷 (Boundary Handling) 防止邊緣出現雜訊黑框。

### 2. `sketch_filter.h` (標頭檔與結構宣告)
定義了 IP 的函數介面與所需的內部資料結構。
*   **解決版本相容性**：因新版 Vitis HLS 棄用了 `<hls_video.h>`，本標頭檔利用 C++ Template 手工刻出了 `MyLineBuffer` 與 `MyWindow` 結構。
*   **內建 Pragma**：直接在結構體內部封裝了 `shift_up`、`shift_right` 等硬體平移邏輯，並內建 `#pragma HLS unroll`，確保在合成時能被直接展開為平行的硬體線路。

### 3. `sketch_filter_tb.cpp` (測試平台 Testbench)
用於 C Simulation 與 C/RTL Cosimulation (協同模擬) 的測試平台。
*   在軟體端產生了一張 $1920 \times 1080$ 的測試圖像（淺灰背景搭配深灰色正方形）。
*   呼叫 `sketch_filter` 硬體 IP 進行模擬。
*   **自動驗證邏輯**：驗證輸出影像的平坦區是否為全白 (255)，以及正方形邊界處是否正確被偵測出邊緣 (像素值 < 255)，藉此證明硬體邏輯與預期一致。

---

## 如何使用此專案？

### 階段一：Vitis HLS 硬體合成
1. 打開 Vitis HLS，建立新專案，設定 Top Function 為 `sketch_filter`。
2. 將 `hls_real/sketch_filter.cpp` 與 `hls_real/sketch_filter.h` 加入 Source files。
3. 將 `hls_real/sketch_filter_tb.cpp` 加入 Testbench files。
4. 點擊 **Run C Simulation** 驗證邏輯。
5. 點擊 **C Synthesis** 進行硬體合成。
6. 點擊 **Export RTL** 產出 IP `.zip` 壓縮檔。

### 階段二：Vivado 系統接線 (Block Design)
將匯出的 IP 導入 Vivado 並建立 Block Design：
1. 加入 **ZYNQ7 Processing System**，並開啟 **S AXI HP0** 與 **S AXI HP1** 介面（雙通道獨立傳輸效能最佳）。
2. 加入 `sketch_filter` IP。
3. **【避坑指南：解決 Address Overlap】**
   不要讓 Vivado 的自動連線將 `gmem0` 與 `gmem1` 塞進同一個 AXI Interconnect！
   請手動將：
   - `m_axi_gmem0` 直接接上 Zynq 的 `S_AXI_HP0`。
   - `m_axi_gmem1` 直接接上 Zynq 的 `S_AXI_HP1`。
4. 點擊 `Run Connection Automation` 補齊時脈與重置訊號。
5. 產出 HDL Wrapper 並點擊 **Generate Bitstream** 輸出 `.bit` 與 `.hwh` 檔。

### 階段三：PYNQ 上板測試
專案內附帶了完整的 Python 測試腳本：
*   **`pynq/sketch_filter_test.ipynb`**：包含如何讀取圖片、分配連續實體記憶體 (CMA)、透過 AXI-Lite 控制 IP、量測運算時間的完整測試代碼。
*   將 `.bit`、`.hwh` 與這個 `.ipynb` 腳本放入 PYNQ 開發板，即可馬上看到硬體加速的驚人效能！
