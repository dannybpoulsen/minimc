
find_package(LLVM CONFIG)

add_library (llvm13 INTERFACE)

if (NOT LLVM_FOUND) 
   MESSAGE (STATUS "No LLVM Found. Downloading")
   include(FetchContent)
   FetchContent_Declare(
   llvmext13
   URL https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.4/clang+llvm-16.0.4-x86_64-linux-gnu-ubuntu-22.04.tar.xz
   URL_HASH SHA256=fd464333bd55b482eb7385f2f4e18248eb43129a3cda4c0920ad9ac3c12bdacf
   )

   FetchContent_MakeAvailable (llvmext13)

   find_package(LLVM CONFIG
		  PATHS ${llvmext13_SOURCE_DIR}/lib/cmake/
		  NO_DEFAULT_PATH
		  		  )
  llvm_map_components_to_libnames(llvm_libs core irreader transformutils passes)
  target_include_directories(llvm13 INTERFACE ${LLVM_INCLUDE_DIRS})
  target_link_libraries(llvm13 INTERFACE ${llvm_libs})
else()
  find_program(LLVM_CONFIG_BINARY NAMES llvm-config)
  function (run_llvm_config params output_var) 
  execute_process(COMMAND "${LLVM_CONFIG_BINARY}" "${params}" 
      RESULT_VARIABLE _exit_code
      OUTPUT_VARIABLE BUF
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
   set(${output_var} ${BUF} PARENT_SCOPE)   	
 endfunction ()

 run_llvm_config ("--libs" "libnames")
 run_llvm_config ("--includedir" "includedir")
 run_llvm_config ("--libdir" "libdir")
 target_include_directories(llvm13 INTERFACE ${includedir})
 target_link_directories(llvm13 INTERFACE ${libdir})
 target_link_libraries(llvm13 INTERFACE ${libnames})
endif()
   

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
