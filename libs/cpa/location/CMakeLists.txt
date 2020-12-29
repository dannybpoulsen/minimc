add_library (location INTERFACE)
target_sources(location INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/state.cpp ${CMAKE_CURRENT_SOURCE_DIR}/single_location.cpp)
target_link_Libraries (location INTERFACE model hash)
target_include_directories (location INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
