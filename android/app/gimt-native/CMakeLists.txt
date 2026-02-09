cmake_minimum_required(VERSION 3.22.1)

project("gimt_native")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 获取项目根目录（向上三级）
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

# 添加 core-cpp 的 include 目录
include_directories(${PROJECT_ROOT}/core-cpp/include)

# 查找 core-cpp 静态库
find_library(GIMT_CORE_LIB
    NAMES gimt_core_cpp libgimt_core_cpp.a
    PATHS ${PROJECT_ROOT}/core-cpp/build
          ${PROJECT_ROOT}/build/core-cpp
          ${PROJECT_ROOT}/cmake-build-debug/core-cpp
    NO_DEFAULT_PATH
)

# 如果找不到预编译的库，则添加 core-cpp 子项目
if(NOT GIMT_CORE_LIB)
    message(STATUS "Pre-built core-cpp library not found, building from source")
    add_subdirectory(${PROJECT_ROOT}/core-cpp ${CMAKE_CURRENT_BINARY_DIR}/core-cpp)
    set(GIMT_CORE_LIB gimt_core_cpp)
else()
    message(STATUS "Found core-cpp library: ${GIMT_CORE_LIB}")
endif()

# 添加 JNI 实现源文件
add_library(${CMAKE_PROJECT_NAME} SHARED
    src/main/cpp/native_aigc_reader.cpp
)

# 链接库
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${GIMT_CORE_LIB}
    android
    log
)

# 如果需要链接第三方库（png, jpeg, webp, heif），取决于 core-cpp 的依赖
# 这些库应该已经被 core-cpp 链接了

