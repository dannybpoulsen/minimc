add_library (modifications STATIC markloopinslocs.cpp funcinliner.cpp splitcmps.cpp loops.cpp splitasserts.cpp inserboolcasts.cpp)
target_include_directories (modifications PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries (modifications support util)
