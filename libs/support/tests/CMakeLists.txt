add_executable (pipes pipes.cpp)
target_link_libraries (pipes doctest support)
add_test(NAME pipes COMMAND pipes)


add_executable (progress messages.cpp)
target_link_libraries (progress support)
