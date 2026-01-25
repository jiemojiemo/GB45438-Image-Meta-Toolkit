add_subdirectory(googletests EXCLUDE_FROM_ALL)

add_executable(gimt_unit_tests
        test_binary_reader.cpp
)
target_link_libraries(gimt_unit_tests
    PRIVATE
        gmock_main
        gimt_core_cpp
)

add_test(NAME gimt_unit_tests COMMAND gimt_unit_tests)