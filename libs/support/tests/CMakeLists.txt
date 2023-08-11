add_executable (pipes pipes.cpp)
target_link_libraries (pipes doctest support)
add_test(NAME pipes COMMAND pipes)
