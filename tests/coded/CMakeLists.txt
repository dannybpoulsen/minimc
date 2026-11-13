add_executable (instruction_tests instructions.cpp)
target_link_libraries(instruction_tests PUBLIC minimclib doctest)
add_test(NAME Instructions COMMAND instruction_tests)

add_executable (symb_eval_reg_tests symb_reg_eval.cpp)
target_link_libraries(symb_eval_reg_tests PUBLIC minimclib doctest)
add_test(NAME SymbRegEval COMMAND symb_eval_reg_tests)

add_executable (symb_memory_eval_tests symb_memory_eval.cpp)
target_link_libraries(symb_memory_eval_tests PUBLIC minimclib symb_vm doctest)
add_test(NAME SymbMemoryEval COMMAND symb_memory_eval_tests)
