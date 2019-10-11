add_library (loader INTERFACE)
add_subdirectory (llvm)
target_link_libraries (loader INTERFACE llvmloader)
target_include_directories (loader INTERFACE include)
