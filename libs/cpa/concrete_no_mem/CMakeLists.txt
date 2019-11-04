add_library ( concrete_no_mem OBJECT Query.cpp)

target_link_Libraries ( concrete_no_mem PUBLIC model hash support)
target_include_directories (concrete_no_mem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)


