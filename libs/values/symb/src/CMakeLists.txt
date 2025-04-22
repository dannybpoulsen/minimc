add_library (symb_vm STATIC dummy.cpp)
target_link_libraries (symb_vm PUBLIC model vm)
target_include_directories (symb_vm PUBLIC ${PROJECT_SOURCE_DIR}/include)
