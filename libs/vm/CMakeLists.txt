add_library (vm_gen INTERFACE)
target_include_directories (vm_gen INTERFACE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory (concrete)
add_subdirectory (pathformula)

add_library (vm INTERFACE )
target_link_libraries (vm INTERFACE vm_gen vm_concrete vm_path)
