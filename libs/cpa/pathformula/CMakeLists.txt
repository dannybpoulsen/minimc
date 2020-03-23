add_library (pathformula OBJECT Query.cpp)

target_link_Libraries ( pathformula PUBLIC model util hash)
target_include_directories (pathformula PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

