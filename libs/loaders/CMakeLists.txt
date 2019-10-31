add_library (loader SHARED)
add_subdirectory (llvm)
target_link_libraries (loader PUBLIC llvmloader)
target_include_directories (loader PUBLIC include)
