package com.jiemo.gimt_native

import com.jiemo.gimt_api.AIGCInfo
import com.jiemo.gimt_api.AIGCReaderUtils

/**
 * Native AIGC Reader 实现
 * 通过 JNI 调用 C++ 层的 core-cpp 库来读取 AIGC 元数据
 */
class NativeAIGCReaderUtils : AIGCReaderUtils {
    
    companion object {
        init {
            // 加载 native 库
            System.loadLibrary("gimt_native")
        }
    }
    
    /**
     * Native 方法：读取 AIGC 信息
     * 
     * @param filePath 文件路径
     * @return JSON 字符串，失败返回 null
     */
    private external fun nativeReadAIGCInfo(filePath: String): String?
    
    /**
     * Native 方法：获取支持的格式列表
     * 
     * @return 支持的格式数组
     */
    private external fun nativeGetSupportedFormats(): Array<String>
    
    /**
     * 从文件路径读取 AIGC 信息
     * 
     * @param filePath 图像文件路径
     * @return AIGCInfo 对象，如果读取失败或文件不包含 AIGC 信息则返回 null
     */
    override fun readFromFilePath(filePath: String): AIGCInfo? {
        try {
            // 调用 native 方法获取 JSON 字符串
            val json = nativeReadAIGCInfo(filePath) ?: return null
            
            // 解析 JSON 为 AIGCInfo 对象
            val info = AIGCInfo.fromJson(json)
            
            // 如果解析后的对象为空，返回 null
            return if (info.isEmpty()) null else info
        } catch (e: Exception) {
            e.printStackTrace()
            return null
        }
    }
    
    /**
     * 获取支持的图像格式列表
     * 
     * @return 支持的图像格式列表（如 "jpg", "png", "webp", "heif"）
     */
    override fun getSupportedFormats(): List<String> {
        return try {
            nativeGetSupportedFormats().toList()
        } catch (e: Exception) {
            e.printStackTrace()
            emptyList()
        }
    }
}

