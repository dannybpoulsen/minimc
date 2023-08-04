add_library (llvmloader SHARED loader.cpp context.cpp)
target_link_libraries (llvmloader PUBLIC loaderinter support model util PUBLIC llvm13)


INSTALL (TARGETS llvmloader)
