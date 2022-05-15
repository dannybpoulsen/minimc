
add_library (llvmloader OBJECT loader.cpp llvminstructioncreator.cpp)
target_include_directories (llvmloader  PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries (llvmloader PUBLIC support model util PUBLIC llvm13)
target_include_directories(llvmloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
