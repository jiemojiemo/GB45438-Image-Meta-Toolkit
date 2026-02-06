# AIGC Reader/Writer 统一接口设计方案

## 概述

本方案为 core-cpp 中的所有 Reader 和 Writer 提供了统一的接口和工厂类，便于扩展和使用。

## 架构设计

### 1. 核心组件

```
gimt/
├── gimt_image_format.h/cpp          # 图像格式枚举和检测
├── gimt_aigc_reader.h               # Reader 抽象基类
├── gimt_aigc_writer.h               # Writer 抽象基类
├── gimt_aigc_reader_factory.h/cpp   # Reader 工厂类
└── gimt_aigc_writer_factory.h/cpp   # Writer 工厂类
```

### 2. 类图关系

```
IAIGCReader (抽象基类)
    ├── JpegAIGCReader
    ├── PngAIGCReader
    ├── WebpAIGCReader
    └── HeifAIGCReader

IAIGCWriter (抽象基类)
    ├── JpegAIGCWriter
    ├── PngAIGCWriter
    ├── WebpAIGCWriter
    └── HeifAIGCWriter

AIGCReaderFactory (工厂类)
AIGCWriterFactory (工厂类)
```

## 接口定义

### IAIGCReader 基类

```cpp
class IAIGCReader {
public:
  virtual ~IAIGCReader() = default;
  virtual bool prepare(const std::string& filepath) = 0;
  virtual bool readAIGCInfo(AIGCInfo& info) = 0;
  virtual ImageFormat getFormat() const = 0;
  virtual bool isPrepared() const { return prepared_; }
protected:
  bool prepared_ = false;
};
```

### IAIGCWriter 基类

```cpp
class IAIGCWriter {
public:
  virtual ~IAIGCWriter() = default;
  virtual bool prepare(const std::string& inputFilepath, 
                      const std::string& outputFilepath) = 0;
  virtual bool writeAIGCInfo(const AIGCInfo& info) = 0;
  virtual ImageFormat getFormat() const = 0;
  virtual bool isPrepared() const { return prepared_; }
protected:
  bool prepared_ = false;
};
```

### ImageFormat 枚举

```cpp
enum class ImageFormat {
  JPEG,
  PNG,
  WEBP,
  HEIF,
  UNKNOWN
};
```

## 工厂类功能

### AIGCReaderFactory

- `createReader(ImageFormat)` - 根据格式创建 Reader
- `createReaderFromPath(filepath)` - 根据文件扩展名创建
- `createReaderFromContent(filepath)` - 根据文件头创建
- `createAndPrepare(filepath, autoDetect)` - 一步式创建并准备

### AIGCWriterFactory

- `createWriter(ImageFormat)` - 根据格式创建 Writer
- `createWriterFromPath(filepath)` - 根据文件扩展名创建
- `createWriterFromContent(filepath)` - 根据文件头创建
- `createAndPrepare(inputPath, outputPath, autoDetect)` - 一步式创建并准备

## 使用示例

### 示例 1: 自动检测格式并读取

```cpp
auto reader = AIGCReaderFactory::createAndPrepare("image.jpg", true);
if (reader) {
  AIGCInfo info;
  if (reader->readAIGCInfo(info)) {
    std::cout << "Label: " << info.label << std::endl;
  }
}
```

### 示例 2: 自动检测格式并写入

```cpp
auto writer = AIGCWriterFactory::createAndPrepare("input.png", "output.png", true);
if (writer) {
  AIGCInfo info;
  info.label = "AIGC";
  info.contentProducer = "MyApp";
  writer->writeAIGCInfo(info);
}
```

### 示例 3: 指定格式创建

```cpp
auto jpegReader = AIGCReaderFactory::createReader(ImageFormat::JPEG);
jpegReader->prepare("image.jpg");
```

### 示例 4: 批量处理多种格式

```cpp
std::vector<std::string> files = {"a.jpg", "b.png", "c.webp", "d.heif"};
for (const auto& file : files) {
  auto reader = AIGCReaderFactory::createAndPrepare(file, true);
  if (reader) {
    AIGCInfo info;
    reader->readAIGCInfo(info);
    // 处理信息...
  }
}
```

### 示例 5: 多态使用

