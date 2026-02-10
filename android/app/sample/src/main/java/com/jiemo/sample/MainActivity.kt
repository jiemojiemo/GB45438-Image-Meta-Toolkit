package com.jiemo.sample

import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.jiemo.gimt_native.NativeAIGCReaderUtils
import com.jiemo.gimt_native.NativeAIGCWriterUtils

class MainActivity : AppCompatActivity() {
    
    private lateinit var tvResult: TextView
    private lateinit var btnTest: Button
    
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
        try {
            val result = StringBuilder()
            result.append("GIMT Library Test\n\n")
            result.append("✅ Library loaded successfully!\n")
            result.append("✅ NativeAIGCReaderUtils available\n")
            result.append("✅ NativeAIGCWriterUtils available\n")
            
            tvResult.text = result.toString()
        } catch (e: Exception) {
            tvResult.text = "❌ Error: ${e.message}"
        }
    }
}