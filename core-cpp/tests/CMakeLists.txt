add_subdirectory(googletests EXCLUDE_FROM_ALL)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gimt_testing_global_paths.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/gimt_testing_global_paths.h)

add_executable(gimt_unit_tests
        test_binary_reader.cpp
        test_jpeg_aigc_reader.cpp
        test_patter_matcher.cpp
)
target_link_libraries(gimt_unit_tests
        PRIVATE
        gmock_main
        gimt_core_cpp
)

add_test(NAME gimt_unit_tests COMMAND gimt_unit_tests)