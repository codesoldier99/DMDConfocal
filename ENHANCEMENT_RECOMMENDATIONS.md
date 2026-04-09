# 将 DMDConfocal 核心优势移植到 DAC-3D System v3.0 的增强建议

## 两个项目的对比概览

| 维度 | DMDConfocal (C++/MFC) | DAC-3D v3.0 (Python/PyQt5) |
|------|----------------------|---------------------------|
| 语言 | C++ 原生编译 | Python 解释执行 |
| 图像采集 | 事件驱动 + SetEvent/WaitForSingleObject | 轮询 + time.sleep(0.01) |
| 数据传递 | RenderChain 零拷贝指针传递 | Dict[str, List[NDArray]] 多层拷贝 |
| 内存管理 | 缓冲区复用 + 尺寸检测 | 每帧 new numpy array |
| 线程模型 | 手动线程 + Win32 Event 精确同步 | ThreadPoolExecutor + GIL 竞争 |
| 文件保存 | 异步线程 + 专用缓冲区 | 扫描完成后批量保存 |
| 实时显示 | GDI+/OpenGL 双缓冲 30ms 刷新 | PyQt5 无实时图像预览 |
| DMD 控制 | 完整的 WLP/ALP 适配器 | 仅接口定义，未实现 |

---

## 建议一：引入 RenderChain 模式 — 流式处理管线

### 问题
DAC-3D 当前的 `scan_service.py` 采用**批量采集-批量处理**模式：先将所有图像收集到 `scan_data` 字典中，扫描完成后才开始处理。这导致：
- 内存峰值巨大（3 相机 × 2048×2048 × uint16 × 数百帧 = 数 GB）
- 处理延迟等于扫描时间 + 处理时间（串行）

### DMDConfocal 的做法
RenderChain（责任链）让每一帧在采集后**立即**流过整条处理链：

```
Camera → memcpy → SetEvent → ProcessThread → RenderChain → Display
```

### 建议实现

```python
# dac3d/core/render_chain.py (新增)

from abc import ABC, abstractmethod
from typing import List
import numpy as np

class FrameProcessor(ABC):
    """DMDConfocal HVideoRender 的 Python 等价物"""
    @abstractmethod
    def process(self, frame: np.ndarray, metadata: dict) -> None:
        pass

class RenderChain(FrameProcessor):
    """DMDConfocal HVideoRenderChain 的 Python 等价物
    同时是处理器和处理器容器，支持运行时热插拔"""

    def __init__(self):
        self._processors: List[FrameProcessor] = []

    def add(self, processor: FrameProcessor) -> None:
        if processor not in self._processors:
            self._processors.append(processor)

    def remove(self, processor: FrameProcessor) -> None:
        self._processors.remove(processor)

    def process(self, frame: np.ndarray, metadata: dict) -> None:
        for proc in self._processors:
            proc.process(frame, metadata)

# 使用示例：
# chain = RenderChain()
# chain.add(ConfocalMergeProcessor())   # 共聚焦合并
# chain.add(LiveDisplayProcessor())     # 实时显示
# chain.add(DiskSaverProcessor())       # 异步保存
#
# 每帧到达时：
# chain.process(frame, {"camera_id": "cam0", "index": i})
```

### 修改 scan_service.py

```python
# 在 _perform_scan() 的每行采集后立即处理，而非累积
for row_idx in range(n_rows):
    images = self._cameras.grab_sequence_all(n_frames=n_points_x)
    for cam_id, frames in images.items():
        for frame in frames:
            self._render_chains[cam_id].process(frame, {
                "row": row_idx, "camera": cam_id
            })
    # 帧已流式处理，无需 scan_data.extend()
```

**预期收益**：内存峰值降低 10-100 倍，处理延迟与采集完全重叠。

---

## 建议二：事件驱动替代轮询 — 消除 sleep 等待

### 问题
DAC-3D 中有大量的轮询等待模式：

```python
# scan_service.py line 298-299
while self._stage_xy.is_moving:
    time.sleep(0.01)  # 10ms 轮询，最大延迟 10ms

# event_bus.py line 111
event = self._event_queue.get(timeout=0.1)  # 100ms 轮询
```

### DMDConfocal 的做法
使用 Windows Event 实现零延迟精确同步：

