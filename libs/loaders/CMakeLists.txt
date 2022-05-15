add_library (loader STATIC)
add_subdirectory (llvm)
target_link_libraries (loader PUBLIC llvmloader)
