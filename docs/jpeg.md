# 1. 存放位置
JPEG 文件由一系列“标记段”（Markers）组成。XMP 数据被存储在一个或多个 APP1 标记段中。

标记代码：0xFFE1
用途：APP1 标记通常用于存储元数据（Exif 和 XMP 都使用 APP1，但通过不同的标识符区分）。

# 2. 段内结构（数据布局）
一个标准 XMP APP1 段的字节结构如下：

| 偏移量 (字节) | 长度 (字节) | 字段说明 | 内容/值 |
| :--- | :--- | :--- | :--- |
| 0 | 2 | APP1 标记 | 0xFFE1 |
| 2 | 2 | 段长度 | 该段的总长度（不含 0xFFE1 这 2 字节） |
| 4 | 29 | XMP 命名空间标识符 | `http://ns.adobe.com/xap/1.0/\0` |
| 33 | 可变 | XMP 数据包 | 以 XML 格式编码的 RDF 数据 |

# 3. 关键细节

## 标识符（Namespace Signature）
这是识别该 APP1 段是否包含 XMP 的关键。

+ 字符串内容：http://ns.adobe.com/xap/1.0/
+ 注意：在该字符串的末尾必须有一个 NULL 终止符（十六进制 0x00）。
+ 因此，标识符的总长度正好是 29 字节。

## 长度限制（Size Limitation）
由于 JPEG APPx 段的长度字段是由 2 个字节表示的，这意味着：
+ 单个段的最大长度为 65,535 字节。
+ 扣除长度字段（2 字节）和标识符（29 字节）后，标准 XMP 数据包的最大容量约为 65,504 字节。
+ 注：这就是为什么当 XMP 数据超过此限制时需要使用“扩展 XMP”方案。此处不展开

## 数据格式
+ 编码：XMP 规范建议使用 UTF-8 编码。
+ 封装：数据必须包裹在 <?xpacket begin="..." id="..."?> 和 <?xpacket end="..."?> 处理指令（Processing Instructions）之间。
+ 内容：内部是符合 W3C RDF 标准的 XML 结构。

# 4. 在文件中存放的顺序
虽然 JPEG 标准允许 APP 标记以任意顺序出现，但通常的做法是：

+ SOI (Start of Image, 0xFFD8)
+ APP0 (JFIF) 或 APP1 (Exif)
+ APP1 (XMP) —— XMP 通常紧跟在 Exif APP1 段之后。
+ 其他段（如 IPTC、量化表等）。