```cpp
// C3DScanner.cpp: 零轮询等待
m_AxisZ->MoveTo(m_Axis_Next, m_ScanSpeed);
WaitForSingleObject(m_hMoveEvent, 10000);  // 阻塞到事件触发，零 CPU 开销
ResetEvent(m_hMoveEvent);
```

### 建议实现

```python
# 使用 threading.Event 替代 sleep 轮询

import threading

class ZMotionStageEnhanced(ZMotionStage):
    def __init__(self, ...):
        super().__init__(...)
        self._move_complete = threading.Event()

    def move_to(self, x: float, y: float, velocity: float = None):
        self._move_complete.clear()
        super().move_to(x, y, velocity)
        # 启动监控线程（或硬件中断回调）
        threading.Thread(target=self._wait_motion_done, daemon=True).start()

    def _wait_motion_done(self):
        """用硬件状态轮询替代业务层轮询，将延迟隔离在驱动层"""
        while self.is_moving:
            time.sleep(0.001)  # 驱动层 1ms 精度
        self._move_complete.set()

    def wait_for_move(self, timeout: float = 10.0) -> bool:
        """业务层零轮询等待"""
        return self._move_complete.wait(timeout=timeout)
```

```python
# scan_service.py 改造
self._stage_xy.move_to(target_x, target_y)
if not self._stage_xy.wait_for_move(timeout=10.0):
    raise TimeoutError("Stage move timeout!")
# 精确在到位后立即继续，零延迟
```

**对 EventBus 同样改造**：

```python
# event_bus.py: 用 Event 替代 timeout 轮询
def _worker(self):
    while self._running:
        event = self._event_queue.get()  # 无 timeout，阻塞到有事件
        self._dispatch(event)
```

**预期收益**：运动同步延迟从 ±10ms 降至 <1ms，EventBus 响应从 100ms 降至 <1ms。

---

## 建议三：引入缓冲区复用 — 消除帧级内存分配

### 问题
当前每帧采集都创建新的 numpy 数组：

```python
# basler_camera.py line 362-363
image = self._converter.Convert(grab_result)
img_array = image.GetArray()  # 每帧 new 一个 8MB 数组 (2048×2048×uint16)
```

### DMDConfocal 的做法

```cpp
// ConfocalMerge.cpp:192 — 仅在尺寸变化时分配
if (m_imgsize != pHeader->Vsize) {
    m_bmpResult = (LPBYTE)malloc(m_imgsize);
    m_bmpTemp = (LPBYTE)malloc(m_imgsize);
}
memcpy(m_bmpTemp, pBuffer, m_imgsize);  // 复用缓冲区
```

### 建议实现

```python
# dac3d/core/frame_buffer_pool.py (新增)

import numpy as np
from collections import deque
import threading

class FrameBufferPool:
    """预分配帧缓冲区池，借鉴 DMDConfocal 的缓冲区复用策略"""

    def __init__(self, shape: tuple, dtype=np.uint16, pool_size: int = 8):
        self._shape = shape
        self._dtype = dtype
        self._lock = threading.Lock()
        # 预分配 pool_size 个缓冲区
        self._available = deque(
            np.empty(shape, dtype=dtype) for _ in range(pool_size)
        )
        self._in_use = set()

    def acquire(self) -> np.ndarray:
        """获取一个缓冲区（零分配）"""
        with self._lock:
            if self._available:
                buf = self._available.popleft()
                self._in_use.add(id(buf))
                return buf
        # 池耗尽时才分配新缓冲区
        buf = np.empty(self._shape, dtype=self._dtype)
        with self._lock:
            self._in_use.add(id(buf))
        return buf

    def release(self, buf: np.ndarray) -> None:
        """归还缓冲区"""
        with self._lock:
            self._in_use.discard(id(buf))
            self._available.append(buf)

# 使用：
# pool = FrameBufferPool((2048, 2048), np.uint16, pool_size=8)
# buf = pool.acquire()
# np.copyto(buf, grabbed_frame)  # 零分配拷贝
# ... process buf ...
# pool.release(buf)
```

**预期收益**：消除每帧 8MB 的 malloc/free 开销，减少 GC 压力，提升采集吞吐率约 20%。

---

## 建议四：异步文件保存 — 处理与 I/O 解耦

### 问题
DAC-3D 当前在扫描完成后才保存，或在扫描循环中同步保存，I/O 阻塞处理流水线。

