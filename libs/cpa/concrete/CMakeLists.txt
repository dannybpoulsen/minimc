add_library (concrete STATIC Query.cpp)

target_link_Libraries (concrete  PUBLIC model util hash support vm)
target_include_directories (concrete PUBLIC ${PROJECT_SOURCE_DIR}/include)

