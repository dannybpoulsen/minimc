add_library (modelanalysis STATIC find_location_defs.cpp)
target_include_directories (modelanalysis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries (modelanalysis support)