### DMDConfocal 的做法

```cpp
// ConfocalMerge.cpp:142-143 — 拷贝到专用缓冲区后立即启动保存线程
memcpy(m_SaveTemp, m_bmpResult, m_imgsize);
m_hSaveThread = CreateThread(NULL, 0, SaveProc, this, 0, 0);
// 处理线程继续下一帧，不等待保存完成
```

### 建议实现

```python
# dac3d/services/async_saver.py (新增)

import queue
import threading
import numpy as np
from pathlib import Path

class AsyncFrameSaver:
    """异步帧保存器，借鉴 DMDConfocal 的保存线程策略"""

    def __init__(self, max_pending: int = 32):
        self._queue = queue.Queue(maxsize=max_pending)
        self._running = True
        self._thread = threading.Thread(target=self._save_worker, daemon=True)
        self._thread.start()

    def save(self, frame: np.ndarray, path: Path) -> None:
        """非阻塞提交保存任务"""
        # 拷贝到专用缓冲区，调用者可立即复用原缓冲区
        frame_copy = frame.copy()
        self._queue.put((frame_copy, path))

    def _save_worker(self):
        while self._running:
            try:
                frame, path = self._queue.get(timeout=1.0)
                np.save(path, frame)  # 或 tifffile.imwrite
                self._queue.task_done()
            except queue.Empty:
                continue

    def flush(self):
        """等待所有挂起的保存完成"""
        self._queue.join()

    def shutdown(self):
        self._running = False
        self._thread.join()
```

**预期收益**：消除保存 I/O 对扫描流水线的阻塞，扫描速度提升 10-30%（取决于帧保存比例）。

---

## 建议五：像素级 NumPy 向量化 + Cython 热点优化

### 问题
DAC-3D 的图像处理尚未实现核心算法。未来实现时如果用纯 Python 循环处理像素，性能会极差。

### DMDConfocal 的做法
指针级操作，一维化遍历，分支外提：

```cpp
void Img8_Max(LPBYTE pDest, LPBYTE pScr, long IWidth, long IHeight, int Thresh) {
    long PixelCount = IWidth * IHeight;  // 一维化
    for (long i = 0; i < PixelCount; i++) {
        if (*Index_Scr > *Index_Dest) *Index_Dest = *Index_Scr;  // 就地操作
        Index_Scr++; Index_Dest++;
    }
}
```

### 建议实现（三级优化策略）

**Level 1: NumPy 向量化（最简单，性能 ~50x vs 纯 Python）**

```python
# dac3d/processing/image_ops.py

import numpy as np

def img_max(dest: np.ndarray, src: np.ndarray, thresh: int = 0) -> None:
    """DMDConfocal Img8_Max 的 NumPy 等价实现，就地操作"""
    if thresh > 0:
        src_adj = np.clip(src.astype(np.int16) - thresh, 0, 255).astype(np.uint8)
        np.maximum(dest, src_adj, out=dest)  # out=dest 实现就地操作
    else:
        np.maximum(dest, src, out=dest)

def reconstruct_3phase(imgs: list, merge: np.ndarray) -> None:
    """DMDConfocal ReconstructImage 的 NumPy 等价实现"""
    a, b, c = imgs[0].astype(np.float64), imgs[1].astype(np.float64), imgs[2].astype(np.float64)
    s = 2 * b - a - c
    t = c - a
    np.sqrt(s * s + t * t, out=s)  # 复用 s 的内存
    s /= 2
    np.clip(s, 0, 255, out=s)
    merge[:] = s.astype(np.uint8)

def is_dark_image(img: np.ndarray, threshold: float = 3.0) -> bool:
    """单遍方差检测"""
    return np.std(img) < threshold
```

**Level 2: Numba JIT（更快，性能 ~200x vs 纯 Python）**

```python
from numba import njit, prange

@njit(parallel=True)
def img_max_numba(dest, src, thresh=0):
    """Numba 并行化版本，接近 C++ 性能"""
    for i in prange(dest.size):
        s = max(src.flat[i] - thresh, 0) if thresh > 0 else src.flat[i]
        if s > dest.flat[i]:
            dest.flat[i] = s
```

**Level 3: Cython 扩展（极限性能，等价于 C++）**

