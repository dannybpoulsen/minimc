add_executable (path path.cpp)
target_link_libraries (path doctest minimclib)
add_test(NAME VM_PATH COMMAND path)


