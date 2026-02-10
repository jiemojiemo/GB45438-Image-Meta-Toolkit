//
// Factory Usage Example
// 演示如何使用 AIGC Reader/Writer 工厂类和统一接口
//

#include "gimt/gimt_aigc_reader_factory.h"
#include "gimt/gimt_aigc_writer_factory.h"
#include "gimt/gimt_def.h"
#include <iostream>

using namespace gimt;

// 示例 1: 使用工厂自动检测格式并读取 AIGC 信息
void example1_auto_read(const std::string& filepath) {
  std::cout << "=== Example 1: Auto-detect and Read ===" << std::endl;
  
  // 方式 1: 分步创建和准备
  auto reader = AIGCReaderFactory::createReaderFromContent(filepath);
  if (!reader) {
    std::cerr << "Failed to create reader (unsupported format)" << std::endl;
    return;
  }
  
  std::cout << "Detected format: " << getFormatName(reader->getFormat()) << std::endl;
  
  if (!reader->prepare(filepath)) {
    std::cerr << "Failed to prepare reader" << std::endl;
    return;
  }
  
  AIGCInfo info;
  if (reader->readAIGCInfo(info)) {
    std::cout << "Successfully read AIGC info:" << std::endl;
    std::cout << "  Label: " << info.label << std::endl;
    std::cout << "  ContentProducer: " << info.contentProducer << std::endl;
    std::cout << "  ProduceID: " << info.produceID << std::endl;
  } else {
    std::cerr << "Failed to read AIGC info" << std::endl;
  }
}

// 示例 2: 使用一步式工厂方法
void example2_one_step_read(const std::string& filepath) {
  std::cout << "\n=== Example 2: One-step Read ===" << std::endl;
  
  // 一步完成：创建 + 准备
  auto reader = AIGCReaderFactory::createAndPrepare(filepath, true);
  if (!reader) {
    std::cerr << "Failed to create and prepare reader" << std::endl;
    return;
  }
  
  std::cout << "Format: " << getFormatName(reader->getFormat()) << std::endl;
  
  AIGCInfo info;
  if (reader->readAIGCInfo(info)) {
    std::cout << "AIGC Info JSON: " << info.toJson() << std::endl;
  }
}

// 示例 3: 使用工厂写入 AIGC 信息
void example3_auto_write(const std::string& inputPath, const std::string& outputPath) {
  std::cout << "\n=== Example 3: Auto-detect and Write ===" << std::endl;
  
  // 创建 Writer（自动检测格式）
  auto writer = AIGCWriterFactory::createAndPrepare(inputPath, outputPath, true);
  if (!writer) {
    std::cerr << "Failed to create and prepare writer" << std::endl;
    return;
  }
  
  std::cout << "Writing to format: " << getFormatName(writer->getFormat()) << std::endl;
  
  // 准备 AIGC 信息
  AIGCInfo info;
  info.label = "AIGC";
  info.contentProducer = "ExampleProducer";
  info.produceID = "12345";
  info.reservedCode1 = "";
  info.contentPropagator = "ExamplePropagator";
  info.propagateID = "67890";
  info.reservedCode2 = "";
  
  if (writer->writeAIGCInfo(info)) {
    std::cout << "Successfully wrote AIGC info to: " << outputPath << std::endl;
  } else {
    std::cerr << "Failed to write AIGC info" << std::endl;
  }
}

// 示例 4: 指定格式创建 Reader/Writer
void example4_specific_format() {
  std::cout << "\n=== Example 4: Create with Specific Format ===" << std::endl;
  
  // 直接指定格式创建
  auto jpegReader = AIGCReaderFactory::createReader(ImageFormat::JPEG);
  auto pngWriter = AIGCWriterFactory::createWriter(ImageFormat::PNG);
  auto webpReader = AIGCReaderFactory::createReader(ImageFormat::WEBP);
  auto heifWriter = AIGCWriterFactory::createWriter(ImageFormat::HEIF);
  
  std::cout << "Created readers and writers for specific formats:" << std::endl;
  if (jpegReader) std::cout << "  - JPEG Reader: " << getFormatName(jpegReader->getFormat()) << std::endl;
  if (pngWriter) std::cout << "  - PNG Writer: " << getFormatName(pngWriter->getFormat()) << std::endl;
  if (webpReader) std::cout << "  - WebP Reader: " << getFormatName(webpReader->getFormat()) << std::endl;
  if (heifWriter) std::cout << "  - HEIF Writer: " << getFormatName(heifWriter->getFormat()) << std::endl;
}

