plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "com.jiemo.sample"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.jiemo.sample"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }
}

dependencies {

    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
    
    // 本地测试：使用项目依赖
//    implementation(project(":app:gimt-native"))
    
    // JitPack 测试：取消注释下面这行，注释掉上面的 project 依赖
    implementation("com.github.jiemojiemo:GB45438-Image-Meta-Toolkit:main-SNAPSHOT")
//    implementation("com.github.jiemojiemo.GB45438-Image-Meta-Toolkit:gimt-native:v1.0.0")
    
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}