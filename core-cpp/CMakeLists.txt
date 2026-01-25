
add_library(gimt_core_cpp INTERFACE)
target_include_directories(gimt_core_cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)