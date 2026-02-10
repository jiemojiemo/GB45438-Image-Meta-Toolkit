package com.jiemo.gimt_api

/**
 * AIGC 写入工具接口
 * 提供向图像文件写入 AIGC 元数据的能力
 */
interface AIGCWriterUtils {
    
    /**
     * 将 AIGC 信息写入图像文件
     * 
     * @param inputPath 输入图像文件路径
     * @param outputPath 输出图像文件路径
     * @param aigcInfo 要写入的 AIGC 信息
     * @return 如果写入成功返回 true，否则返回 false
     */
    fun writeAIGC(inputPath: String, outputPath: String, aigcInfo: AIGCInfo): Boolean
    
    /**
     * 获取支持的图像格式列表
     * 
     * @return 支持的图像格式列表（如 "jpg", "png", "webp", "heif"）
     */
    fun getSupportedFormats(): List<String>
}




