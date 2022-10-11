add_library (loader SHARED loader.cpp)
add_subdirectory (llvm)
add_subdirectory (mmc)
target_link_libraries (loader PUBLIC minimcloader llvmloader)
