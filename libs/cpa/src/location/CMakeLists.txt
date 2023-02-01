add_library (location STATIC state.cpp)
target_link_Libraries (location PUBLIC model hash vm)
target_include_directories (location PUBLIC ${PROJECT_SOURCE_DIR}/include)

