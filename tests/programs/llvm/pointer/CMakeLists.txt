
add_executable (pointer_tests pointer.cpp )
target_link_libraries(pointer_tests PUBLIC minimclib doctest)
add_test(NAME PRGM_POINTER COMMAND pointer_tests)
