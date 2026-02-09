package com.jiemo.gimt_native

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File

/**
 * 演示如何在 androidTest 中使用项目根目录 /resource 下的测试资源
 */
@RunWith(AndroidJUnit4::class)
class ResourceAccessTest {
    
    @Test
    fun testListAllResources() {
        // 列出所有可用的测试资源
        val resources = TestResourceHelper.listAllResources()
        println("Available test resources: $resources")
        
        // 验证预期的文件存在
        assertTrue("Should contain jpg_empty.jpg", resources.contains("jpg_empty.jpg"))
        assertTrue("Should contain png_with_xmp.png", resources.contains("png_with_xmp.png"))
    }
    
    @Test
    fun testReadResourceAsStream() {
        // 方式 1: 读取为 InputStream
        val inputStream = TestResourceHelper.getResourceAsStream("jpg_empty.jpg")
        assertNotNull("InputStream should not be null", inputStream)
        
        // 验证文件不为空
        val bytes = inputStream.readBytes()
        assertTrue("File should not be empty", bytes.isNotEmpty())
        
        // 验证 JPEG 文件头 (FF D8 FF)
        assertEquals(0xFF.toByte(), bytes[0])
        assertEquals(0xD8.toByte(), bytes[1])
        assertEquals(0xFF.toByte(), bytes[2])
    }
    
    @Test
    fun testReadResourceAsBytes() {
        // 方式 2: 直接读取为 ByteArray
        val bytes = TestResourceHelper.getResourceAsBytes("png_empty.png")
        assertTrue("PNG file should not be empty", bytes.isNotEmpty())
        
        // 验证 PNG 文件头 (89 50 4E 47)
        assertEquals(0x89.toByte(), bytes[0])
        assertEquals(0x50.toByte(), bytes[1])
        assertEquals(0x4E.toByte(), bytes[2])
        assertEquals(0x47.toByte(), bytes[3])
    }
    
    @Test
    fun testCopyResourceToTempFile() {
        // 方式 3: 复制到临时文件（适用于需要文件路径的场景，如 JNI）
        val tempFile = TestResourceHelper.copyResourceToTempFile("webp_with_xmp.webp")
        
        assertTrue("Temp file should exist", tempFile.exists())
        assertTrue("Temp file should not be empty", tempFile.length() > 0)
        
        // 可以获取文件路径传给 JNI 方法
        val filePath = tempFile.absolutePath
        println("Temp file path: $filePath")
        
        // 验证 WebP 文件头 (RIFF)
        val bytes = tempFile.readBytes()
        assertEquals('R'.code.toByte(), bytes[0])
        assertEquals('I'.code.toByte(), bytes[1])
        assertEquals('F'.code.toByte(), bytes[2])
        assertEquals('F'.code.toByte(), bytes[3])
        
        // 清理临时文件
        tempFile.delete()
    }
    
    @Test
    fun testResourceExists() {
        // 检查文件是否存在
        assertTrue(TestResourceHelper.resourceExists("heif_empty.heif"))
        assertTrue(TestResourceHelper.resourceExists("heif_with_xmp.heif"))
        assertFalse(TestResourceHelper.resourceExists("non_existent_file.jpg"))
    }
    
    @Test
    fun testAllTestResources() {
        // 测试所有预期的资源文件都可以访问
        val expectedFiles = listOf(
            "heif_empty.heif",
            "heif_with_xmp.heif",
            "jpg_empty.jpg",
            "jpg_with_xmp.jpg",
            "png_empty.png",
            "png_with_xmp.png",
            "webp_empty.webp",
            "webp_with_xmp.webp"
        )
        
        expectedFiles.forEach { fileName ->
            assertTrue("$fileName should exist", TestResourceHelper.resourceExists(fileName))
            val bytes = TestResourceHelper.getResourceAsBytes(fileName)
            assertTrue("$fileName should not be empty", bytes.isNotEmpty())
            println("✓ $fileName: ${bytes.size} bytes")
        }
    }
    
    @Test
    fun testJniUsageExample() {
        // 示例：如何在 JNI 测试中使用
        val testFile = TestResourceHelper.copyResourceToTempFile("jpg_with_xmp.jpg")
        
        try {
            // 假设你有一个 JNI 方法需要文件路径
            // val result = YourNativeClass.processImage(testFile.absolutePath)
            // assertEquals(expectedValue, result)
            
            // 或者传递字节数组
            // val bytes = testFile.readBytes()
            // val result = YourNativeClass.processImageBytes(bytes)
            
            println("File ready for JNI processing: ${testFile.absolutePath}")
            assertTrue(testFile.exists())
            
        } finally {
            // 清理
            testFile.delete()
        }
    }
}

