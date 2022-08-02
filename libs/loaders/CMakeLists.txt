add_library (loader SHARED loader.cpp)
add_subdirectory (llvm)
target_link_libraries (loader PUBLIC llvmloader)
