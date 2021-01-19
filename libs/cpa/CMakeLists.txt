add_subdirectory (location)
add_subdirectory (pathformula)
add_subdirectory (concrete)

add_library (cpa STATIC interface.cpp)

target_link_libraries (cpa hash location  pathformula concrete)
target_include_directories (cpa INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
