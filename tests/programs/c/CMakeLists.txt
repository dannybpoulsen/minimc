find_program(CLANG clang)

if (CLANG)
   function(clang_compile INP)
   add_custom_target("${INP}" ALL 
        COMMAND ${CLANG} -emit-llvm  -S ${CMAKE_CURRENT_SOURCE_DIR}/${INP}.c -o ${CMAKE_CURRENT_BINARY_DIR}/${INP}.ll		
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${INP}.c
        COMMENT "Compiling ${INP}.c " # Just for nice message during build
    )
endfunction()

clang_compile("array")
clang_compile("array2")
clang_compile("main")
clang_compile("multiply1")
clang_compile("multiply2")
clang_compile("switch")

add_executable(c_tests driver.cpp)
target_link_libraries(c_tests PUBLIC minimclib doctest )
add_test(NAME c_tests COMMAND c_tests)
target_compile_definitions (c_tests PUBLIC LOCATION="${CMAKE_CURRENT_BINARY_DIR}")

endif(CLANG)
