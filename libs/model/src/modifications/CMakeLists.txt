add_library (modifications STATIC  funcinliner.cpp splitcmps.cpp loops.cpp splitasserts.cpp inserboolcasts.cpp expandnondet.cpp)
target_include_directories (modifications PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (modifications support util host)