```cython
# dac3d/processing/_fast_ops.pyx
cimport cython
from libc.math cimport sqrt

@cython.boundscheck(False)
@cython.wraparound(False)
def img_max_c(unsigned char[:] dest, unsigned char[:] src, int thresh):
    cdef int i, n = dest.shape[0]
    cdef int s, d
    for i in range(n):
        if thresh > 0:
            s = src[i] - thresh
            if s < 0: s = 0
            if s > dest[i]: dest[i] = s
        else:
            if src[i] > dest[i]: dest[i] = src[i]
```

**预期收益**：NumPy 向量化即可满足大多数实时需求；Numba/Cython 用于极端性能场景。

---

## 建议六：共聚焦合并的流式累积器

### 问题
DAC-3D 尚未实现共聚焦合并（最大值投影）。未来实现时若采用"收集所有帧再处理"的模式，内存占用线性增长。

### DMDConfocal 的做法
流式累积，恒定内存：

```cpp
// 每帧到达后立即与结果合并
Img8_Max(m_bmpResult, m_bmpTemp, iWidth, iHeight, m_iBackTh);
// 仅需 2 个缓冲区，无论总帧数多少
```

### 建议实现

```python
# dac3d/processing/confocal_merge.py (新增)

import numpy as np
import threading
from .image_ops import img_max, is_dark_image

class ConfocalMergeProcessor(FrameProcessor):
    """流式共聚焦合并器，借鉴 DMDConfocal CConfocalMerge"""

    def __init__(self, merge_count: int, background_thresh: int = 0):
        self._merge_count = merge_count
        self._bg_thresh = background_thresh
        self._result = None        # 累积结果缓冲区（常驻）
        self._current_index = 0
        self._event = threading.Event()
        self._on_complete = None

    def process(self, frame: np.ndarray, metadata: dict) -> None:
        # 懒初始化（仅在尺寸变化时分配）
        if self._result is None or self._result.shape != frame.shape:
            self._result = np.zeros_like(frame)

        # 暗图跳过
        if is_dark_image(frame):
            return

        # 流式最大值投影 — 恒定内存
        img_max(self._result, frame, self._bg_thresh)
        self._current_index += 1

        if self._current_index >= self._merge_count:
            if self._on_complete:
                self._on_complete(self._result.copy())
            self._reset()

    def _reset(self):
        if self._result is not None:
            self._result.fill(0)
        self._current_index = 0
```

**预期收益**：内存占用从 O(N×帧大小) 降至 O(2×帧大小)，支持无限帧数的合并。

---

## 建议七：完善 DMD 驱动实现

### 问题
DAC-3D 已定义 `IDMD` 接口但未实现。DMDConfocal 有完整的 WLP/ALP 适配器可供参考。

### DMDConfocal 的关键实现（可移植）

```
WlpAdapter:
  - OpenDev() → 设备初始化 + DDR 内存检测
  - DownloadImage() → 二进制图案批量下载
  - SetTrigger() → 触发模式配置
  - Start()/Stop() → 图案序列播放控制

PatternLine/PatternPoint:
  - 线型/点阵图案生成器
  - Binary2Gray()/Bit2Byte() 格式转换
  - 帧数预计算: FrameCount = Period × Period
```

### 建议实现

```python
# dac3d/hal/dmd/wlp_dmd.py (新增)

import ctypes
import numpy as np
from ..interfaces import IDMD

class WlpDMD(IDMD):
    """WLP DMD 驱动，移植自 DMDConfocal WlpAdapter"""

    def __init__(self, dll_path: str = "WlpDMDApi.dll"):
        self._dll = ctypes.windll.LoadLibrary(dll_path)
        self._handle = None
        self._frame_count = 0

    def connect(self) -> bool:
        self._handle = self._dll.WLP_OpenDev(0)
        if self._handle:
            self._dll.WLP_DevReset(self._handle)
            self._dll.WLP_DMDReset(self._handle)
            return True
        return False

    def load_pattern_sequence(self, patterns: list) -> bool:
        """批量下载二进制图案到 DMD 板载 DDR"""
        for i, pattern in enumerate(patterns):
            binary = self._pack_binary(pattern)  # 8bit → 1bit 压缩
            self._dll.WLP_DownloadImage(self._handle, binary, len(binary), i)
        self._frame_count = len(patterns)
        return True

    def start(self, trigger_mode: str = "loop") -> bool:
        mode_map = {"single": 0, "sequence": 1, "loop": 3}
        self._dll.WLP_SetTrigger(self._handle, mode_map.get(trigger_mode, 3))
        self._dll.WLP_Start(self._handle)
        return True

    def _pack_binary(self, gray_pattern: np.ndarray) -> bytes:
        """灰度图转 1-bit 二进制，内存节省 8 倍"""
        binary = np.packbits(gray_pattern > 127)
        return binary.tobytes()
```

