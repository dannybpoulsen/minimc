add_library (vm_concrete STATIC valuet.cpp memoryt.cpp)
target_link_libraries (vm_concrete PUBLIC model vm)
target_include_directories (vm_concrete PUBLIC ${PROJECT_SOURCE_DIR}/include)
