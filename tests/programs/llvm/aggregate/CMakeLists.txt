add_ll_test("llvm" "insert_extract_nofai")
add_ll_test("llvm" "insert_extract_fail")
#add_ll_test("llvm" "load_store")

add_executable (aggregate_tests aggregate.cpp )
target_link_libraries(aggregate_tests PUBLIC minimclib doctest)