---

## 建议八：双引擎实时显示

### 问题
DAC-3D 的 UI 当前没有实时图像预览，只有文字进度条。

### DMDConfocal 的做法
- GDI+ 2D 预览 + 256 色调色板，30ms 定时刷新
- OpenGL 3D 高度图，双缓冲渲染

### 建议实现

```python
# ui/live_view_widget.py (新增)

from PyQt5.QtWidgets import QWidget, QLabel, QVBoxLayout
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtCore import QTimer
import numpy as np

class LiveViewWidget(QWidget, FrameProcessor):
    """实时图像预览，借鉴 DMDConfocal VideoView 的定时刷新策略"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self._label = QLabel()
        layout = QVBoxLayout(self)
        layout.addWidget(self._label)

        self._latest_frame = None
        self._frame_lock = threading.Lock()

        # 30ms 定时刷新 (~33 FPS)，与 DMDConfocal 一致
        self._timer = QTimer()
        self._timer.timeout.connect(self._refresh)
        self._timer.start(30)

    def process(self, frame: np.ndarray, metadata: dict) -> None:
        """接收帧数据（从采集线程调用）"""
        with self._frame_lock:
            self._latest_frame = frame  # 仅保留最新帧，跳帧显示

    def _refresh(self):
        """定时器回调，在 UI 线程中刷新显示"""
        with self._frame_lock:
            frame = self._latest_frame
            self._latest_frame = None

        if frame is None:
            return

        # uint16 → uint8 映射显示
        if frame.dtype == np.uint16:
            display = (frame >> 8).astype(np.uint8)
        else:
            display = frame

        h, w = display.shape[:2]
        qimg = QImage(display.data, w, h, w, QImage.Format_Grayscale8)
        self._label.setPixmap(QPixmap.fromImage(qimg).scaled(
            self._label.size(), aspectRatioMode=1))
```

**预期收益**：扫描过程中实时查看采集图像，极大改善用户体验。

---

## 实施优先级建议

| 优先级 | 建议 | 复杂度 | 性能收益 | 说明 |
|--------|------|--------|---------|------|
| **P0** | 建议二：事件驱动替代轮询 | 低 | 10ms→<1ms 延迟 | 改动最小，收益最直接 |
| **P0** | 建议八：实时图像预览 | 低 | 用户体验质变 | 独立模块，不影响现有代码 |
| **P1** | 建议一：RenderChain 流式管线 | 中 | 内存降 10-100 倍 | 架构级改造，需重构 scan_service |
| **P1** | 建议四：异步文件保存 | 低 | 扫描提速 10-30% | 独立组件，可渐进集成 |
| **P2** | 建议三：帧缓冲区池 | 中 | 采集提速 ~20% | 需修改相机驱动层 |
| **P2** | 建议五：NumPy 向量化算法 | 中 | 处理 50-200x 加速 | 新增模块，依赖算法需求 |
| **P2** | 建议六：流式共聚焦合并 | 中 | 内存 O(N)→O(1) | 依赖建议一 |
| **P3** | 建议七：DMD 驱动实现 | 高 | 功能补全 | 依赖硬件，需实际联调 |

---

## 总结

DAC-3D v3.0 拥有优秀的分层架构（5 层）和清晰的接口抽象，但在**数据流效率**上有显著提升空间。DMDConfocal 的核心优势——**零拷贝渲染链、事件驱动同步、缓冲区复用、流式累积处理、异步 I/O 分离**——可以系统性地移植到 Python 技术栈中。

关键原则：**不是把 DAC-3D 改写成 C++，而是将 DMDConfocal 的设计思想用 Python 惯用方式表达**。NumPy 向量化 + threading.Event + 缓冲区池 + 异步保存，足以在 Python 中实现接近 C++ 的数据流效率。
