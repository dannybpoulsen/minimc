add_library (gencases_lib OBJECT gencases.cpp)
target_link_libraries (gencases_lib PUBLIC cpa storage  smt)
target_include_directories (gencases_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
