add_library (location STATIC state.cpp)
target_link_Libraries (location PUBLIC model hash vm)
target_include_directories (location PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library (location_headers INTERFACE)
target_include_directories (location_headers INTERFACE ${PROJECT_SOURCE_DIR}/include)

