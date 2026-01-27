这是一个为您编写的 WebP XMP 元数据读写实现指南。这篇文档旨在帮助对 WebP 格式尚不熟悉的 C/C++ 开发者快速上手。

# WebP XMP 元数据读写实现指南

## 1. 简介
本主要说明如何在 C/C++ 环境下实现对 WebP 图片格式中 XMP（Extensible Metadata Platform）元数据的读取和写入。

**核心概念：**
*   **WebP**:Google 开发的一种现代图像格式，基于 RIFF（Resource Interchange File Format）容器格式。
*   **XMP**: Adobe 定义的基于 XML 的元数据标准。
*   **目标**: 在 WebP 文件中，XMP 数据被封装在一个特定的“Chunk（块）”中。我们的任务是解析文件结构，找到或插入这个块。

---

## 2. WebP 文件格式基础 (RIFF)

WebP 文件是基于 **RIFF** 容器的。这意味着文件由一个文件头和一系列的数据块（Chunks）组成。

### 2.1 文件头结构 (12 字节)
所有 WebP 文件都以一个标准的 RIFF 头开始。数据均采用 **小端序 (Little-Endian)** 存储（这与 x86/x64 的 C/C++ 内存布局一致，非常方便）。

| 偏移量 | 长度 | 类型 | 内容 / 描述 |
| :--- | :--- | :--- | :--- |
| 0 | 4 | FourCC | ASCII 字符 `'R' 'I' 'F' 'F'` |
| 4 | 4 | uint32 | **File Size** (文件总大小 - 8 字节) |
| 8 | 4 | FourCC | ASCII 字符 `'W' 'E' 'B' 'P'` |

### 2.2 Chunk (数据块) 通用结构
在文件头之后，紧跟着一系列的 Chunk。每个 Chunk 都有相同的头部结构：

| 偏移量 | 长度 | 类型 | 描述 |
| :--- | :--- | :--- | :--- |
| 0 | 4 | FourCC | 块的标识符 (例如 `'V' 'P' '8' ' '` 或 `'X' 'M' 'P' ' '`) |
| 4 | 4 | uint32 | **Chunk Size** (仅 Payload 的大小，不含这8字节头) |
| 8 | *Size* | Bytes | **Payload** (实际数据) |

**重要规则 - 字节对齐 (Padding)：**
*   RIFF 标准要求每个 Chunk 的大小必须是**偶数**。
*   如果 `Chunk Size` 是奇数，Payload 后面必须跟一个 `0x00` 的填充字节。
*   **注意**：这个填充字节**不**计入 `Chunk Size` 字段中，但读取下一个 Chunk 时必须跳过它。

---

## 3. WebP 的两种形态与 XMP

这是实现中最关键的逻辑部分。WebP 有“简单格式”和“扩展格式”。

### 3.1 简单格式 (Simple Format)
只包含图像数据（VP8 块）。**标准规定：简单格式不支持 XMP 元数据。**

### 3.2 扩展格式 (Extended Format)
如果你想给 WebP 添加 XMP，该文件必须是（或被转换为）**扩展格式**。
扩展格式的特征是：在 `WEBP` 签名之后，第一个 Chunk 必须是 `VP8X`。

**`VP8X` 块结构 (Header):**
用来标记文件包含哪些扩展特性（如动画、透明度、ICC 配置、XMP 等）。

```c
struct VP8X_Chunk_Payload {
    uint8_t flags_0; // 包含保留位和标志位
    uint8_t reserved[3]; // 24位保留空间
    uint8_t canvas_width[3]; // 画布宽度
    uint8_t canvas_height[3]; // 画布高度
};
```

**`flags_0` 的位定义 (从高位到低位):**
*   Bit 7: Reserved
*   Bit 6: Reserved
*   Bit 5: **ICC** (是否有 ICC Profile)
*   Bit 4: **Alpha** (是否有透明通道)
*   Bit 3: **Exif** (是否有 Exif)
*   Bit 2: **XMP** (是否有 XMP) -> **我们要操作这个位**
*   Bit 1: Animation
*   Bit 0: Reserved

### 3.3 Chunk 的排列顺序
WebP 扩展格式对 Chunk 的顺序有严格要求。写入时必须遵守，否则文件损坏：
1.  `VP8X` (必须在第一位)
2.  `ICCP` (颜色配置)
3.  `ANIM` (动画控制)
4.  `VP8` / `VP8L` (图像数据)
5.  `EXIF` (Exif 信息)
6.  `XMP ` (XMP 信息) -> **必须放在图像数据和 Exif 之后**

---

## 4. XMP Chunk 详解

*   **FourCC**: `'X' 'M' 'P' ' '` (注意第四个字符是空格，Hex: `58 4D 50 20`)
*   **Payload**: XMP 内容是一个完整的 XML 字符串（通常以 `<x:xmpmeta ...>` 开头）。
*   **无需**在 XML 字符串末尾添加 null 终止符（`\0`），除非 XML 本身包含它。

---

## 5. C/C++ 实现伪代码

### 5.1 定义基础结构

```cpp
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 小端序读取辅助
#define FOURCC(a,b,c,d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

const uint32_t CHUNK_VP8X = FOURCC('V', 'P', '8', 'X');
const uint32_t CHUNK_XMP  = FOURCC('X', 'M', 'P', ' ');
const uint32_t TAG_RIFF   = FOURCC('R', 'I', 'F', 'F');
const uint32_t TAG_WEBP   = FOURCC('W', 'E', 'B', 'P');

typedef struct {
    uint32_t chunk_id;
    uint32_t size;
} ChunkHeader;
```

