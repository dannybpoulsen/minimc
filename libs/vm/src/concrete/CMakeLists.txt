add_library (vm_concrete STATIC)
target_sources (vm_concrete PRIVATE valuet.cpp memoryt.cpp)
target_include_directories (vm_concrete PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (vm_concrete PUBLIC model)
