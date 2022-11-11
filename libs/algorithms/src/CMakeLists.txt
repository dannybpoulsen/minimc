add_library (algorithms STATIC ../include/algorithms/reachability/verdict.hpp)
add_subdirectory (reachability)
target_link_libraries (algorithms PUBLIC algo_reach )
