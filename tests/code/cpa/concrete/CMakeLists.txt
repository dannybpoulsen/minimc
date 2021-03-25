include(GoogleTest)
find_package(GTest)
add_executable (tac tacimpl.cpp)
add_executable (cmp cmpimpl.cpp)


target_link_libraries (tac minimclib ${GTEST_BOTH_LIBRARIES})
target_include_directories (tac PUBLIC ${PROJECT_SOURCE_DIR}/libs/cpa/concrete)

target_link_libraries (cmp minimclib ${GTEST_BOTH_LIBRARIES})
target_include_directories (cmp PUBLIC ${PROJECT_SOURCE_DIR}/libs/cpa/concrete)


gtest_discover_tests(tac PROPERTIES  LABELS unit)
gtest_discover_tests(cmp PROPERTIES  LABELS unit)
