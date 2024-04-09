add_executable (arith_tests driver.cpp )
target_link_libraries(arith_tests PUBLIC minimclib doctest)
add_test(NAME PRGM_ARITH COMMAND arith_tests)
