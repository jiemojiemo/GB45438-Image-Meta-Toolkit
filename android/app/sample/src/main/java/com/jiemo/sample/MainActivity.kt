package com.jiemo.sample

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.MediaStore
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.content.FileProvider
import com.jiemo.gimt_api.AIGCInfo
import com.jiemo.gimt_native.NativeAIGCReaderUtils
import com.jiemo.gimt_native.NativeAIGCWriterUtils
import java.io.File
import java.io.FileOutputStream

class MainActivity : AppCompatActivity() {
    
    private lateinit var tvResult: TextView
    private lateinit var btnTest: Button
    
    private val reader = NativeAIGCReaderUtils()
    private val writer = NativeAIGCWriterUtils()
    
    // 图片选择器
    private val pickImageLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == RESULT_OK) {
            result.data?.data?.let { uri ->
                handleSelectedImage(uri)
            }
        }
    }
    
    // 权限请求
    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted ->
        if (isGranted) {
            openImagePicker()
        } else {
            Toast.makeText(this, "需要存储权限才能选择图片", Toast.LENGTH_SHORT).show()
        }
    }
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        tvResult = findViewById(R.id.tv_result)
        btnTest = findViewById(R.id.btn_test)
        
        btnTest.setOnClickListener {
            testGimtLibrary()
        }
    }
    
    private fun testGimtLibrary() {
        // 检查权限并打开图片选择器
        if (checkStoragePermission()) {
            openImagePicker()
        } else {
            requestStoragePermission()
        }
    }
    
    private fun checkStoragePermission(): Boolean {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.READ_MEDIA_IMAGES
            ) == PackageManager.PERMISSION_GRANTED
        } else {
            ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.READ_EXTERNAL_STORAGE
            ) == PackageManager.PERMISSION_GRANTED
        }
    }
    
    private fun requestStoragePermission() {
        val permission = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            Manifest.permission.READ_MEDIA_IMAGES
        } else {
            Manifest.permission.READ_EXTERNAL_STORAGE
        }
        requestPermissionLauncher.launch(permission)
    }
    
    private fun openImagePicker() {
        val intent = Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI)
        pickImageLauncher.launch(intent)
    }
    
    private fun handleSelectedImage(uri: Uri) {
        try {
            val result = StringBuilder()
            result.append("=== GIMT Library 测试 ===\n\n")
            
            // 1. 获取图片的真实路径
            val imagePath = getRealPathFromUri(uri)
            if (imagePath == null) {
                result.append("❌ 无法获取图片路径\n")
                tvResult.text = result.toString()
                return
            }
            
            result.append("📁 图片路径:\n$imagePath\n\n")
            
            // 2. 读取 AIGC 信息
            result.append("--- 读取 AIGC 信息 ---\n")
            val aigcInfo = reader.readFromFilePath(imagePath)
            
            if (aigcInfo != null && aigcInfo.isValid()) {
                result.append("✅ 成功读取 AIGC 信息:\n")
                result.append("  Label: ${aigcInfo.label}\n")
                result.append("  ContentProducer: ${aigcInfo.contentProducer}\n")
                result.append("  ProduceID: ${aigcInfo.produceID}\n")
                result.append("  ReservedCode1: ${aigcInfo.reservedCode1}\n")
                result.append("  ContentPropagator: ${aigcInfo.contentPropagator}\n")
                result.append("  PropagateID: ${aigcInfo.propagateID}\n")
                result.append("  ReservedCode2: ${aigcInfo.reservedCode2}\n\n")
            } else {
                result.append("ℹ️ 图片不包含 AIGC 信息\n\n")
            }
            
            // 3. 写入 fake AIGC 信息
            result.append("--- 写入 Fake AIGC 信息 ---\n")
            val fakeAigcInfo = AIGCInfo(
                label = "AI_GENERATED",
                contentProducer = "TestProducer",
                produceID = "PROD_12345",
                reservedCode1 = "RESERVED_1",
                contentPropagator = "TestPropagator",
                propagateID = "PROP_67890",
                reservedCode2 = "RESERVED_2"
            )
            
            // 保存到 cache 目录
            val cacheDir = cacheDir
            val outputFile = File(cacheDir, "aigc_test_${System.currentTimeMillis()}.jpg")
            
            // 先复制原图到 cache 目录
            copyFile(imagePath, outputFile.absolutePath)
            
            // 写入 AIGC 信息
            val writeSuccess = writer.writeAIGC(
                outputFile.absolutePath,
                outputFile.absolutePath,
                fakeAigcInfo
            )
            
            if (writeSuccess) {
                result.append("✅ 成功写入 AIGC 信息\n")
                result.append("📁 输出路径:\n${outputFile.absolutePath}\n\n")
                
                // 验证写入
                result.append("--- 验证写入结果 ---\n")
                val verifyInfo = reader.readFromFilePath(outputFile.absolutePath)
                if (verifyInfo != null && verifyInfo.isValid()) {
                    result.append("✅ 验证成功，读取到写入的信息:\n")
                    result.append("  Label: ${verifyInfo.label}\n")
                    result.append("  ContentProducer: ${verifyInfo.contentProducer}\n")
                    result.append("  ProduceID: ${verifyInfo.produceID}\n")
                } else {
                    result.append("⚠️ 验证失败，无法读取写入的信息\n")
                }
            } else {
                result.append("❌ 写入 AIGC 信息失败\n")
            }
            
            tvResult.text = result.toString()
            
        } catch (e: Exception) {
            tvResult.text = "❌ 错误: ${e.message}\n${e.stackTraceToString()}"
        }
    }
    
    private fun getRealPathFromUri(uri: Uri): String? {
        // 尝试直接获取路径
        if (uri.scheme == "file") {
            return uri.path
        }
        
        // 从 content:// URI 获取路径
        val projection = arrayOf(MediaStore.Images.Media.DATA)
        contentResolver.query(uri, projection, null, null, null)?.use { cursor ->
            if (cursor.moveToFirst()) {
                val columnIndex = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA)
                return cursor.getString(columnIndex)
            }
        }
        
        // 如果无法获取路径，复制到临时文件
        return copyUriToTempFile(uri)
    }
    
    private fun copyUriToTempFile(uri: Uri): String? {
        return try {
            val tempFile = File(cacheDir, "temp_image_${System.currentTimeMillis()}.jpg")
            contentResolver.openInputStream(uri)?.use { input ->
                FileOutputStream(tempFile).use { output ->
                    input.copyTo(output)
                }
            }
            tempFile.absolutePath
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }
    
    private fun copyFile(sourcePath: String, destPath: String) {
        File(sourcePath).copyTo(File(destPath), overwrite = true)
    }
}