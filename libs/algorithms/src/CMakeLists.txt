add_library (algorithms STATIC reachability.cpp gencases.cpp)
target_link_libraries (algorithms PUBLIC cpa storage  smt)
target_include_directories (algorithms PUBLIC ${PROJECT_SOURCE_DIR}/include)
