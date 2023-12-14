add_subdirectory (gencases)
add_library (algorithms STATIC reachability.cpp )
target_link_libraries (algorithms PUBLIC cpa storage  smt gencases_lib)
target_include_directories (algorithms PUBLIC ${PROJECT_SOURCE_DIR}/include)
