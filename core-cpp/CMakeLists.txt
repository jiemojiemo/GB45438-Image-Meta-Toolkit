find_package(ZLIB REQUIRED)

add_library(gimt_core_cpp STATIC
        src/gimt_jpeg_aigc_reader.cpp
        src/gimt_jpeg_aigc_writer.cpp
        src/gimt_png_aigc_reader.cpp
        src/gimt_png_aigc_writer.cpp
        src/gimt_webp_aigc_reader.cpp
        src/gimt_webp_aigc_writer.cpp
        src/gimt_xml_utils.cpp
)
target_include_directories(gimt_core_cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(gimt_core_cpp PRIVATE ZLIB::ZLIB)

add_subdirectory(tests)