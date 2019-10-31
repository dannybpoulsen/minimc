include(GoogleTest)
find_package(GTest)
add_executable (output output.cpp)
add_executable (divisions divisions.cpp)
add_executable (sdivisions sdivisions.cpp)

target_link_libraries (output minimclib)
target_link_libraries (divisions minimclib  ${GTEST_BOTH_LIBRARIES})
target_link_libraries (sdivisions  minimclib ${GTEST_BOTH_LIBRARIES})


gtest_add_tests(TARGET      divisions
)

gtest_add_tests(TARGET      sdivisions
)
