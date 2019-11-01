include(GoogleTest)
find_package(GTest)
add_executable (concrete_add add.cpp)

target_include_directories (concrete_add PUBLIC ${PROJECT_SOURCE_DIR}/libs/cpa/concrete_no_mem)
target_link_libraries (concrete_add minimclib ${GTEST_BOTH_LIBRARIES})
gtest_discover_tests(concrete_add
)
