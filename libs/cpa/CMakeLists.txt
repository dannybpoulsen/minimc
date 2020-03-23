add_subdirectory (location)
add_subdirectory (concrete_no_mem)
add_subdirectory (pathformula)

add_library (cpa SHARED interface.cpp)

target_link_libraries (cpa hash location concrete_no_mem pathformula)
target_include_directories (cpa INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
