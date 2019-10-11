find_package(LLVM REQUIRED CONFIG)
 message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
 message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
 llvm_map_components_to_libnames(llvm_libs  core irreader transformutils passes)

add_library (llvmloader INTERFACE)
target_sources (llvmloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/loader.cpp)
target_include_directories (llvmloader  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include INTERFACE ${LLVM_INCLUDE_DIRS} )
target_link_libraries (llvmloader INTERFACE model ${llvm_libs})
