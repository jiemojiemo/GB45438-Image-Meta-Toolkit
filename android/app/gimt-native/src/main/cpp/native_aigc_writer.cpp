#include <jni.h>
#include <string>
#include <android/log.h>
#include "gimt/gimt_aigc_writer_factory.h"
#include "gimt/gimt_def.h"

#define LOG_TAG "NativeAIGCWriter"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

/**
 * JNI 函数：写入 AIGC 信息
 * 
 * @param env JNI 环境指针
 * @param thiz Java 对象引用
 * @param jInputPath 输入文件路径（Java String）
 * @param jOutputPath 输出文件路径（Java String）
 * @param jJsonInfo AIGC 信息的 JSON 字符串（Java String）
 * @return 成功返回 true，失败返回 false
 */
JNIEXPORT jboolean JNICALL
Java_com_jiemo_gimt_1native_NativeAIGCWriterUtils_nativeWriteAIGC(
    JNIEnv* env,
    jobject thiz,
    jstring jInputPath,
    jstring jOutputPath,
    jstring jJsonInfo) {
    
    if (jInputPath == nullptr || jOutputPath == nullptr || jJsonInfo == nullptr) {
        LOGE("Input parameters are null");
        return JNI_FALSE;
    }
    
    // 转换 Java String 到 C++ string
    const char* inputPath = env->GetStringUTFChars(jInputPath, nullptr);
    const char* outputPath = env->GetStringUTFChars(jOutputPath, nullptr);
    const char* jsonInfo = env->GetStringUTFChars(jJsonInfo, nullptr);
    
    if (inputPath == nullptr || outputPath == nullptr || jsonInfo == nullptr) {
        LOGE("Failed to get string chars");
        if (inputPath) env->ReleaseStringUTFChars(jInputPath, inputPath);
        if (outputPath) env->ReleaseStringUTFChars(jOutputPath, outputPath);
        if (jsonInfo) env->ReleaseStringUTFChars(jJsonInfo, jsonInfo);
        return JNI_FALSE;
    }
    
    std::string inputFilepath(inputPath);
    std::string outputFilepath(outputPath);
    std::string json(jsonInfo);
    
    // 释放 JNI 字符串
    env->ReleaseStringUTFChars(jInputPath, inputPath);
    env->ReleaseStringUTFChars(jOutputPath, outputPath);
    env->ReleaseStringUTFChars(jJsonInfo, jsonInfo);
    
    LOGD("Writing AIGC info from: %s to: %s", inputFilepath.c_str(), outputFilepath.c_str());
    
    // 使用 Factory 创建并准备 Writer（自动检测格式）
    auto writer = gimt::AIGCWriterFactory::createAndPrepare(inputFilepath, outputFilepath, true);
    if (!writer) {
        LOGE("Failed to create or prepare writer for: %s", inputFilepath.c_str());
        return JNI_FALSE;
    }
    
    LOGD("Writer created successfully, format: %s", 
         gimt::getFormatName(writer->getFormat()).c_str());
    
    // 解析 JSON 为 AIGCInfo
    gimt::AIGCInfo info;
    gimt::AIGCInfo::parseJsonToStruct(json, info);
    
    LOGD("Parsed AIGC info - Label: %s, Producer: %s", 
         info.label.c_str(), info.contentProducer.c_str());
    
    // 写入 AIGC 信息
    if (!writer->writeAIGCInfo(info)) {
        LOGE("Failed to write AIGC info to: %s", outputFilepath.c_str());
        return JNI_FALSE;
    }
    
    LOGD("AIGC info written successfully to: %s", outputFilepath.c_str());
    return JNI_TRUE;
}

/**
 * JNI 函数：获取支持的图像格式列表
 * 
 * @param env JNI 环境指针
 * @param thiz Java 对象引用
 * @return 字符串数组，包含所有支持的格式
 */
JNIEXPORT jobjectArray JNICALL
Java_com_jiemo_gimt_1native_NativeAIGCWriterUtils_nativeGetSupportedFormats(
    JNIEnv* env,
    jobject thiz) {
    
    // 支持的格式列表
    const char* formats[] = {"jpg", "jpeg", "png", "webp", "heif", "heic"};
    int count = sizeof(formats) / sizeof(formats[0]);
    
    // 创建 String 数组
    jclass stringClass = env->FindClass("java/lang/String");
    if (stringClass == nullptr) {
        LOGE("Failed to find String class");
        return nullptr;
    }
    
    jobjectArray result = env->NewObjectArray(count, stringClass, nullptr);
    if (result == nullptr) {
        LOGE("Failed to create object array");
        return nullptr;
    }
    
    // 填充数组
    for (int i = 0; i < count; i++) {
        jstring format = env->NewStringUTF(formats[i]);
        if (format == nullptr) {
            LOGE("Failed to create string for format: %s", formats[i]);
            return nullptr;
        }
        env->SetObjectArrayElement(result, i, format);
        env->DeleteLocalRef(format);
    }
    
    LOGD("Returning %d supported formats", count);
    return result;
}

} // extern "C"




