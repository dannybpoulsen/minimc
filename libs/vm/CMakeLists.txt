add_library (vm_gen STATIC vm.cpp)
target_include_directories (vm_gen PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_subdirectory (concrete)

add_library (vm INTERFACE )
target_link_libraries (vm INTERFACE vm_gen vm_concrete)
