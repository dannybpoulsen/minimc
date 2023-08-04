add_executable (phi_tests phi.cpp )
target_link_libraries(phi_tests PUBLIC minimclib doctest)
add_test(NAME PRGM_PHI COMMAND phi_tests)
