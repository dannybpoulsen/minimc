include(GoogleTest)
find_package(GTest)

add_executable (ssamap ssamap.cpp)
target_link_libraries (ssamap minimclib  ${GTEST_BOTH_LIBRARIES})

gtest_discover_tests(ssamap PROPERTIES  LABELS unit)
