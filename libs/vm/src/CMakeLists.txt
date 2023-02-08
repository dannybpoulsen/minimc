add_library (vm_gen INTERFACE)
target_include_directories (vm_gen INTERFACE ${PROJECT_SOURCE_DIR}/include)

add_library (vm INTERFACE )
target_link_libraries (vm INTERFACE vm_gen)
