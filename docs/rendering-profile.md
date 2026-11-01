# 大棋盘渲染 profile

环境：aarch64 8 核、Qt 5.15.13、Release/Debug 混合验证、`QT_QPA_PLATFORM=offscreen`。
棋盘 1500×1000，`BoardView` 1000×700。

## 结果（无其他编译负载时）

| 项目 | 结果 |
|---|---:|
| `BoardView::grab()` | 约 **9.0 ms/帧** |
| `renderBoardImage()`（导出） | 约 **45 ms/张** |
| 串行 `computeNext` | 约 **34.8 ms/代**（Debug） |
| 8 线程异步单代 | 约 **12–24 ms/代**（受机器负载影响） |

Callgrind 指令占比：

- QtGui 图像缩放/合成：约 **70–78%**
- `BoardView::drawCellsAsImage` 逐格填图：约 **9.5%**

## 位存储微基准

同一 150 万格、ARGB 写图循环：

```text
byte 0.214 ms/frame
bit  1.662 ms/frame   // 约 7.8× 慢
```

`QImage::Format_Mono` 1bpp 源图 vs ARGB 源图（同样缩放）：

```text
ARGB source scale 0.433 ms/frame
Mono  source scale 1.767 ms/frame   // 约 4× 慢
```

结论：

- 位存储的主要收益是内存（活棋盘 1.5 MB → 187.5 KB，快照 96 MB → 12 MB）；
- 它不会改善 Qt 的缩放/合成成本，朴素位渲染反而更慢；
- 因此活棋盘和渲染保持 1 byte/cell，只把历史快照改为 `PackedGrid`；
- 渲染侧按屏幕刷新率自适应：屏幕刷新率 ≥ 当前代速时逐代刷新，不足时才合并；速度上限 120 代/秒。
- 只绘制与窗口相交的可见 cell 范围，缩放/平移时不再扫描整张棋盘。

## 说明

- `grab()` 是 offscreen 路径，包含 QPixmap 分配/拷贝，绝对毫秒会受桌面合成和后端影响；
  这里用于比较路径成本，而不是宣称屏幕上的固定帧时。
- 绝对时间受并发编译负载影响明显，结论主要依据指令占比和同条件微基准比例。
