add_executable (hosttests hosttester.cpp  operations.cpp)
target_link_libraries (hosttests doctest host)
add_test(NAME hosttests COMMAND $<TARGET_NAME:hosttests>)
