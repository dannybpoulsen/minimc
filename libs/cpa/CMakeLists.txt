add_subdirectory (location)
add_library (cpa INTERFACE)
target_link_libraries (cpa INTERFACE hash location)
target_include_directories (cpa INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
