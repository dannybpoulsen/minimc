
find_package(LLVM CONFIG)

add_library (llvm13 INTERFACE)

if (NOT LLVM_FOUND) 
   MESSAGE (STATUS "No LLVM Found. Downloading")
   include(FetchContent)
   FetchContent_Declare(
   llvmext13
   URL https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.1/clang+llvm-13.0.1-x86_64-linux-gnu-ubuntu-18.04.tar.xz
   URL_HASH SHA256=84a54c69781ad90615d1b0276a83ff87daaeded99fbc64457c350679df7b4ff0
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
  find_program(LLVM_CONFIG_BINARY
  NAMES llvm-config)
  message(STATUS "LLVM_CONFIG_BINARY: ${LLVM_CONFIG_BINARY}")				  
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
 run_llvm_config ("--liddir" "libdir")
 target_include_directories(llvm13 INTERFACE ${includedir})
 target_link_directories(llvm13 INTERFACE ${libdir})
 target_link_libraries(llvm13 INTERFACE ${libnames})
 		 
message (STATUS ${libnames})
endif()
   

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
