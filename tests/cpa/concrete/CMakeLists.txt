include(GoogleTest)
find_package(GTest)
add_executable (tac tacimpl.cpp)

target_link_libraries (tac minimclib ${GTEST_BOTH_LIBRARIES})
target_include_directories (tac PUBLIC ${PROJECT_SOURCE_DIR}/libs/cpa/concrete)

gtest_discover_tests(tac)
