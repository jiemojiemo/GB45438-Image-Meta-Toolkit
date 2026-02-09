package com.jiemo.gimt_native

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.jiemo.gimt_api.AIGCReaderUtils
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File

/**
 * AIGC Reader Utils 单元测试
 * 
 * 测试包含三大块：
 * 1. 所有支持的格式检查
 * 2. 能读取到所有支持格式的 AIGC 数据
 * 3. 如果没有 AIGC 数据，读取失败
 */
@RunWith(AndroidJUnit4::class)
class AIGCReaderUtilsTest {
    
    private lateinit var readerUtils: AIGCReaderUtils
    private val context = InstrumentationRegistry.getInstrumentation().context
    
    @Before
    fun setUp() {
        readerUtils = NativeAIGCReaderUtils()
    }
    
    // ========================================================================
    // 测试块 1: 所有支持的格式检查
    // ========================================================================
    
    @Test
    fun testGetSupportedFormats() {
        val formats = readerUtils.getSupportedFormats()
        
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
        val formats = readerUtils.getSupportedFormats()
        
        // 至少支持 4 种主要格式（JPEG, PNG, WebP, HEIF）
        assertTrue("Should support at least 4 formats", formats.size >= 4)
        
        // 验证没有重复
        assertEquals("Should not have duplicates", formats.size, formats.toSet().size)
    }
    
    @Test
    fun testSupportedFormatsNotEmpty() {
        val formats = readerUtils.getSupportedFormats()
        
        assertFalse("Supported formats should not be empty", formats.isEmpty())
    }
    
    // ========================================================================
    // 测试块 2: 能读取到所有支持格式的 AIGC 数据
    // ========================================================================
    
    @Test
    fun testReadJpegWithAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("jpg_with_xmp.jpg")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNotNull("Should read AIGC info from JPEG", info)
        assertEquals("Label should be '1'", "1", info?.label)
        assertFalse("Info should not be empty", info?.isEmpty() ?: true)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadPngWithAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("png_with_xmp.png")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNotNull("Should read AIGC info from PNG", info)
        assertEquals("Label should be '1'", "1", info?.label)
        assertFalse("Info should not be empty", info?.isEmpty() ?: true)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadWebpWithAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("webp_with_xmp.webp")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNotNull("Should read AIGC info from WebP", info)
        assertEquals("Label should be '1'", "1", info?.label)
        assertFalse("Info should not be empty", info?.isEmpty() ?: true)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadHeifWithAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("heif_with_xmp.heif")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNotNull("Should read AIGC info from HEIF", info)
        assertEquals("Label should be '1'", "1", info?.label)
        assertFalse("Info should not be empty", info?.isEmpty() ?: true)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadAllFormatsWithAIGC() {
        val testFiles = listOf(
            "jpg_with_xmp.jpg",
            "png_with_xmp.png",
            "webp_with_xmp.webp",
            "heif_with_xmp.heif"
        )
        
        testFiles.forEach { fileName ->
            val file = TestResourceHelper.copyResourceToTempFile(fileName)
            val info = readerUtils.readFromFilePath(file.absolutePath)
            
            assertNotNull("Should read AIGC info from $fileName", info)
            assertTrue("Info should be valid for $fileName", info?.isValid() ?: false)
            
            file.delete()
        }
    }
    
    // ========================================================================
    // 测试块 3: 如果没有 AIGC 数据，读取失败
    // ========================================================================
    
    @Test
    fun testReadJpegWithoutAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("jpg_empty.jpg")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNull("Should return null for JPEG without AIGC", info)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadPngWithoutAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("png_empty.png")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNull("Should return null for PNG without AIGC", info)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadWebpWithoutAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("webp_empty.webp")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNull("Should return null for WebP without AIGC", info)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadHeifWithoutAIGC() {
        val file = TestResourceHelper.copyResourceToTempFile("heif_empty.heif")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNull("Should return null for HEIF without AIGC", info)
        
        // 清理
        file.delete()
    }
    
    @Test
    fun testReadAllFormatsWithoutAIGC() {
        val testFiles = listOf(
            "jpg_empty.jpg",
            "png_empty.png",
            "webp_empty.webp",
            "heif_empty.heif"
        )
        
        testFiles.forEach { fileName ->
            val file = TestResourceHelper.copyResourceToTempFile(fileName)
            val info = readerUtils.readFromFilePath(file.absolutePath)
            
            assertNull("Should return null for $fileName without AIGC", info)
            
            file.delete()
        }
    }
    
    @Test
    fun testReadNonExistentFile() {
        val info = readerUtils.readFromFilePath("/non/existent/file.jpg")
        
        assertNull("Should return null for non-existent file", info)
    }
    
    @Test
    fun testReadInvalidPath() {
        val info = readerUtils.readFromFilePath("")
        
        assertNull("Should return null for empty path", info)
    }
    
    @Test
    fun testReadUnsupportedFormat() {
        // 创建一个不支持的格式文件（BMP）
        val tempFile = File(context.cacheDir, "test_unsupported.bmp")
        try {
            tempFile.writeBytes(byteArrayOf(0x42, 0x4D, 0x00, 0x00)) // BMP header
            val info = readerUtils.readFromFilePath(tempFile.absolutePath)
            
            assertNull("Should return null for unsupported format", info)
        } finally {
            tempFile.delete()
        }
    }
    
    @Test
    fun testReadCorruptedFile() {
        // 创建一个损坏的 JPEG 文件
        val tempFile = File(context.cacheDir, "test_corrupted.jpg")
        try {
            tempFile.writeBytes(byteArrayOf(0xFF.toByte(), 0xD8.toByte(), 0x00, 0x00)) // 不完整的 JPEG
            val info = readerUtils.readFromFilePath(tempFile.absolutePath)
            
            assertNull("Should return null for corrupted file", info)
        } finally {
            tempFile.delete()
        }
    }
    
    // ========================================================================
    // 额外测试：验证读取的数据完整性
    // ========================================================================
    
    @Test
    fun testReadAIGCInfoFields() {
        val file = TestResourceHelper.copyResourceToTempFile("jpg_with_xmp.jpg")
        val info = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNotNull("Info should not be null", info)
        
        // 验证至少 label 字段有值
        assertNotNull("Label should not be null", info?.label)
        assertFalse("Label should not be empty", info?.label?.isEmpty() ?: true)
        
        file.delete()
    }
    
    @Test
    fun testMultipleReadsFromSameFile() {
        val file = TestResourceHelper.copyResourceToTempFile("jpg_with_xmp.jpg")
        
        // 多次读取同一个文件
        val info1 = readerUtils.readFromFilePath(file.absolutePath)
        val info2 = readerUtils.readFromFilePath(file.absolutePath)
        
        assertNotNull("First read should succeed", info1)
        assertNotNull("Second read should succeed", info2)
        assertEquals("Both reads should return same data", info1, info2)
        
        file.delete()
    }
    
    @Test
    fun testReadMultipleFilesSequentially() {
        val files = listOf(
            "jpg_with_xmp.jpg",
            "png_with_xmp.png",
            "webp_with_xmp.webp"
        )
        
        files.forEach { fileName ->
            val file = TestResourceHelper.copyResourceToTempFile(fileName)
            val info = readerUtils.readFromFilePath(file.absolutePath)
            
            assertNotNull("Should read from $fileName", info)
            file.delete()
        }
    }
}

