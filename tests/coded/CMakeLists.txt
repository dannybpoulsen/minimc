add_executable (instruction_tests instructions.cpp)
target_link_libraries(instruction_tests PUBLIC minimclib doctest)
add_test(NAME Instructions COMMAND instruction_tests)
