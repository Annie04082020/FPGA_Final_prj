# FPGA 專題簡報講稿（修改版）

## Slide 1 封面

各位教授、助教、同學大家好，我們是第七組。
今天要報告的題目是「邊緣特徵提取與素描風格即時轉換」。

我們的專題主要是在 Zynq FPGA 平台上，實作一個即時影像素描化的硬體加速系統，希望能提升影像處理速度，降低 CPU 的負擔。

---

## Slide 2 專案動機與目標

現在很多影像濾鏡，例如模糊、邊緣強化或風格轉換，如果直接用 CPU 做即時處理，通常會有運算量大、記憶體存取頻繁的問題。

尤其像高斯模糊這種卷積運算，在高解析度影像下會比較吃效能。

因此我們希望利用 FPGA 的平行化特性，設計一個影像處理加速器，目標是達成 II = 1，也就是每個 clock 都能持續輸入新的 pixel，提高整體吞吐量。

---

## Slide 3 演算法分析與設計

一開始我們先實作 Sobel Edge Detection。

Sobel 的優點是硬體成本低，而且容易實作，但實際測試後發現，產生的線條比較生硬，比較像單純邊緣偵測，不太有素描的感覺。

後來我們改成使用 Gaussian Blur 搭配 Color Dodge 的方式。

先對影像做模糊，再利用 Color Dodge 進行混合，讓亮部被保留下來、邊緣被強化，最後得到比較接近鉛筆素描的效果。

---

## Slide 4 演算法矩陣比較

左邊是 Sobel 使用的 3×3 kernel，右邊則是我們最後採用的 5×5 Gaussian kernel。

這邊有一個硬體設計上的重點：
我們把 Gaussian kernel 的權重總和設計成 256。

因為 256 剛好是 2 的 8 次方，所以最後做 normalization 的時候，可以直接用 bit shift 取代除法器，降低硬體資源消耗。

---

## Slide 5 硬體架構最佳化

在硬體實作上，我們主要做了兩個最佳化。

第一個是記憶體存取的最佳化。
我們使用 Line Buffer 和 Window Buffer，避免每次卷積都重新從 DDR 讀大量資料。

第二個是運算最佳化。
像剛剛提到的 Gaussian normalization，我們直接使用右移運算取代除法。

另外在 Color Dodge 的分母加上 1，避免發生除以 0 的問題。

---

## Slide 6 Line Buffer 與 Window Buffer

這頁是整個系統比較核心的部分。

因為 5×5 卷積一次需要 25 個 pixel，如果全部都直接從 DDR 讀，頻寬會非常大。

所以我們利用 FPGA 內部的 buffer 暫存資料。

每個 clock 只需要輸入一個新的 pixel，剩下的資料由 Line Buffer 與 Window Buffer 自動更新。

這樣在同一個 clock cycle 內，就可以同時取得完整的 5×5 window 來做卷積運算。

這也是後面能做到 II = 1 的重要原因。

---

## Slide 7~10 HLS 合成結果

這幾頁是 Vitis HLS 的 synthesis 與 cosimulation 結果。

可以看到不管是 Sobel 還是 Gaussian 版本，最後都成功達成 II = 1。

代表 pipeline 建立成功後，每個 cycle 都可以持續處理新的 pixel。

另外 cosimulation 也都有通過，代表 RTL 與 C model 的結果一致。

---

## Slide 11 軟硬體協同設計

系統整合部分，我們使用 AXI 匯流排連接 PS 與 PL。

控制訊號由 ARM 端透過 AXI-Lite 傳送。

影像資料則由 AXI Master 直接存取 DDR。

一開始我們有遇到 AXI Crossbar 的位址衝突問題，所以後來把讀取與寫入拆成不同通道。

---

## Slide 12 Block Design

這邊可以看到我們將讀取與寫入分別接到 HP0 與 HP1。

這樣可以降低 AXI 壅塞，同時提升記憶體傳輸效率。

對於連續影像處理來說，頻寬穩定性會比較好。

---

## Slide 13~15 Demo 展示

接下來是實際執行結果。

我們在 PYNQ 平台上測試 720p 影像，單張處理時間大約是 10 毫秒左右。

後面也展示不同的後處理效果，例如素描、漫畫風格以及彩色插畫風格。

其中部分效果是利用 FPGA 輸出的邊緣結果，再搭配軟體端做後處理。

---

## Slide 16 效能分析

在效能方面，720p 影像處理大約可以達到接近 100 FPS。

不過在影片測試時，整體 FPS 並沒有完全達到硬體理論值。

後來分析發現，瓶頸主要不是 FPGA 運算，而是在 Python OpenCV 的影片解碼，以及 CPU 與記憶體之間的資料搬移。

也就是說，目前系統比較偏向受限於軟體端的 overhead。

---

## Slide 17 結論

最後總結一下。

我們成功在 Zynq FPGA 上實作即時影像素描化系統，並透過 pipeline 與 buffer 架構達成 II = 1。

未來如果要再提升即時性，可以進一步導入 VDMA 或 HDMI streaming，減少 CPU 介入，往全硬體串流架構發展。

以上是我們的報告，謝謝大家。
