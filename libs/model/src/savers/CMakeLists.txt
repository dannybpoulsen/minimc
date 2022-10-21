add_library (saver STATIC)
add_subdirectory (json)
target_link_libraries (saver  PUBLIC jsonsaver)
target_include_directories (saver PUBLIC include)