```cpp
std::vector<AIGCReaderPtr> readers;
readers.push_back(AIGCReaderFactory::createReader(ImageFormat::JPEG));
readers.push_back(AIGCReaderFactory::createReader(ImageFormat::PNG));

for (const auto& reader : readers) {
  std::cout << "Format: " << getFormatName(reader->getFormat()) << std::endl;
}
```

## 格式检测

### 基于扩展名检测

```cpp
ImageFormat format = detectFormatFromPath("image.jpg");
// 支持: .jpg, .jpeg, .png, .webp, .heif, .heic
```

### 基于文件头检测（更可靠）

```cpp
ImageFormat format = detectFormatFromContent("image.jpg");
// 检测文件头魔数:
// - JPEG: FF D8
// - PNG: 89 50 4E 47 0D 0A 1A 0A
// - WebP: RIFF....WEBP
// - HEIF: ....ftyp
```

## 设计优势

### 1. 统一接口
- 所有 Reader/Writer 实现相同接口
- 易于理解和使用
- 支持多态

### 2. 工厂模式
- 自动格式检测
- 简化对象创建
- 集中管理实例化逻辑

### 3. 易于扩展
- 添加新格式只需：
  1. 实现 Reader/Writer 类
  2. 在工厂类中添加创建逻辑
  3. 在 ImageFormat 枚举中添加格式

### 4. 智能指针管理
- 使用 `std::unique_ptr` 自动管理内存
- 类型别名：`AIGCReaderPtr`, `AIGCWriterPtr`

### 5. 向后兼容
- 现有代码无需修改
- 可以继续直接使用具体类
- 也可以使用新的统一接口

## 测试结果

✅ 所有 125 个单元测试通过
- 9 个 BinaryReader 测试
- 5 个 JpegAIGCReader 测试
- 8 个 PatternMatcher 测试
- 6 个 JpegAIGCWriter 测试
- 13 个 PngAIGCReader 测试
- 16 个 PngAIGCWriter 测试
- 14 个 WebpAIGCReader 测试
- 19 个 WebpAIGCWriter 测试
- 16 个 HeifAIGCReader 测试
- 19 个 HeifAIGCWriter 测试

## 文件清单

### 新增头文件
- `core-cpp/include/gimt/gimt_image_format.h`
- `core-cpp/include/gimt/gimt_aigc_reader.h`
- `core-cpp/include/gimt/gimt_aigc_writer.h`
- `core-cpp/include/gimt/gimt_aigc_reader_factory.h`
- `core-cpp/include/gimt/gimt_aigc_writer_factory.h`

### 新增实现文件
- `core-cpp/src/gimt_image_format.cpp`
- `core-cpp/src/gimt_aigc_reader_factory.cpp`
- `core-cpp/src/gimt_aigc_writer_factory.cpp`

### 示例代码
- `core-cpp/examples/factory_usage_example.cpp`

### 修改的文件
- `core-cpp/CMakeLists.txt` - 添加新源文件
- `core-cpp/include/gimt/gimt_jpeg_aigc_reader.h` - 继承基类
- `core-cpp/include/gimt/gimt_jpeg_aigc_writer.h` - 继承基类
- `core-cpp/include/gimt/gimt_png_aigc_reader.h` - 继承基类
- `core-cpp/include/gimt/gimt_png_aigc_writer.h` - 继承基类
- `core-cpp/include/gimt/gimt_webp_aigc_reader.h` - 继承基类
- `core-cpp/include/gimt/gimt_webp_aigc_writer.h` - 继承基类
- `core-cpp/include/gimt/gimt_heif_aigc_reader.h` - 继承基类
- `core-cpp/include/gimt/gimt_heif_aigc_writer.h` - 继承基类

## 使用建议

1. **新代码推荐使用工厂类**：更简洁，自动检测格式
2. **批量处理使用基类指针**：利用多态特性
3. **优先使用文件头检测**：比扩展名更可靠
4. **使用一步式方法**：`createAndPrepare()` 更方便

## 未来扩展

如需添加新格式（如 AVIF、TIFF 等）：

1. 创建 `XxxAIGCReader` 和 `XxxAIGCWriter` 类
2. 继承 `IAIGCReader` 和 `IAIGCWriter`
3. 在 `ImageFormat` 枚举中添加新格式
4. 在 `detectFormatFromContent()` 中添加检测逻辑
5. 在工厂类的 `createReader/Writer()` 中添加创建逻辑

