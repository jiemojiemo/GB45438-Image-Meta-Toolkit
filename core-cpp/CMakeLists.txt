find_package(ZLIB REQUIRED)

add_library(gimt_core_cpp STATIC
        src/gimt_jpeg_aigc_reader.cpp
        src/gimt_jpeg_aigc_writer.cpp
        src/gimt_png_aigc_reader.cpp
        src/gimt_png_aigc_writer.cpp
        src/gimt_webp_aigc_reader.cpp
        src/gimt_webp_aigc_writer.cpp
        src/gimt_heif_aigc_reader.cpp
        src/gimt_heif_aigc_writer.cpp
        src/gimt_xml_utils.cpp
        src/gimt_image_format.cpp
        src/gimt_aigc_reader_factory.cpp
        src/gimt_aigc_writer_factory.cpp
)
target_include_directories(gimt_core_cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(gimt_core_cpp PRIVATE ZLIB::ZLIB)

add_subdirectory(tests)