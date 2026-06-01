add_library (modifications OBJECT funcinliner.cpp loops.cpp splitasserts.cpp   isntrument_loads.cpp)
target_include_directories (modifications PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (modifications modelisa  support util host)
