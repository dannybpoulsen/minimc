#add_subdirectory (aggregate)
#add_subdirectory (pointer)
#add_subdirectory (phi)

add_executable (llvm_tests driver.cpp )
target_link_libraries(llvm_tests PUBLIC minimclib doctest )
add_test(NAME LLVM_tests COMMAND llvm_tests)
