project(util CXX)

add_library (util INTERFACE)
target_include_directories (util INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (util INTERFACE hash )
