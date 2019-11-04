include(GoogleTest)
find_package(GTest)
add_executable (output output.cpp)
add_executable (divisions divisions.cpp)
add_executable (sdivisions sdivisions.cpp)
add_executable (rshifts shifts.cpp)


target_link_libraries (output minimclib)
target_link_libraries (divisions minimclib  ${GTEST_BOTH_LIBRARIES})
target_link_libraries (sdivisions  minimclib ${GTEST_BOTH_LIBRARIES})
target_link_libraries (rshifts  minimclib ${GTEST_BOTH_LIBRARIES})


gtest_discover_tests(divisions)
gtest_discover_tests(sdivisions)
gtest_discover_tests(rshifts)
