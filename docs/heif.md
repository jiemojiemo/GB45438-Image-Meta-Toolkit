# HEIF/HEIC XMP 嵌入式元数据开发指南 (C/C++)

本指南旨在帮助开发者理解 HEIF/HEIC (ISO/IEC 23008-12) 格式中 XMP 元数据的存储逻辑，并提供实现读写工具的思路。

---

## 1. 基础概念：HEIF 封装架构

HEIF (High Efficiency Image File Format) 并非传统的线性位图格式，它基于 **ISOBMFF (ISO Base Media File Format)** 规范（与 MP4 视频容器同宗同源）。

### 1.1 Box 结构
HEIF 文件由一系列称为 **Box（盒子）** 的数据单元组成。
*   每个 Box 的开头 8 字节：`4字节长度 + 4字节类型(如 'meta', 'iloc')`。
*   Box 可以嵌套。

### 1.2 核心 Box 角色
对于 XMP 读写，我们只需关注以下 Box 路径：
*   **`ftyp`**: 文件类型标识，判断是否为 `heic` 或 `mif1`。
*   **`meta`**: 关键容器，存放所有元数据的索引。
    *   **`hdlr`**: 处理器类型，必须为 `pict`。
    *   **`iinf` (Item Information)**: 罗列所有“物件”（Item），如主图、缩略图、XMP。
    *   **`iloc` (Item Location)**: 记录每个 Item 在文件中的具体物理偏移（Offset）和长度（Size）。
*   **`mdat` (Media Data)**: 实际存放 XMP 文本和图像像素数据的二进制大块。

---

## 2. XMP 在 HEIF 中的存储标准

在 HEIF 标准中，XMP 被视为一个**独立的 Item**（通常称为 Metadata Item）。

### 2.1 识别 XMP Item
要找到 XMP，需要遍历 `meta -> iinf` 盒子下的所有条目（`infe`）：
1.  **Item Type**: XMP 条目的类型通常标记为 `'mime'`。
2.  **MIME Type**: 其对应的字符串属性为 `application/rdf+xml`。
3.  **Item ID**: 记录下该条目的 ID（例如 `ID=10`），这是连接位置信息的唯一索引。

### 2.2 定位 XMP 数据
拿到 `ItemID` 后，去 `meta -> iloc` 盒子查找：
*   匹配相同的 `ItemID`。
*   读取该 Item 的 `base_offset`、`extent_offset` 和 `extent_length`。
*   **计算公式**: `实际物理地址 = base_offset + extent_offset`。

---

## 3. 读取 XMP 的逻辑步骤

```cpp
// 伪代码流程
1. 打开文件，读取 4字节 Size + 4字节 Type ('ftyp')。
2. 循环读取 Box，直到找到 Type == 'meta'。
3. 进入 'meta' Box：
   a. 跳过 4 字节的 Version/Flags。
   b. 找到 'iinf' Box，遍历里面的 'infe' 条目。
   c. 如果 infe.item_type == 'mime' 且 content_type == "application/rdf+xml":
      - 记录当前 infe.item_ID。
   d. 找到 'iloc' Box：
      - 遍历条目，匹配 item_ID。
      - 获取 offset 和 length。
4. 根据 offset 和 length，从原始文件中 fseek 并读取数据。
5. 解析读取到的 XML 字符串。
```

---

## 4. 写入 XMP 的逻辑步骤（核心难点）

写入 HEIF 比读取复杂得多，因为 ISOBMFF 采用的是**绝对偏移**索引。修改数据长度会导致所有后续偏移失效。

### 4.1 策略 A：覆盖写入（仅限长度不变）
如果新旧 XMP 长度完全一致，可直接在 `mdat` 原位替换。

### 4.2 策略 B：追加写入（标准做法）
1.  **解析原始文件**：记录所有 Box 的结构。
2.  **准备新 XMP 数据**：
3.  **更新 `mdat`**：将新的 XMP 数据追加到文件末尾（或重新构建 `mdat`）。
4.  **更新 `iloc`**：将 XMP 对应 Item 的 `extent_length` 改为新长度，`offset` 改为新位置。
5.  **更新父 Box 长度**：由于 `iloc` 和 `mdat` 内容变了，所有父级 Box（如 `meta`）以及文件头的 `Size` 字段都必须递归更新。

---

## 5. C++ 实现建议与伪代码

### 5.1 数据结构定义
```cpp
struct BoxHeader {
    uint32_t size;    // 包含 header 自身的总长度
    char type[4];     // 如 'meta', 'iloc'
    uint64_t largeSize; // 如果 size == 1，则使用此 8 字节
};

struct ItemLocation {
    uint32_t itemID;
    uint64_t offset;
    uint32_t length;
};
```

### 5.2 读取逻辑片段
```cpp
// 简化的读取示例
void ReadXMP(FILE* fp) {
    // 1. 定位到 meta box
    BoxHeader metaHeader = FindBox(fp, "meta");
    
    // 2. 在 meta 内部查找 iinf 以获取 ID
    uint32_t xmpID = 0;
    while(HasNextItem(fp)) {
        InfeEntry entry = ParseInfe(fp);
        if(entry.type == "mime" && entry.mime == "application/rdf+xml") {
            xmpID = entry.id;
            break;
        }
    }

    // 3. 在 meta 内部查找 iloc 以获取位置
    ItemLocation loc = FindInIloc(fp, xmpID);

    // 4. 提取数据
    fseek(fp, loc.offset, SEEK_SET);
    char* xmpBuffer = new char[loc.length + 1];
    fread(xmpBuffer, 1, loc.length, fp);
    xmpBuffer[loc.length] = '\0';
    
    printf("XMP Data: %s\n", xmpBuffer);
}
```

---

## 6. 开发注意事项（新手必看）

1.  **大端字节序 (Big-Endian)**: ISOBMFF 规范规定所有数值以大端存储。在 C++ 中读取 `uint32_t` 后必须使用 `ntohl()` 或手动位移转换。
2.  **FullBox**: 有些 Box（如 `meta`, `iinf`, `iloc`）是 "FullBox"，它们在 `Type` 之后多出 4 个字节，包含 1 字节 Version 和 3 字节 Flags。解析时必须跳过这 4 字节。
3.  **对齐与填充**: 某些实现可能会在 `mdat` 中对数据进行 4 字节对齐，读取时以 `iloc` 记录的精确长度为准。
4.  **UUID Box**: 虽然标准建议使用 `mime` 项存储 XMP，但某些设备（如早期苹果设备或部分安卓）可能会直接在 `meta` 下面放一个 `uuid` Box 来存 XMP。
    *   XMP 专用的 UUID 是：`be7e8122-df71-11e0-a430-0902a28f5f00`。
    *   **建议**：如果通过 `iinf` 找不到 XMP，尝试搜索此 UUID Box。
---