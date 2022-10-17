
add_executable (symbol symbol.cpp)
target_link_libraries (symbol doctest minimclib)
add_test(NAME MODEL_SYMBOL COMMAND symbol)

add_subdirectory (analysis)
