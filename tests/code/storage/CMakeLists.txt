include(GoogleTest)
find_package(GTest)


add_executable (store store.cpp)

target_link_libraries (store minimclib ${GTEST_BOTH_LIBRARIES})

gtest_discover_tests(store PROPERTIES  LABELS unit)
