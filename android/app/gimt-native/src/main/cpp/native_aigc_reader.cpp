#include <jni.h>
#include <string>
#include <android/log.h>
#include "gimt/gimt_aigc_reader_factory.h"
#include "gimt/gimt_def.h"

#define LOG_TAG "NativeAIGCReader"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

/**
 * JNI 函数：读取 AIGC 信息
 * 
 * @param env JNI 环境指针
 * @param thiz Java 对象引用
 * @param jFilePath 文件路径（Java String）
 * @return JSON 字符串（包含 AIGC 信息），失败返回 null
 */
JNIEXPORT jstring JNICALL
Java_com_jiemo_gimt_1native_NativeAIGCReaderUtils_nativeReadAIGCInfo(
    JNIEnv* env,
    jobject thiz,
    jstring jFilePath) {
    
    if (jFilePath == nullptr) {
        LOGE("File path is null");
        return nullptr;
    }
    
    // 转换 Java String 到 C++ string
    const char* filePath = env->GetStringUTFChars(jFilePath, nullptr);
    if (filePath == nullptr) {
        LOGE("Failed to get file path string");
        return nullptr;
    }
    
    std::string filepath(filePath);
    env->ReleaseStringUTFChars(jFilePath, filePath);
    
    LOGD("Reading AIGC info from: %s", filepath.c_str());
    
    // 使用 Factory 创建并准备 Reader（自动检测格式）
    auto reader = gimt::AIGCReaderFactory::createAndPrepare(filepath, true);
    if (!reader) {
        LOGE("Failed to create or prepare reader for: %s", filepath.c_str());
        return nullptr;
    }
    
    LOGD("Reader created successfully, format: %s", 
         gimt::getFormatName(reader->getFormat()).c_str());
    
    // 读取 AIGC 信息
    gimt::AIGCInfo info;
    if (!reader->readAIGCInfo(info)) {
        LOGD("No AIGC info found in file: %s", filepath.c_str());
        return nullptr;
    }
    
    // 转换为 JSON 字符串
    std::string json = info.toJson();
    LOGD("AIGC info read successfully: %s", json.c_str());
    
    return env->NewStringUTF(json.c_str());
}

/**
 * JNI 函数：获取支持的图像格式列表
 * 
 * @param env JNI 环境指针
 * @param thiz Java 对象引用
 * @return 字符串数组，包含所有支持的格式
 */
JNIEXPORT jobjectArray JNICALL
Java_com_jiemo_gimt_1native_NativeAIGCReaderUtils_nativeGetSupportedFormats(
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