### 5.2 读取流程 (Reader)

读取相对简单，只需要遍历文件找到 `XMP ` 块。

```cpp
// 伪代码：读取 WebP 中的 XMP
char* ReadWebPXMP(const char* filepath, size_t* out_len) {
    FILE* fp = fopen(filepath, "rb");
    
    // 1. 验证 RIFF 头
    uint32_t riff_tag, file_size, webp_tag;
    fread(&riff_tag, 4, 1, fp);
    fread(&file_size, 4, 1, fp);
    fread(&webp_tag, 4, 1, fp);
    
    if (riff_tag != TAG_RIFF || webp_tag != TAG_WEBP) return NULL;

    ChunkHeader header;
    while (fread(&header, sizeof(ChunkHeader), 1, fp) == 1) {
        // 2. 检查是否是 XMP 块
        if (header.chunk_id == CHUNK_XMP) {
            char* xmp_data = (char*)malloc(header.size + 1);
            fread(xmp_data, 1, header.size, fp);
            xmp_data[header.size] = '\0'; // 方便当作字符串处理
            *out_len = header.size;
            fclose(fp);
            return xmp_data;
        }

        // 3. 跳过当前块
        fseek(fp, header.size, SEEK_CUR);

        // 4. 处理 Padding (如果大小是奇数，多跳过1字节)
        if (header.size % 2 != 0) {
            fseek(fp, 1, SEEK_CUR);
        }
    }

    fclose(fp);
    return NULL; // 未找到 XMP
}
```

### 5.3 写入流程 (Writer)

写入比较复杂，因为不能直接追加。如果原文件是“简单格式”，必须升级为“扩展格式”。这里展示一个**简化版的核心逻辑**（假设内存足够大，将文件读入内存重组）。

**算法步骤：**
1.  读取原文件所有数据。
2.  检查是否存在 `VP8X` 块。
    *   **情况 A (存在 VP8X)**: 修改 `VP8X` 中的 Flags，将 XMP 位置 1。
    *   **情况 B (不存在 VP8X)**: 这是一个简单格式 (VP8/VP8L)。你需要创建一个新的 `VP8X` 块，根据原图像宽高填充，并设置 XMP 标志位，插入到文件最前面。
3.  按顺序写入新文件：
    *   `RIFF` 头
    *   `VP8X` 块
    *   其他块 (ICCP, ANIM 等)
    *   图像数据块 (VP8/VP8L)
    *   Exif 块 (如果有)
    *   **新的 XMP 块**
4.  修正文件开头的 `File Size`。

```cpp
// 伪代码：写入 XMP 到 WebP
bool WriteWebPXMP(const char* infile, const char* outfile, const char* xmp_data, size_t xmp_len) {
    // 省略具体的文件读取代码，假设我们已经解析了原文件的 chunks 列表
    
    bool has_vp8x = false;
    // ... 检查原文件是否有 VP8X ...

    FILE* fp_out = fopen(outfile, "wb");
    
    // 1. 预留 RIFF 头位置
    fseek(fp_out, 12, SEEK_SET);

    // 2. 写入/处理 VP8X
    if (has_vp8x) {
        // 读取原有 VP8X，设置 XMP bit (Bit 2)
        // vp8x_data[0] |= 0x04; 
        // fwrite(...)
    } else {
        // 创建新的 VP8X
        // 解析原 VP8/VP8L 头获取宽高
        // 构造 VP8X payload, 设置 flags = 0x04 (只包含 XMP)
        // fwrite(VP8X header + payload)
    }

    // 3. 复制原有 Chunk (ICCP, ANIM, VP8/VP8L, EXIF)
    // 注意：如果是覆盖原有 XMP，则跳过原有的 XMP 块不写
    // 注意：必须严格按照顺序复制
    
    // 4. 写入新的 XMP Chunk
    ChunkHeader xmp_header;
    xmp_header.chunk_id = CHUNK_XMP;
    xmp_header.size = (uint32_t)xmp_len;
    
    fwrite(&xmp_header, sizeof(ChunkHeader), 1, fp_out);
    fwrite(xmp_data, 1, xmp_len, fp_out);
    
    // 处理 XMP 的 Padding
    if (xmp_len % 2 != 0) {
        fputc(0x00, fp_out);
    }

    // 5. 回填 RIFF 总大小
    uint32_t total_size = ftell(fp_out) - 8;
    fseek(fp_out, 4, SEEK_SET);
    fwrite(&total_size, 4, 1, fp_out);

    fclose(fp_out);
    return true;
}
```

## 6. 避坑指南 (Checklist)

1.  **Padding 陷阱**: 这是最容易出错的地方。**任何** Chunk（包括 XMP, VP8, VP8X），只要 `Size` 是奇数，写入文件时末尾必须加 `0x00`。读取时必须跳过这个字节。
2.  **VP8X 标志位**: 不要忘记在 `VP8X` chunk 的 payload 第一个字节中设置 `Metadata` 位（Bit 2），否则某些看图软件会忽略后面的 XMP 块。
3.  **Chunk 顺序**: 绝对不要把 `XMP` 块放到 `VP8/VP8L` 图像块的前面。
4.  **小端序**: 在手动组装 `uint32` 或 `uint16` 时，确保遵循 Little-Endian（低字节在前）。
5.  **文件总长度**: RIFF 头中的 file size = 文件总字节数 - 8。写完所有数据后务必更新它。

---
