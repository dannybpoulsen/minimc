add_library (concrete OBJECT Query.cpp)

target_link_Libraries (concrete  PUBLIC model util hash support)
target_include_directories (concrete PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

