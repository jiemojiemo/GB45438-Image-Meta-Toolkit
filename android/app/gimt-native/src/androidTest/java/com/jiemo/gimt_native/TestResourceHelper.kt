package com.jiemo.gimt_native

import android.content.Context
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.io.InputStream

/**
 * 测试资源辅助类
 * 用于在 androidTest 中访问项目根目录 /resource 下的测试文件
 */
object TestResourceHelper {
    
    /**
     * 获取测试上下文
     */
    val context: Context
        get() = InstrumentationRegistry.getInstrumentation().context
    
    /**
     * 从 assets 中读取文件为 InputStream
     * @param fileName 文件名，例如 "jpg_with_xmp.jpg"
     */
    fun getResourceAsStream(fileName: String): InputStream {
        return context.assets.open(fileName)
    }
    
    /**
     * 从 assets 中读取文件为 ByteArray
     * @param fileName 文件名，例如 "png_empty.png"
     */
    fun getResourceAsBytes(fileName: String): ByteArray {
        return getResourceAsStream(fileName).use { it.readBytes() }
    }
    
    /**
     * 将 assets 中的文件复制到临时文件
     * 适用于需要文件路径的场景（如 JNI 方法需要文件路径）
     * @param fileName 文件名
     * @return 临时文件对象
     */
    fun copyResourceToTempFile(fileName: String): File {
        val tempFile = File(context.cacheDir, fileName)
        getResourceAsStream(fileName).use { input ->
            tempFile.outputStream().use { output ->
                input.copyTo(output)
            }
        }
        return tempFile
    }
    
    /**
     * 列出所有可用的测试资源文件
     */
    fun listAllResources(): List<String> {
        return context.assets.list("")?.toList() ?: emptyList()
    }
    
    /**
     * 检查资源文件是否存在
     */
    fun resourceExists(fileName: String): Boolean {
        return try {
            context.assets.open(fileName).close()
            true
        } catch (e: Exception) {
            false
        }
    }
}




