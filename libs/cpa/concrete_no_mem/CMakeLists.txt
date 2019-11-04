add_library ( concrete_no_mem INTERFACE )
target_sources(location INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Query.cpp)

target_link_Libraries ( concrete_no_mem INTERFACE model hash support)
target_include_directories (concrete_no_mem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)


