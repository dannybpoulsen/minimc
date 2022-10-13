find_program(CLANG clang)

if (CLANG)


add_custom_target(testmodels)

function(clang_compile PREP INP)
    add_custom_target("${PREP}_${INP}" ALL 
        COMMAND ${CLANG} -emit-llvm -c -S ${CMAKE_CURRENT_SOURCE_DIR}/${INP}.c -o ${CMAKE_CURRENT_BINARY_DIR}/${INP}.ll
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${INP}.c
        COMMENT "Generate $PREP_${INP}" # Just for nice message during build
    )
endfunction()

function(add_c_test PREP INP)
	clang_compile (${PREP} ${INP})
	add_dependencies(testmodels "${PREP}_${INP}")
    add_ll_test (${PREP} ${INP})
endfunction ()	

function(add_ll_test PREP INP)
	configure_file (${INP}.ini.in ${INP}.ini)
	add_test (NAME ${PREP}_${INP}_t COMMAND minimc --config ${CMAKE_CURRENT_BINARY_DIR}/${INP}.ini) 
	set_tests_properties(${PREP}_${INP}_t PROPERTIES LABELS "model")
   
endfunction ()	

function(add_mmc_test PREP INP)
	configure_file (${INP}.ini.in ${INP}.ini)
	add_test (NAME ${PREP}_${INP}_t COMMAND minimc --config ${CMAKE_CURRENT_BINARY_DIR}/${INP}.ini)
endfunction()

add_subdirectory(dummy)
add_subdirectory(array)
add_subdirectory(llvm)
add_subdirectory(minimc)

endif(CLANG)
