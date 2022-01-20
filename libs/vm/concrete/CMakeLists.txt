add_library (vm_concrete STATIC)
target_sources (vm_concrete PUBLIC value.cpp valuet.cpp memoryt.cpp)
target_include_directories (vm_concrete PUBLIC ${PROJECT_SOURCE_DIR}/include)

