add_subdirectory (locvm)
add_library (pathformula STATIC Query.cpp)

target_link_Libraries ( pathformula PUBLIC model util hash  vm smt pathvm cpa_headers PRIVATE cpa_internal)
target_include_directories (pathformula PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

