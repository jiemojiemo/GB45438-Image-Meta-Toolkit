package com.jiemo.gimt_api

/**
 * AIGC 读取工具接口
 * 提供从图像文件中读取 AIGC 元数据的能力
 */
interface AIGCReaderUtils {
    
    /**
     * 从文件路径读取 AIGC 信息
     * 
     * @param filePath 图像文件路径
     * @return AIGCInfo 对象，如果读取失败或文件不包含 AIGC 信息则返回 null
     */
    fun readFromFilePath(filePath: String): AIGCInfo?
    
    /**
     * 获取支持的图像格式列表
     * 
     * @return 支持的图像格式列表（如 "jpg", "png", "webp", "heif"）
     */
    fun getSupportedFormats(): List<String>
}

