add_library (algo_reach STATIC reacahbility.cpp)
target_link_libraries (algo_reach PUBLIC cpa)
target_include_directories (algo_reach INTERFACE ${PROJECT_SOURCE_DIR}/include)
