add_library (location OBJECT)
target_sources(location PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/state.cpp)
target_link_Libraries (location PUBLIC model hash)
target_include_directories (location PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
