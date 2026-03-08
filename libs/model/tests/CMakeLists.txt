
add_executable (symbol symbol.cpp)
target_link_libraries (symbol doctest model)

add_executable (simplifier simplifier.cpp)
target_link_libraries (simplifier doctest model)


add_test(NAME MODEL_SYMBOL COMMAND symbol)
add_test(NAME MODEL_SIMPLIFY COMMAND simplifier)

add_subdirectory (analysis)


add_executable (visitor visitor.cpp)
target_link_libraries (visitor model)
