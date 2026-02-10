package com.jiemo.gimt_native

import com.jiemo.gimt_api.AIGCInfo
import com.jiemo.gimt_api.AIGCWriterUtils

/**
 * Native AIGC Writer 实现
 * 通过 JNI 调用 C++ 层的 core-cpp 库来写入 AIGC 元数据
 */
class NativeAIGCWriterUtils : AIGCWriterUtils {
    
    companion object {
        init {
            // 加载 native 库
            System.loadLibrary("gimt_native")
        }
    }
    
    /**
     * Native 方法：写入 AIGC 信息
     * 
     * @param inputPath 输入文件路径
     * @param outputPath 输出文件路径
     * @param jsonInfo AIGC 信息的 JSON 字符串
     * @return 成功返回 true，失败返回 false
     */
    private external fun nativeWriteAIGC(
        inputPath: String,
        outputPath: String,
        jsonInfo: String
    ): Boolean
    
    /**
     * Native 方法：获取支持的格式列表
     * 
     * @return 支持的格式数组
     */
    private external fun nativeGetSupportedFormats(): Array<String>
    
    /**
     * 将 AIGC 信息写入图像文件
     * 
     * @param inputPath 输入图像文件路径
     * @param outputPath 输出图像文件路径
     * @param aigcInfo 要写入的 AIGC 信息
     * @return 如果写入成功返回 true，否则返回 false
     */
    override fun writeAIGC(inputPath: String, outputPath: String, aigcInfo: AIGCInfo): Boolean {
        return try {
            // 将 AIGCInfo 转换为 JSON 字符串
            val json = aigcInfo.toJson()
            
            // 调用 native 方法写入
            nativeWriteAIGC(inputPath, outputPath, json)
        } catch (e: Exception) {
            e.printStackTrace()
            false
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




