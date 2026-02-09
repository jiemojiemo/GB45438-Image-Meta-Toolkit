package com.jiemo.gimt_native

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.jiemo.gimt_api.AIGCInfo
import com.jiemo.gimt_api.AIGCWriterUtils
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File

/**
 * AIGC Writer Utils 单元测试
 * 
 * 测试包含三大块：
 * 1. 所有支持的格式检查
 * 2. 能写入 AIGC 数据到所有支持的格式
 * 3. 如果文件路径非法，写入失败
 */
@RunWith(AndroidJUnit4::class)
class AIGCWriterUtilsTest {
    
    private lateinit var writerUtils: AIGCWriterUtils
    private lateinit var readerUtils: NativeAIGCReaderUtils
    private val context = InstrumentationRegistry.getInstrumentation().context
    
    @Before
    fun setUp() {
        writerUtils = NativeAIGCWriterUtils()
        readerUtils = NativeAIGCReaderUtils()
    }
    
    // ========================================================================
    // 测试块 1: 所有支持的格式检查
    // ========================================================================
    
    @Test
    fun testGetSupportedFormats() {
        val formats = writerUtils.getSupportedFormats()
        
        // 验证包含所有支持的格式
        assertTrue("Should support jpg", formats.contains("jpg"))
        assertTrue("Should support jpeg", formats.contains("jpeg"))
        assertTrue("Should support png", formats.contains("png"))
        assertTrue("Should support webp", formats.contains("webp"))
        assertTrue("Should support heif", formats.contains("heif"))
        assertTrue("Should support heic", formats.contains("heic"))
    }
    
    @Test
    fun testSupportedFormatsCount() {
        val formats = writerUtils.getSupportedFormats()
        
        // 至少支持 4 种主要格式（JPEG, PNG, WebP, HEIF）
        assertTrue("Should support at least 4 formats", formats.size >= 4)
        
        // 验证没有重复
        assertEquals("Should not have duplicates", formats.size, formats.toSet().size)
    }
    
    @Test
    fun testSupportedFormatsNotEmpty() {
        val formats = writerUtils.getSupportedFormats()
        
        assertFalse("Supported formats should not be empty", formats.isEmpty())
    }
    
    // ========================================================================
    // 测试块 2: 能写入 AIGC 数据到所有支持的格式
    // ========================================================================
    
    @Test
    fun testWriteAIGCToJpeg() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        val outputFile = File(context.cacheDir, "output_test_jpeg.jpg")
        
