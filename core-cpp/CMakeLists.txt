
add_library(gimt_core_cpp STATIC
        src/gimt_jpeg_aigc_reader.cpp
        src/gimt_jpeg_aigc_writer.cpp
)
target_include_directories(gimt_core_cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)