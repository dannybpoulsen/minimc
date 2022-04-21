
add_library (algorithms INTERFACE)
add_subdirectory (reachability)
target_link_libraries (algorithms INTERFACE support cpa algo_reach)



target_include_directories (algorithms INTERFACE ${PROJECT_SOURCE_DIR}/include)
