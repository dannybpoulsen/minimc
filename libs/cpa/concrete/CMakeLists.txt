project (cpa_concrete)
add_subdirectory (locvm)
add_library (concrete STATIC Query.cpp)

target_link_Libraries (concrete PUBLIC model util hash support vm vm_concrete cpa_internal)
target_include_directories (concrete PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

