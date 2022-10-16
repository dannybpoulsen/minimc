add_library (algorithms STATIC)
add_subdirectory (reachability)
target_link_libraries (algorithms PUBLIC algo_reach )
