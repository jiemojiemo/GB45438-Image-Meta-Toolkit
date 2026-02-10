package com.jiemo.gimt_api

import com.google.gson.Gson
import com.google.gson.annotations.SerializedName

/**
 * AIGC 信息数据类
 * 对应 C/C++ 层的 AIGCInfo 类
 * 
 * 该类用于存储和传输 AIGC（AI Generated Content）相关的元数据信息，
 * 支持 JSON 序列化和反序列化，与 C++ 层保持字段名一致（首字母大写）
 * 
 * @property label 标识
 * @property contentProducer 内容生成者
 * @property produceID 生成者 ID
 * @property reservedCode1 预留字段 1
 * @property contentPropagator 内容传播者
 * @property propagateID 传播者 ID
 * @property reservedCode2 预留字段 2
 */
data class AIGCInfo(
    @SerializedName("Label")
    val label: String = "",
    
    @SerializedName("ContentProducer")
    val contentProducer: String = "",
    
    @SerializedName("ProduceID")
    val produceID: String = "",
    
    @SerializedName("ReservedCode1")
    val reservedCode1: String = "",
    
    @SerializedName("ContentPropagator")
    val contentPropagator: String = "",
    
    @SerializedName("PropagateID")
    val propagateID: String = "",
    
    @SerializedName("ReservedCode2")
    val reservedCode2: String = ""
) {
    
    companion object {
        private val gson = Gson()
        
        /**
         * 从 JSON 字符串反序列化为 AIGCInfo 对象
         * 
         * @param json JSON 字符串
         * @return AIGCInfo 对象，如果解析失败则返回空对象
         */
        fun fromJson(json: String): AIGCInfo {
            return try {
                gson.fromJson(json, AIGCInfo::class.java) ?: AIGCInfo()
            } catch (e: Exception) {
                // 解析失败时返回空对象
                AIGCInfo()
            }
        }
    }
    
    /**
     * 将 AIGCInfo 对象序列化为 JSON 字符串
     * 
     * @return JSON 字符串
     */
    fun toJson(): String {
        return gson.toJson(this)
    }
    
    /**
     * 判断当前对象是否为空（所有字段都为空字符串）
     * 
     * @return 如果所有字段都为空则返回 true，否则返回 false
     */
    fun isEmpty(): Boolean {
        return label.isEmpty() &&
                contentProducer.isEmpty() &&
                produceID.isEmpty() &&
                reservedCode1.isEmpty() &&
                contentPropagator.isEmpty() &&
                propagateID.isEmpty() &&
                reservedCode2.isEmpty()
    }
    
    /**
     * 判断当前对象是否有效（至少有一个字段不为空）
     * 
     * @return 如果至少有一个字段不为空则返回 true，否则返回 false
     */
    fun isValid(): Boolean {
        return !isEmpty()
    }
}