        val aigcInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "TestProducer",
            produceID = "12345"
        )
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            aigcInfo
        )
        
        assertTrue("Should write AIGC to JPEG", result)
        assertTrue("Output file should exist", outputFile.exists())
        
        // 验证写入的数据可以读取
        val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
        assertNotNull("Should read back AIGC info", readInfo)
        assertEquals("Label should match", aigcInfo.label, readInfo?.label)
        assertEquals("Producer should match", aigcInfo.contentProducer, readInfo?.contentProducer)
        assertEquals("ProduceID should match", aigcInfo.produceID, readInfo?.produceID)
        
        // 清理
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testWriteAIGCToPng() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("png_empty.png")
        val outputFile = File(context.cacheDir, "output_test_png.png")
        
        val aigcInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "PngTestProducer",
            produceID = "PNG123"
        )
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            aigcInfo
        )
        
        assertTrue("Should write AIGC to PNG", result)
        assertTrue("Output file should exist", outputFile.exists())
        
        // 验证写入的数据可以读取
        val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
        assertNotNull("Should read back AIGC info", readInfo)
        assertEquals("Label should match", aigcInfo.label, readInfo?.label)
        
        // 清理
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testWriteAIGCToWebp() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("webp_empty.webp")
        val outputFile = File(context.cacheDir, "output_test_webp.webp")
        
        val aigcInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "WebpTestProducer",
            produceID = "WEBP456"
        )
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            aigcInfo
        )
        
        assertTrue("Should write AIGC to WebP", result)
        assertTrue("Output file should exist", outputFile.exists())
        
        // 验证写入的数据可以读取
        val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
        assertNotNull("Should read back AIGC info", readInfo)
        assertEquals("Label should match", aigcInfo.label, readInfo?.label)
        
        // 清理
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testWriteAIGCToHeif() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("heif_empty.heif")
        val outputFile = File(context.cacheDir, "output_test_heif.heif")
        
        val aigcInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "HeifTestProducer",
            produceID = "HEIF789"
        )
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            aigcInfo
        )
        
        assertTrue("Should write AIGC to HEIF", result)
        assertTrue("Output file should exist", outputFile.exists())
        
        // 验证写入的数据可以读取
        val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
        assertNotNull("Should read back AIGC info", readInfo)
        assertEquals("Label should match", aigcInfo.label, readInfo?.label)
        
        // 清理
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testWriteAIGCToAllFormats() {
        val testCases = listOf(
            "jpg_empty.jpg" to "output_batch_test.jpg",
            "png_empty.png" to "output_batch_test.png",
            "webp_empty.webp" to "output_batch_test.webp",
            "heif_empty.heif" to "output_batch_test.heif"
        )
        
        val aigcInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "BatchTest",
            produceID = "99999"
        )
        
        testCases.forEach { (inputName, outputName) ->
            val inputFile = TestResourceHelper.copyResourceToTempFile(inputName)
            val outputFile = File(context.cacheDir, outputName)
            
            val result = writerUtils.writeAIGC(
                inputFile.absolutePath,
                outputFile.absolutePath,
                aigcInfo
            )
            
            assertTrue("Should write AIGC to $inputName", result)
            assertTrue("Output file $outputName should exist", outputFile.exists())
            
            // 验证可以读取
            val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
            assertNotNull("Should read back from $outputName", readInfo)
            assertEquals("Label should match for $outputName", aigcInfo.label, readInfo?.label)
            
            inputFile.delete()
            outputFile.delete()
        }
    }
    
    @Test
    fun testWriteAndReadRoundtrip() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        val outputFile = File(context.cacheDir, "roundtrip_test.jpg")
        
        val originalInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "RoundtripTest",
            produceID = "54321",
            reservedCode1 = "Reserved1",
            contentPropagator = "Propagator",
            propagateID = "67890",
            reservedCode2 = "Reserved2"
        )
        
        // 写入
        assertTrue("Should write successfully", writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            originalInfo
        ))
        
        // 读取
        val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
        
        assertNotNull("Should read back info", readInfo)
        assertEquals("Label should match", originalInfo.label, readInfo?.label)
        assertEquals("ContentProducer should match", originalInfo.contentProducer, readInfo?.contentProducer)
        assertEquals("ProduceID should match", originalInfo.produceID, readInfo?.produceID)
        assertEquals("ReservedCode1 should match", originalInfo.reservedCode1, readInfo?.reservedCode1)
        assertEquals("ContentPropagator should match", originalInfo.contentPropagator, readInfo?.contentPropagator)
        assertEquals("PropagateID should match", originalInfo.propagateID, readInfo?.propagateID)
        assertEquals("ReservedCode2 should match", originalInfo.reservedCode2, readInfo?.reservedCode2)
        
        inputFile.delete()
        outputFile.delete()
    }
    
    // ========================================================================
    // 测试块 3: 如果文件路径非法，写入失败
    // ========================================================================
    
    @Test
    fun testWriteAIGCWithInvalidPaths() {
        val aigcInfo = AIGCInfo(label = "AIGC", contentProducer = "Test")
        
        // 测试不存在的输入文件
        assertFalse(
            "Should fail with non-existent input",
            writerUtils.writeAIGC(
                "/non/existent/input.jpg",
                File(context.cacheDir, "output.jpg").absolutePath,
                aigcInfo
            )
        )
        
        // 测试空输入路径
        assertFalse(
            "Should fail with empty input path",
            writerUtils.writeAIGC(
                "",
                File(context.cacheDir, "output.jpg").absolutePath,
                aigcInfo
            )
        )
        
        // 测试空输出路径
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        assertFalse(
            "Should fail with empty output path",
            writerUtils.writeAIGC(
                inputFile.absolutePath,
                "",
                aigcInfo
            )
        )
        inputFile.delete()
        
        // 测试不支持的格式
        val tempFile = File(context.cacheDir, "test_unsupported.bmp")
        tempFile.writeBytes(byteArrayOf(0x42, 0x4D)) // BMP header
        assertFalse(
            "Should fail with unsupported format",
            writerUtils.writeAIGC(
                tempFile.absolutePath,
                File(context.cacheDir, "output.bmp").absolutePath,
                aigcInfo
            )
        )
        tempFile.delete()
        
        // 测试无效的输出目录
        val validInput = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        assertFalse(
            "Should fail with invalid output directory",
            writerUtils.writeAIGC(
                validInput.absolutePath,
                "/invalid/directory/that/does/not/exist/output.jpg",
                aigcInfo
            )
        )
        validInput.delete()
    }
    
    // ========================================================================
    // 额外测试：边界情况和数据验证
    // ========================================================================
    
    @Test
    fun testWriteEmptyAIGCInfo() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        val outputFile = File(context.cacheDir, "empty_info_test.jpg")
        
        val emptyInfo = AIGCInfo() // 所有字段为空
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            emptyInfo
        )
        
        assertTrue("Should write even with empty info", result)
        assertTrue("Output file should exist", outputFile.exists())
        
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testWriteSpecialCharacters() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        val outputFile = File(context.cacheDir, "special_chars_test.jpg")
        
        val specialInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "测试中文",
            produceID = "特殊字符!@#$%"
        )
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            specialInfo
        )
        
        assertTrue("Should write with special characters", result)
        
        // 验证可以读取特殊字符
        val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
        assertNotNull("Should read back special characters", readInfo)
        assertEquals("Chinese characters should match", specialInfo.contentProducer, readInfo?.contentProducer)
        
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testOverwriteExistingFile() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        val outputFile = File(context.cacheDir, "overwrite_test.jpg")
        
        // 创建已存在的输出文件
        outputFile.writeBytes(byteArrayOf(1, 2, 3))
        assertTrue("Output file should exist before test", outputFile.exists())
        val originalSize = outputFile.length()
        
        val aigcInfo = AIGCInfo(label = "AIGC", contentProducer = "Overwrite")
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            aigcInfo
        )
        
        assertTrue("Should overwrite existing file", result)
        assertTrue("Output file should still exist", outputFile.exists())
        assertNotEquals("File size should change", originalSize, outputFile.length())
        
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testMultipleWritesToSameOutput() {
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        val outputFile = File(context.cacheDir, "multiple_writes_test.jpg")
        
        val info1 = AIGCInfo(label = "AIGC", contentProducer = "First", produceID = "111")
        val info2 = AIGCInfo(label = "AIGC", contentProducer = "Second", produceID = "222")
        
        // 第一次写入
        assertTrue("First write should succeed", writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            info1
        ))
        
        // 第二次写入（覆盖）
        assertTrue("Second write should succeed", writerUtils.writeAIGC(
            inputFile.absolutePath,
            outputFile.absolutePath,
            info2
        ))
        
        // 验证最后写入的数据
        val readInfo = readerUtils.readFromFilePath(outputFile.absolutePath)
        assertNotNull("Should read back info", readInfo)
        assertEquals("Should have second producer", "Second", readInfo?.contentProducer)
        assertEquals("Should have second ID", "222", readInfo?.produceID)
        
        inputFile.delete()
        outputFile.delete()
    }
    
    @Test
    fun testWriteToSameInputOutput() {
        // 测试输入输出为同一文件的情况
        val inputFile = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        
        val aigcInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "SameFileTest",
            produceID = "SAME123"
        )
        
        val result = writerUtils.writeAIGC(
            inputFile.absolutePath,
            inputFile.absolutePath,
            aigcInfo
        )
        
        assertTrue("Should write to same file", result)
        
        // 验证可以读取
        val readInfo = readerUtils.readFromFilePath(inputFile.absolutePath)
        assertNotNull("Should read back from same file", readInfo)
        assertEquals("Label should match", aigcInfo.label, readInfo?.label)
        
        inputFile.delete()
    }
    
    @Test
    fun testWriteMultipleFormatsSequentially() {
        val formats = listOf(
            "jpg_empty.jpg" to "seq_test.jpg",
            "png_empty.png" to "seq_test.png",
            "webp_empty.webp" to "seq_test.webp"
        )
        
        formats.forEach { (inputName, outputName) ->
            val inputFile = TestResourceHelper.copyResourceToTempFile(inputName)
            val outputFile = File(context.cacheDir, outputName)
            
            val aigcInfo = AIGCInfo(
                label = "AIGC",
                contentProducer = "Sequential_$inputName",
                produceID = "SEQ_${System.currentTimeMillis()}"
            )
            
            assertTrue(
                "Should write to $outputName",
                writerUtils.writeAIGC(
                    inputFile.absolutePath,
                    outputFile.absolutePath,
                    aigcInfo
                )
            )
            
            inputFile.delete()
            outputFile.delete()
        }
    }
}

