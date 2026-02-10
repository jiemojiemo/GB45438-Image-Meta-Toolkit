# Android Instrumented 测试资源使用指南

## 概述

本模块的 `androidTest` 可以访问项目根目录 `/resource` 下的所有测试资源文件。

## 配置说明

在 `build.gradle.kts` 中已配置：

```kotlin
sourceSets {
    getByName("androidTest") {
        assets.srcDirs("../../../resource")
    }
}
```

这样配置后，`/resource` 目录下的所有文件会自动作为 assets 打包到测试 APK 中。

## 使用方法

### 方法 1：读取为 InputStream

```kotlin
val inputStream = TestResourceHelper.getResourceAsStream("jpg_with_xmp.jpg")
// 使用 inputStream...
```

### 方法 2：读取为 ByteArray

```kotlin
val bytes = TestResourceHelper.getResourceAsBytes("png_empty.png")
// 处理字节数组...
```

### 方法 3：复制到临时文件（推荐用于 JNI）

```kotlin
val tempFile = TestResourceHelper.copyResourceToTempFile("webp_with_xmp.webp")
try {
    // 传递文件路径给 JNI 方法
    val result = YourNativeClass.processImage(tempFile.absolutePath)
    // 或者读取字节数组
    val bytes = tempFile.readBytes()
} finally {
    tempFile.delete() // 清理临时文件
}
```

## 可用的测试资源

- `heif_empty.heif` - 空 HEIF 图片
- `heif_with_xmp.heif` - 包含 XMP 元数据的 HEIF 图片
- `jpg_empty.jpg` - 空 JPEG 图片
- `jpg_with_xmp.jpg` - 包含 XMP 元数据的 JPEG 图片
- `png_empty.png` - 空 PNG 图片
- `png_with_xmp.png` - 包含 XMP 元数据的 PNG 图片
- `webp_empty.webp` - 空 WebP 图片
- `webp_with_xmp.webp` - 包含 XMP 元数据的 WebP 图片

## 运行测试

```bash
# 运行所有 instrumented 测试
./gradlew :app:gimt-native:connectedAndroidTest

# 运行特定测试类
./gradlew :app:gimt-native:connectedAndroidTest --tests "*.ResourceAccessTest"

# 运行特定测试方法
./gradlew :app:gimt-native:connectedAndroidTest --tests "*.ResourceAccessTest.testReadResourceAsBytes"
```

## 注意事项

1. **必须在真机或模拟器上运行**：这些测试是 Instrumented 测试，不能在 JVM 上运行
2. **临时文件清理**：使用 `copyResourceToTempFile` 后记得删除临时文件
3. **文件路径**：临时文件会被复制到 `context.cacheDir`，路径类似 `/data/user/0/com.jiemo.gimt_native.test/cache/`
4. **资源同步**：修改 `/resource` 目录下的文件后，需要重新构建测试 APK

## JNI 测试示例

```kotlin
@Test
fun testNativeImageProcessing() {
    // 1. 准备测试文件
    val testFile = TestResourceHelper.copyResourceToTempFile("jpg_with_xmp.jpg")
    
    try {
        // 2. 调用 JNI 方法
        val metadata = NativeImageReader.readMetadata(testFile.absolutePath)
        
        // 3. 验证结果
        assertNotNull(metadata)
        assertTrue(metadata.hasXmp)
        
    } finally {
        // 4. 清理
        testFile.delete()
    }
}
```

## 其他模块使用

如果其他模块（如 `gimt-api`）也需要使用这些测试资源，在对应的 `build.gradle.kts` 中添加相同的配置即可。




