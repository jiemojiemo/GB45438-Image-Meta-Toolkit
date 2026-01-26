# PNG XMP 元数据读写技术指南

本文档旨在指导开发者了解如何在 PNG 图片中嵌入和提取 XMP (Extensible Metadata Platform) 信息。

---

## 1. PNG 基础复习：数据块 (Chunks)

PNG 文件由一个 8 字节的头部签名和多个**数据块 (Chunk)** 组成。每个数据块的标准结构如下：

| 长度 (4 bytes) | 类型 (4 bytes) | 数据内容 (N bytes) | CRC 校验 (4 bytes) |
| :--- | :--- | :--- | :--- |
| `length` | `type` (如 IHDR, iTXt) | `data` | 对 type+data 的 CRC32 |

---

## 2. XMP 存放在哪里？

根据 W3C 和 Adobe 的规范，PNG 中的 XMP 信息存储在 **`iTXt` (International Textual Data)** 块中。

### 为什么选择 `iTXt`？
1. **UTF-8 支持**：`iTXt` 原生支持 UTF-8 编码，而 `tEXt` 仅支持 Latin-1。
2. **关键词标识**：`iTXt` 允许通过一个关键词（Keyword）来标识数据用途。

### XMP `iTXt` 块的具体内部结构：
在一个包含 XMP 的 `iTXt` 块中，其 `Data` 部分由以下几部分组成：

1.  **Keyword**: 固定为 `XML:com.adobe.xmp` (以空字符 `\0` 结尾)。
2.  **Compression Flag**: 1 字节（`0` 表示不压缩，`1` 表示压缩）。通常 XMP **不压缩**，设为 `0`。
3.  **Compression Method**: 1 字节（通常为 `0`）。
4.  **Language Tag**: 字符串（以 `\0` 结尾），可选，通常为空。
5.  **Translated Keyword**: 字符串（以 `\0` 结尾），可选，通常为空。
6.  **XMP Content**: 实际的 XML 字符串（即 XMP 数据包）。

---

## 3. 读取 XMP 的逻辑流程

1.  **打开文件**：以二进制模式读取 PNG。
2.  **跳过签名**：跳过前 8 个字节。
3.  **遍历块**：
    *   读取 `length`。
    *   读取 `type`。
    *   如果是 `iTXt`：
        *   读取 `data` 部分的前几个字节，检查关键词是否为 `XML:com.adobe.xmp\0`。
        *   如果是，则剩余的 `data` 部分（跳过 flag 和空字符串占位符）即为 XMP XML 文本。
    *   如果不是 `iTXt`：根据 `length + 4` (CRC) 的长度跳过，继续寻找。
    *   如果遇到 `IEND`：遍历结束。

---

## 4. 写入 XMP 的逻辑流程

写入 XMP 实际上是向 PNG 文件中**插入**一个新的 `iTXt` 块。

1.  **准备 XMP 数据**：构建标准的 XMP XML 字符串。
2.  **构建 iTXt Data 负载**：
    *   写入 `XML:com.adobe.xmp\0` (18 字节)。
    *   写入 `0` (Compression flag)。
    *   写入 `0` (Compression method)。
    *   写入 `\0` (Empty Language Tag)。
    *   写入 `\0` (Empty Translated Keyword)。
    *   写入 完整的 XMP XML 字符串。
3.  **计算 CRC**：对 `iTXt` 类型字符和上述 `Data` 负载进行 CRC32 校验。
4.  **重写文件**：
    *   先写入 PNG 签名和 `IHDR` 块。
    *   **建议插入点**：在第一个 `IDAT` 块之前插入你的 `iTXt` XMP 块。
    *   复制原文件的其余块。
    *   确保最后是 `IEND`。

---

## 5. C++ 参考代码片段

以下是实现该工具类的核心逻辑参考：

### 计算 CRC32 (必要)
可以使用 `zlib` 库的 `crc32()` 函数，或者实现一个简单的算法。PNG 规范要求必须对 Type + Data 进行校验。

### 核心写入结构
```cpp
struct PngChunk {
    uint32_t length;
    char type[4];
    std::vector<uint8_t> data;
    uint32_t crc;
};

// 构造 XMP iTXt 块
std::vector<uint8_t> createXmpChunk(const std::string& xmpXml) {
    std::string keyword = "XML:com.adobe.xmp";
    std::vector<uint8_t> payload;

    // 1. Keyword + \0
    payload.insert(payload.end(), keyword.begin(), keyword.end());
    payload.push_back(0);

    // 2. Compression Flag (0) & Method (0)
    payload.push_back(0);
    payload.push_back(0);

    // 3. Language Tag + \0 (空)
    payload.push_back(0);

    // 4. Translated Keyword + \0 (空)
    payload.push_back(0);

    // 5. XMP XML 数据
    payload.insert(payload.end(), xmpXml.begin(), xmpXml.end());

    return payload; 
    // 注意：后续还需封装成 [Length][iTXt][Payload][CRC] 格式
}
