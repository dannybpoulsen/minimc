add_library (llvmloader OBJECT loader.cpp context.cpp)
target_link_libraries (llvmloader PUBLIC support model util PUBLIC llvm13)
target_include_directories(llvmloader PRIVATE ${PROJECT_SOURCE_DIR}/include)
