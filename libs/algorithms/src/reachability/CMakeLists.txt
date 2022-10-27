add_library (algo_reach OBJECT reacahbility.cpp)
target_link_libraries (algo_reach PUBLIC cpa storage)
target_include_directories (algo_reach PUBLIC ${PROJECT_SOURCE_DIR}/include)