// 示例 5: 批量处理多种格式
void example5_batch_process(const std::vector<std::string>& inputFiles, 
                            const std::string& outputDir) {
  std::cout << "\n=== Example 5: Batch Process Multiple Formats ===" << std::endl;
  
  for (const auto& inputFile : inputFiles) {
    std::cout << "\nProcessing: " << inputFile << std::endl;
    
    // 读取
    auto reader = AIGCReaderFactory::createAndPrepare(inputFile, true);
    if (!reader) {
      std::cerr << "  Skipped (unsupported format)" << std::endl;
      continue;
    }
    
    AIGCInfo info;
    if (!reader->readAIGCInfo(info)) {
      std::cerr << "  No AIGC info found" << std::endl;
      continue;
    }
    
    std::cout << "  Read AIGC info from " << getFormatName(reader->getFormat()) << std::endl;
    
    // 修改信息
    info.contentPropagator = "BatchProcessor";
    
    // 写入到输出文件
    std::string outputFile = outputDir + "/processed_" + inputFile.substr(inputFile.find_last_of("/\\") + 1);
    auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
    if (writer && writer->writeAIGCInfo(info)) {
      std::cout << "  Written to: " << outputFile << std::endl;
    }
  }
}

// 示例 6: 使用基类指针实现多态
void example6_polymorphism() {
  std::cout << "\n=== Example 6: Polymorphism with Base Class ===" << std::endl;
  
  // 使用基类指针存储不同格式的 Reader
  std::vector<AIGCReaderPtr> readers;
  readers.push_back(AIGCReaderFactory::createReader(ImageFormat::JPEG));
  readers.push_back(AIGCReaderFactory::createReader(ImageFormat::PNG));
  readers.push_back(AIGCReaderFactory::createReader(ImageFormat::WEBP));
  readers.push_back(AIGCReaderFactory::createReader(ImageFormat::HEIF));
  
  std::cout << "Created " << readers.size() << " readers using polymorphism:" << std::endl;
  for (const auto& reader : readers) {
    if (reader) {
      std::cout << "  - " << getFormatName(reader->getFormat()) << " Reader" << std::endl;
    }
  }
  
  // 使用基类指针存储不同格式的 Writer
  std::vector<AIGCWriterPtr> writers;
  writers.push_back(AIGCWriterFactory::createWriter(ImageFormat::JPEG));
  writers.push_back(AIGCWriterFactory::createWriter(ImageFormat::PNG));
  writers.push_back(AIGCWriterFactory::createWriter(ImageFormat::WEBP));
  writers.push_back(AIGCWriterFactory::createWriter(ImageFormat::HEIF));
  
  std::cout << "Created " << writers.size() << " writers using polymorphism:" << std::endl;
  for (const auto& writer : writers) {
    if (writer) {
      std::cout << "  - " << getFormatName(writer->getFormat()) << " Writer" << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  std::cout << "AIGC Reader/Writer Factory Usage Examples\n" << std::endl;
  
  // 运行示例
  if (argc > 1) {
    std::string testFile = argv[1];
    example1_auto_read(testFile);
    example2_one_step_read(testFile);
    
    if (argc > 2) {
      std::string outputFile = argv[2];
      example3_auto_write(testFile, outputFile);
    }
  }
  
  example4_specific_format();
  example6_polymorphism();
  
  std::cout << "\n=== All examples completed ===" << std::endl;
  
  return 0;
}





